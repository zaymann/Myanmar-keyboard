// Myanmar Romanized -> Unicode conversion engine (C++ port, header-only).
//
// 1:1 port of core/romanizer.py and macos/.../Romanizer.swift.
// Keep all three in sync. See ROMANIZATION.md for the specification.
//
// Works in UTF-8 std::string throughout. The Burmese output is UTF-8;
// convert to UTF-16 with the helper Utf8ToWide() before SendInput on Windows.

#ifndef MYANMAR_ROMANIZER_H
#define MYANMAR_ROMANIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace myanmar {

// UTF-8 building blocks
inline const char* ASAT     = "\xE1\x80\xBA"; // ်
inline const char* ANUSVARA = "\xE1\x80\xB6"; // ံ
inline const char* DOT      = "\xE1\x80\xB7"; // ့
inline const char* VISARGA  = "\xE1\x80\xB8"; // း

struct Tables {
    std::unordered_map<std::string, std::string> onsets, medials, vowels, finals, tones;
    std::vector<std::string> medialOrder;
    std::unordered_set<char> vowelLetters;

    Tables() {
        onsets = {
            {"kh","\xE1\x80\x81"},{"g","\xE1\x80\x82"},{"ng","\xE1\x80\x84"},{"k","\xE1\x80\x80"},
            {"hs","\xE1\x80\x86"},{"ss","\xE1\x80\x86"},{"s","\xE1\x80\x85"},{"z","\xE1\x80\x87"},
            {"ny","\xE1\x80\x8A"},
            {"ht","\xE1\x80\x91"},{"th","\xE1\x80\x9E"},{"dh","\xE1\x80\x93"},{"t","\xE1\x80\x90"},
            {"d","\xE1\x80\x92"},{"n","\xE1\x80\x94"},
            {"hp","\xE1\x80\x96"},{"ph","\xE1\x80\x96"},{"hb","\xE1\x80\x98"},{"bh","\xE1\x80\x98"},
            {"p","\xE1\x80\x95"},{"b","\xE1\x80\x97"},{"m","\xE1\x80\x99"},
            {"y","\xE1\x80\x9A"},{"r","\xE1\x80\x9B"},{"l","\xE1\x80\x9C"},{"w","\xE1\x80\x9D"},
            {"h","\xE1\x80\x9F"},{"a","\xE1\x80\xA1"},
            {"gh","\xE1\x80\x83"},
        };
        medials = {
            {"y","\xE1\x80\xBB"},{"r","\xE1\x80\xBC"},{"w","\xE1\x80\xBD"},{"h","\xE1\x80\xBE"},
        };
        medialOrder = {"\xE1\x80\xBB","\xE1\x80\xBC","\xE1\x80\xBD","\xE1\x80\xBE"};
        vowels = {
            {"aung","\xE1\x80\xB1\xE1\x80\xAC\xE1\x80\x84\xE1\x80\xBA"}, // ောင်
            {"aa","\xE1\x80\xAC"},{"ar","\xE1\x80\xAC"},
            {"ai","\xE1\x80\xB2"},
            {"au","\xE1\x80\xB1\xE1\x80\xAC"},{"aw","\xE1\x80\xB1\xE1\x80\xAC"},
            {"ay","\xE1\x80\xB1"},{"ee","\xE1\x80\xAE"},{"ii","\xE1\x80\xAE"},
            {"oo","\xE1\x80\xB0"},{"uu","\xE1\x80\xB0"},
            {"o","\xE1\x80\xAD\xE1\x80\xAF"},{"e","\xE1\x80\xB1"},
            {"i","\xE1\x80\xAD"},{"u","\xE1\x80\xAF"},{"a",""},
        };
        finals = {
            {"ng","\xE1\x80\x84"},{"ny","\xE1\x80\x8A"},
            {"k","\xE1\x80\x80"},{"s","\xE1\x80\x85"},{"t","\xE1\x80\x90"},{"n","\xE1\x80\x94"},
            {"p","\xE1\x80\x95"},{"m","\xE1\x80\x99"},{"y","\xE1\x80\x9A"},{"w","\xE1\x80\x9D"},
            {"th","\xE1\x80\x9E"},{"l","\xE1\x80\x9C"},
        };
        tones = {{".",DOT},{":",VISARGA}};
        vowelLetters = {'a','e','i','o','u'};
    }
};

inline const Tables& tables() { static Tables t; return t; }

// longest-match (len 4..1) lookup at position i in s
inline bool matchLongest(const std::unordered_map<std::string,std::string>& tbl,
                         const std::string& s, size_t i,
                         std::string& valOut, size_t& newIndex) {
    for (int len = 4; len >= 1; --len) {
        if (i + (size_t)len <= s.size()) {
            std::string chunk = s.substr(i, len);
            auto it = tbl.find(chunk);
            if (it != tbl.end()) { valOut = it->second; newIndex = i + len; return true; }
        }
    }
    return false;
}

inline bool isAlpha(char c) { return (c>='a'&&c<='z')||(c>='A'&&c<='Z'); }

inline std::string convertWord(const std::string& word) {
    const Tables& T = tables();
    std::string out;
    size_t i = 0, n = word.size();
    while (i < n) {
        char c = word[i];
        std::string cs(1, c);
        if (T.tones.count(cs)) { out += T.tones.at(cs); ++i; continue; }
        if (c == 'N') { out += ANUSVARA; ++i; continue; }
        if (!isAlpha(c)) { out += c; ++i; continue; }

        std::string ons; size_t j;
        if (!matchLongest(T.onsets, word, i, ons, j)) { out += c; ++i; continue; }
        std::string syl = ons;
        i = j;

        // medials
        std::unordered_set<std::string> found;
        while (i < n) {
            std::string ms(1, word[i]);
            auto it = T.medials.find(ms);
            if (it == T.medials.end()) break;
            char nxt = (i + 1 < n) ? word[i+1] : '\0';
            bool vowelNext  = T.vowelLetters.count(nxt) > 0;
            bool medialNext = T.medials.count(std::string(1, nxt)) > 0;
            if (vowelNext || medialNext) { found.insert(it->second); ++i; }
            else break;
        }
        for (const auto& code : T.medialOrder)
            if (found.count(code)) syl += code;

        // vowel
        std::string vval; size_t j2;
        if (matchLongest(T.vowels, word, i, vval, j2)) { syl += vval; i = j2; }

        // tone
        if (i < n) { std::string ts(1, word[i]); if (T.tones.count(ts)) { syl += T.tones.at(ts); ++i; } }

        // final
        std::string fval; size_t j3;
        if (matchLongest(T.finals, word, i, fval, j3)) {
            char after = (j3 < n) ? word[j3] : '\0';
            bool vowelAfter  = T.vowelLetters.count(after) > 0;
            bool medialAfter = T.medials.count(std::string(1, after)) > 0;
            if (!vowelAfter && !medialAfter) {
                syl += fval; syl += ASAT;
                i = j3;
                if (i < n) { std::string ts(1, word[i]); if (T.tones.count(ts)) { syl += T.tones.at(ts); ++i; } }
            }
        }
        out += syl;
    }
    return out;
}

// Convert full roman string -> Myanmar Unicode (UTF-8), preserving whitespace.
inline std::string convert(const std::string& text) {
    std::string result, token;
    for (char ch : text) {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            if (!token.empty()) { result += convertWord(token); token.clear(); }
            result += ch;
        } else token += ch;
    }
    if (!token.empty()) result += convertWord(token);
    return result;
}

} // namespace myanmar
#endif
