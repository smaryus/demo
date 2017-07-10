#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <thread>
#include <string>
#include <functional>
#include <condition_variable>
#include <atomic>

/**
 * Make a search on a binary file.
 * At construction create a thread on which the run will take place.
 * A callback function must be set to receive results
 */
class Search
{
public:
    /// Callback signature
    typedef std::function<void(const std::string&, bool)> Callback;

    /// Error status retuned by search
    enum Error : uint8_t { Ok = 0, FileNotFound, Unknown };

private:
    enum State : uint8_t { Waiting = 0, NewSearch, StopSearch, KillThread };

    /// Used internaly to represent an intermediat result
    struct IntermediarResult
    {
        std::string name; /// original result name
        int score; /// result match score

        bool operator<(const IntermediarResult& other) const
        {
            return score > other.score;
        }
    };

public:
    /**
     * Default constructor.
     */
    Search(const std::string& fileName = "words.bin");

    Search(const Search&) = delete;
    Search& operator=(const Search&) = delete;

    ~Search();

    /**
     * Make a search after text in file.
     * If a search is already in progress stop it.
     * @param text - searched word
     * @return returns Ok when search started, otherwise return an error code
     */
    Error search(const std::string& text);

    /**
     * Stop the current search.
     * If no search is in progress has no effect.
     */
    void stopSearch();

public:
    /**
     * Set callback that will be invoked from search thread for each result
     * found
     * @param callback - callback that will be invoked, supports also lamdas
     */
    void setCallback(Callback callback) { m_callback = callback; }

private:
    // read the searched file
    void readFile(const std::string& fileName);

    // here takes place the search
    void searchImpl(const std::string& searchedWord) const;

    // thread run loop
    void runLoop();

    // simplify the provided param. Make it lower case and ignore all non asci
    // chars
    std::string prepareSearchedWord(const std::string& param) const;

    // compare two strings and return a score for them. Returns -1 when
    // strings don't match.
    int compare(const std::string& searchedText,
                const std::string& other) const;

    // add a result into the resuts list and keep the list ordered by score
    void addResult(IntermediarResult&& result,
                   std::vector<IntermediarResult>& resultsList) const;

    // calculate levenshtein distance between two strings
    int levenshteinDistance(const std::string& searchedText,
                            const std::string& other, const int maxDiff) const;

private:
    std::atomic<State> m_searchState; /// searching state
    std::string m_searchText; /// current searching string
    Callback m_callback; /// callback
    FILE* m_file; /// file handler
    std::condition_variable m_wakeNotification; /// used to wakeup the worker thread
    std::mutex m_mutex; /// mutex used to restrict access to notification and search data
    std::thread m_workerThread; /// worker thread
};

#endif // SEARCH_H
