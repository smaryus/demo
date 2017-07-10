#include "search.h"

#include <cassert>
#include <iostream>
#include <cstring>
#include <algorithm>

#include "soundex.h"

using namespace std;

constexpr int kMaxNumberOfResults = 10;

Search::Search(const string& fileName)
: m_searchState(Waiting)
    , m_searchText()
    , m_callback(nullptr)
    , m_file(nullptr)
    , m_wakeNotification()
    , m_mutex()
    , m_workerThread(&Search::runLoop, this)
{
    readFile(fileName);
}

Search::~Search()
{
    m_searchState = State::KillThread;
    m_wakeNotification.notify_all();

    if( m_file )
    {
        fclose(m_file);
        m_file = nullptr;
    }

    m_workerThread.join();
}

Search::Error Search::search(const string& text)
{
    if( !m_file )
    {
        return Error::FileNotFound;
    }

    assert( m_callback );

    std::unique_lock<mutex> lock(m_mutex);

    m_searchState = State::NewSearch;
    m_searchText = text;

    lock.unlock();

    m_wakeNotification.notify_all();

    return Error::Ok;
}

void Search::stopSearch()
{
    m_searchState = State::StopSearch;

    m_wakeNotification.notify_all();
}

void Search::readFile(const std::string& fileName)
{
    assert( m_file == nullptr );

    m_file = fopen(fileName.c_str(), "rb");

    assert(m_file);
}

void Search::searchImpl(const string& searchedWord) const
{
    assert(m_file);

    fseek(m_file, 0, SEEK_SET);

    const string simplifiedWord = prepareSearchedWord(searchedWord);

    vector<IntermediarResult> exactMatchResults;
    exactMatchResults.reserve(kMaxNumberOfResults);

    vector<IntermediarResult> aproxResults;
    aproxResults.reserve(kMaxNumberOfResults);

    while( !feof(m_file) )
    {
        if( m_searchState != State::Waiting )
        {
            return;
        }

        uint8_t numberOfChars = 0;

        auto readSize = fread(&numberOfChars, sizeof(uint8_t), 1, m_file);

        if( readSize != sizeof(uint8_t) )
        {
            if( !feof(m_file) )
            {
                cout << "Decoding file error " << readSize
                << " != sizeof(uint8_t)" << endl;
            }

            break;
        }

        if( numberOfChars == 0 )
        {
            continue;
        }

        char buffer[numberOfChars];
        readSize = fread(buffer, sizeof(char), numberOfChars, m_file);
        assert( readSize == sizeof(char) * numberOfChars );

        if( readSize != numberOfChars )
        {
            cout << "Decoding file error " << readSize
                 << " != sizeof(char) * numberOfChars" << endl;
            return;
        }

        assert( strlen(buffer) + 1 == numberOfChars );

        const string readWord(buffer);
        const string simplifiedReadWord = prepareSearchedWord(readWord);

        auto score = compare(searchedWord, readWord);

        if( score > 0 )
        {
            // exact match, even cases

            addResult(IntermediarResult{ readWord, score + 1 },
                      exactMatchResults);

            continue;
        }

        score = compare(simplifiedWord, simplifiedReadWord);
        if( score > 0 )
        {
            addResult(IntermediarResult{ readWord, score }, exactMatchResults);

            continue;
        }

        // Levenshtein distance
        auto leveDistance = levenshteinDistance(simplifiedWord,
                                                simplifiedReadWord,
                                                5);
        if( leveDistance > 0 )
        {
            addResult(IntermediarResult{ readWord, 10 - leveDistance },
                      aproxResults);

            continue;
        }

        // soundex search

        if( Soundex::generateSoundex(readWord) == Soundex::generateSoundex(simplifiedWord) )
        {
            addResult(IntermediarResult{ readWord, 1 }, aproxResults);

            continue;
        }
    }

    if( m_searchState != State::Waiting )
    {
        return;
    }

    assert( m_callback );

    if( m_callback )
    {
        for( const auto& result : exactMatchResults )
        {
#ifndef NDEBUG
            cout << result.score << " - " << result.name << endl;
#endif
            m_callback(result.name, true);
        }

        for( auto result : aproxResults )
        {
#ifndef NDEBUG
            cout << result.score << "  " << result.name << endl;
#endif
            m_callback(result.name, false);
        }
    }
}

