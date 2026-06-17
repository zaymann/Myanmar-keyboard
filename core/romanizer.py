#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Myanmar Romanized -> Unicode conversion engine (reference implementation).

This is the *core* of the keyboard. The Windows (C++) and macOS (Swift)
input methods each re-implement this exact same algorithm so that all three
platforms behave identically.

Design (inspired by WaitZar, but modernised):
  - WaitZar produced Zawgyi glyphs from a big binary dictionary/model.
  - This engine is RULE-BASED and produces clean Myanmar **Unicode** (the
    encoding every current system uses), so no dictionary file is needed.

A Burmese syllable is parsed as:
      onset  +  medial(s)  +  vowel  +  tone  +  final
e.g.  "kyaw:"  ->  ky(ကျ) + aw(ော) + :(း)            = ကျော်... -> ကျော်
      "mingga" handled syllable by syllable.

See ROMANIZATION.md for the full human-readable table.
"""

# ---- Unicode building blocks -------------------------------------------------
ASAT      = "်"   # ်  killer / virama  (makes a final consonant)
STACK     = "္"   # ္  stacking virama (pat-sint)
ANUSVARA  = "ံ"   # ံ
DOT_BELOW = "့"   # ့  creaky tone
VISARGA   = "း"   # း  high tone

# ---- Onsets (initial consonants). Longest key MUST be matched first. ---------
ONSETS = {
    "kh": "ခ", "g": "ဂ", "ng": "င", "k": "က",
    "hs": "ဆ", "ss": "ဆ", "s": "စ", "z": "ဇ",
    "ny": "ည",
    "ht": "ထ", "th": "သ", "dh": "ဓ", "t": "တ",
    "d": "ဒ", "n": "န",
    "hp": "ဖ", "ph": "ဖ", "hb": "ဘ", "bh": "ဘ",
    "p": "ပ", "b": "ဗ", "m": "မ",
    "y": "ယ", "r": "ရ", "l": "လ", "w": "ဝ",
    "h": "ဟ", "a": "အ",
    # less common / classical
    "gh": "ဃ", " jh": "ဈ",
}

# ---- Medials. Appear AFTER an onset, BEFORE the vowel. -----------------------
# Unicode storage order is fixed: ya/ra, then wa, then ha.
MEDIALS = {
    "y": "ျ",   # ◌ျ  ya-pint
    "r": "ြ",   # ◌ြ  ya-yit
    "w": "ွ",   # ◌ွ  wa-hswe
    "h": "ှ",   # ◌ှ  ha-hto
}

# ---- Vowels (rhyme nucleus). Longest key first. ------------------------------
# An empty string means the inherent "a" (bare consonant).
VOWELS = {
    "aung": "ောင်",   # ‌ောင်
    "aing": "ိုင်",    # not standard; handled via ai+ng
    "aa": "ာ", "ar": "ာ",
    "ai": "ဲ", "au": "ော", "aw": "ော",
    "ay": "ေ", "ee": "ီ", "ii": "ီ",
    "oo": "ူ", "uu": "ူ",
    "o": "ို",
    "e": "ေ",
    "i": "ိ", "u": "ု",
    "a": "",
}

# ---- Final consonants (get an ASAT appended). --------------------------------
FINALS = {
    "ng": "င", "ny": "ည",
    "k": "က", "s": "စ", "t": "တ", "n": "န",
    "p": "ပ", "m": "မ", "y": "ယ", "w": "ဝ",
    "th": "သ", "l": "လ",
}

# Tone marks typed as trailing punctuation.
TONES = {".": DOT_BELOW, ":": VISARGA}

VOWEL_LETTERS = set("aeiou")

# ---- User customisation ------------------------------------------------------
# Whole-word shortcuts: an exact roman token -> exact Myanmar output. Checked
# before syllable parsing, so it always wins. Populated from a custom file.
WORDS = {}

# Map a [section] name to the table it edits.
_SECTIONS = {"words": WORDS, "onset": ONSETS, "vowel": VOWELS, "final": FINALS}


def load_custom(path):
    """Load user customisations from a simple text file. Returns count loaded.

    Format (see core/custom.sample.txt):
        # comment
        [words]            <- section header; default section is [words]
        mingalar = မင်္ဂလာ
        [onset]
        sh = ရှ
    """
    import os
    if not path or not os.path.exists(path):
        return 0
    section = WORDS
    count = 0
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("[") and line.endswith("]"):
                section = _SECTIONS.get(line[1:-1].strip().lower(), section)
                continue
            if "=" not in line:
                continue
            key, _, val = line.partition("=")
            key, val = key.strip(), val.strip()
            if key:
                section[key] = val
                count += 1
    return count


def _match_longest(table, s, i):
    """Return (key, value, newindex) for the longest table key matching at i."""
    for length in (4, 3, 2, 1):
        chunk = s[i:i + length]
        if chunk in table:
            return chunk, table[chunk], i + length
    return None, None, i


def _convert_word(word):
    """Convert one lowercase roman 'word' (letters + . : only) to Unicode."""
    # whole-word override wins over the rules
    if word in WORDS:
        return WORDS[word]
    out = []
    i = 0
    n = len(word)
    while i < n:
        c = word[i]
        # tone marks / anything not a letter -> pass through (handled by caller)
        if c in TONES:
            out.append(TONES[c]); i += 1; continue
        if c == "N":
            out.append(ANUSVARA); i += 1; continue
        if c == "_":
            # stacker: pull the following consonant under the previous one
            # (kinzi keeps the preceding asat; plain stacking does not).
            out.append(STACK); i += 1; continue
        if not c.isalpha():
            out.append(c); i += 1; continue

        # 1) ONSET (required to start a syllable)
        _, ons, j = _match_longest(ONSETS, word, i)
        if ons is None:
            out.append(c); i += 1; continue
        syl = ons
        i = j

        # 2) MEDIALS (y, r, w, h) — only while the next char after is a vowel
        #    or another medial, never when it would swallow the next onset.
        med_order = []
        while i < n and word[i] in MEDIALS:
            nxt = word[i + 1] if i + 1 < n else ""
            # a medial must be followed by a vowel (or another medial then vowel)
            if nxt in VOWEL_LETTERS or nxt in MEDIALS:
                med_order.append(MEDIALS[word[i]]); i += 1
            else:
                break
        # store medials in canonical order ya/ra, wa, ha
        for code in ("ျ", "ြ", "ွ", "ှ"):
            if code in med_order:
                syl += code

        # 3) VOWEL
        vkey, vval, j = _match_longest(VOWELS, word, i)
        if vkey is not None:
            i = j
        else:
            vval = ""   # inherent 'a'

        # 4) explicit anusvara via 'N' already handled; nasal 'an' style:
        #    leave to finals.

        # ‌ော / ောင် place vowel after consonant+medial
        syl += vval

        # 5) TONE
        if i < n and word[i] in TONES:
            syl += TONES[word[i]]; i += 1

        # 6) FINAL consonant: only if the consonant is NOT followed by a vowel
        #    (a consonant + vowel begins the next syllable instead).
        fkey, fval, j = _match_longest(FINALS, word, i)
        if fkey is not None:
            after = word[j] if j < n else ""
            if after == "_":
                # this final stacks onto the next consonant.
                #   kinzi (င) keeps its asat: င ် ္
                #   any other consonant stacks bare:  C ္
                syl += fval
                if fval == "င":
                    syl += ASAT
                i = j
            elif after not in VOWEL_LETTERS and after not in MEDIALS:
                syl += fval + ASAT
                i = j
                # trailing tone after a final (e.g. "kann:")
                if i < n and word[i] in TONES:
                    syl += TONES[word[i]]; i += 1

        out.append(syl)
    return "".join(out)


def convert(text):
    """Convert a full roman string to Myanmar Unicode, preserving spaces."""
    result = []
    token = []
    for ch in text:
        if ch.isspace():
            if token:
                result.append(_convert_word("".join(token))); token = []
            result.append(ch)
        else:
            token.append(ch)
    if token:
        result.append(_convert_word("".join(token)))
    return "".join(result)


if __name__ == "__main__":
    import sys
    tests = [
        ("ka",     "က"),                         # က
        ("kaa",    "ကာ"),                    # ကာ
        ("kaa:",   "ကား"),              # ကား
        ("ki",     "ကိ"),                    # ကိ
        ("kii",    "ကီ"),                    # ကီ
        ("ku",     "ကု"),                    # ကု
        ("kuu",    "ကူ"),                    # ကူ
        ("ko",     "ကို"),              # ကို
        ("ke",     "ကေ"),                    # ကေ
        ("kai",    "ကဲ"),                    # ကဲ
        ("kaw",    "ကော"),              # ကော
        ("kya",    "ကျ"),                    # ကျ
        ("kywa",   "ကျွ"),              # ကျွ
        ("khaa",   "ခာ"),                    # ခါ -> ခာ
        ("thaa",   "သာ"),                    # သာ
        ("man",    "မန်"),              # မန်
        ("kan",    "ကန်"),              # ကန်
        ("kaN",    "ကံ"),                    # ကံ
        ("kat",    "ကတ်"),              # ကတ်
        ("kaung",  "ကောင်"),  # ကောင်
        # --- stacking / kinzi ---
        ("mng_galar", "မင်္ဂလာ"),   # kinzi: မင်္ဂလာ
        ("bud_dha",   "ဗုဒ္ဓ"),         # stack: ဗုဒ္ဓ
        ("pat_ti",    "ပတ္တိ"),         # stack: ပတ္တိ
        ("kak_ka",    "ကက္က"),         # stack: ကက္က
    ]
    if len(sys.argv) > 1:
        print(convert(" ".join(sys.argv[1:])))
    else:
        ok = 0
        for roman, expect in tests:
            got = convert(roman)
            mark = "OK " if got == expect else "XX "
            if got == expect:
                ok += 1
            print(f"{mark} {roman:8} -> {got}    (expected {expect})")
        print(f"\n{ok}/{len(tests)} passed")
