
#ifndef SOUNDEX_H
#define SOUNDEX_H

#include <stdio.h>
#include <array>
#include <string>

struct Soundex
{
    const char firstLetter;
    const std::array<uint8_t, 3> digits;

    bool operator==(const Soundex& other) const
    {
        return (firstLetter == other.firstLetter) && (digits == other.digits);
    }

    static Soundex generateSoundex(const std::string& word);
};

#endif /* SOUNDEX_H */