void Search::runLoop()
{
    while( m_searchState != State::KillThread )
    {
        string seachTextCopy;
        {
            std::unique_lock<mutex> lock(m_mutex);

            m_wakeNotification.wait(lock,
                                    [this] {
                                        return (m_searchState != State::Waiting);
                                    });

            if( m_searchState == State::KillThread )
            {
                return;
            }

            m_searchState = State::Waiting;
            seachTextCopy = m_searchText;
        }

        searchImpl(seachTextCopy);
    }
}

string Search::prepareSearchedWord(const string& param) const
{
    auto ptr = reinterpret_cast<const unsigned char *>(param.c_str());
    std::string output;
    output.reserve(param.size());

    // ignore non-asci chars and work only with lower case
    while( *ptr )
    {
        while( *ptr && (*ptr<0x80) )
        {
            output.push_back( ::tolower(*ptr++) );
        }

        if( *ptr )
        {
            if ((*ptr & 0xe0) == 0xc0)
            {
                ptr += 2;
            }
            else if ((*ptr & 0xf0) == 0xe0)
            {
                ptr += 3;
            }
            else if ((*ptr & 0xf8) == 0xf0)
            {
                ptr += 4;
            }
            else if ((*ptr & 0xfc) == 0xf8)
            {
                ptr += 5;
            }
            else if ((*ptr & 0xfe) == 0xfc)
            {
                ptr += 6;
            }
            else
            {
                ++ptr;
            }
        }
    }

    return output;
}

int Search::compare(const string& searchedText, const string& other) const
{
    if( searchedText.length() > other.length() )
    {
        return -1;
    }
    const char* ptr1 = searchedText.c_str();
    const char* ptr2 = other.c_str();

    int score = 0;

    while( *ptr1 && (*ptr1 == *ptr2) )
    {
        score += 2;
        ++ptr1;
        ++ptr2;
    }

    if( !*ptr1 )
    {
        // exact match
        score *= 2;
        if( !*ptr2 )
        {
            score *= 2;
        }
        else
        {
            score -= ( int(std::max(searchedText.length(), other.length())) - score);
        }
    }
    else
    {
        score = -1;
    }

    return score;
}

void Search::addResult(IntermediarResult&& result,
                       std::vector<IntermediarResult>& resultsList) const
{
    auto it = std::lower_bound(resultsList.begin(),
                               resultsList.end(), result);

    const bool popBack = (resultsList.size() >= kMaxNumberOfResults);

    if( !popBack || (it != resultsList.end()) )
    {
        resultsList.insert(it, std::move(result));
        if( popBack )
        {
            resultsList.pop_back();
        }
    }
}

int Search::levenshteinDistance(const string& searchedText,
                                const string& other,
                                const int maxDiff) const
{
    if( other.empty() || searchedText.empty() )
    {
        return -1;
    }

    int rowLength = (int)searchedText.length();
    int colLength = (int)other.length();

    int val0[rowLength+1];
    int val1[rowLength+1];

    for( int i=0; i <= rowLength; ++i )
    {
        val0[i] = i;
    }

    int* v0 = val0;
    int* v1 = val1;

    int minRowDistance = rowLength + colLength;

    for( int colIndex = 1; colIndex <= colLength; ++colIndex )
    {
        v1[0] = colIndex;

        for( int j=1; j<=rowLength; ++j )
        {
            const int cost = (searchedText[j-1] == other[colIndex-1]) ? 0 : 1;

            v1[j] = std::min({v1[j-1] + 1, v0[j] + 1, v0[j-1] + cost});
            minRowDistance = std::min(minRowDistance, v1[j]);
        }

        if( minRowDistance > maxDiff )
            return minRowDistance;

        std::swap(v0, v1);
    }

    int prefixDistance = v0[rowLength];
    return prefixDistance;
}
