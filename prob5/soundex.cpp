
#include "soundex.h"

using namespace std;

Soundex Soundex::generateSoundex(const std::string& word)
{
    if( word.empty() )
    {
        return Soundex{ 0, {{0, 0, 0}}};
    }

    auto itWord = word.begin();

    const char firstLetter = ::toupper(*itWord++);

    array<uint8_t, 3> digits {{0}};

    auto it = digits.begin();

    uint8_t prev = 0;

    for( ; itWord != word.end(); ++itWord )
    {
        if( it == digits.end() )
        {
            break;
        }

        uint8_t digit = 0;
        switch ( ::tolower(*itWord) )
        {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
            case 'y':
                prev = 0;
                digit = 0;
                break;

            case 'h':
            case 'w':
                // ignore char
                digit = 0;
                break;

            case 'b':
            case 'f':
            case 'p':
            case 'v':

                digit  = 1;
                break;

            case 'c':
            case 'g':
            case 'j':
            case 'k':
            case 'q':
            case 's':
            case 'x':
            case 'z':
                digit = 2;
                break;

            case 'd':
            case 't':
                digit = 3;
                break;

            case 'l':
                digit = 4;
                break;

            case 'm':
            case 'n':
                digit = 5;
                break;

            case 'r':
                digit = 6;
                break;
        }

        if( (digit != 0) && (prev != digit) )
        {
            *it++ = digit;
        }

        prev = digit;
    }

    return Soundex{firstLetter, digits};
}
