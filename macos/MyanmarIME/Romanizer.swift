import Foundation

/// Myanmar Romanized -> Unicode conversion engine (Swift port).
///
/// This is a 1:1 port of core/romanizer.py and the C++ engine. Keep all three
/// in sync — see ROMANIZATION.md for the human-readable specification.
enum Romanizer {

    // MARK: Unicode building blocks
    static let ASAT     = "\u{103A}"   // ်  killer / virama
    static let ANUSVARA = "\u{1036}"   // ံ
    static let DOT      = "\u{1037}"   // ့  creaky tone
    static let VISARGA  = "\u{1038}"   // း  high tone

    // MARK: Tables (longest key matched first)
    static let onsets: [String: String] = [
        "kh": "ခ", "g": "ဂ", "ng": "င", "k": "က",
        "hs": "ဆ", "ss": "ဆ", "s": "စ", "z": "ဇ",
        "ny": "ည",
        "ht": "ထ", "th": "သ", "dh": "ဓ", "t": "တ",
        "d": "ဒ", "n": "န",
        "hp": "ဖ", "ph": "ဖ", "hb": "ဘ", "bh": "ဘ",
        "p": "ပ", "b": "ဗ", "m": "မ",
        "y": "ယ", "r": "ရ", "l": "လ", "w": "ဝ",
        "h": "ဟ", "a": "အ",
        "gh": "ဃ",
    ]

    static let medials: [String: String] = [
        "y": "\u{103B}", "r": "\u{103C}", "w": "\u{103D}", "h": "\u{103E}",
    ]
    // canonical Unicode storage order
    static let medialOrder = ["\u{103B}", "\u{103C}", "\u{103D}", "\u{103E}"]

    static let vowels: [String: String] = [
        "aung": "ောင်", "aa": "ာ", "ar": "ာ",
        "ai": "ဲ", "au": "ော", "aw": "ော",
        "ay": "ေ", "ee": "ီ", "ii": "ီ",
        "oo": "ူ", "uu": "ူ",
        "o": "ို", "e": "ေ",
        "i": "ိ", "u": "ု", "a": "",
    ]

    static let finals: [String: String] = [
        "ng": "င", "ny": "ည",
        "k": "က", "s": "စ", "t": "တ", "n": "န",
        "p": "ပ", "m": "မ", "y": "ယ", "w": "ဝ",
        "th": "သ", "l": "လ",
    ]

    static let tones: [String: String] = [".": DOT, ":": VISARGA]
    static let vowelLetters = Set("aeiou")

    private static func matchLongest(_ table: [String: String], _ s: [Character], _ i: Int) -> (String, String, Int)? {
        for len in stride(from: 4, through: 1, by: -1) {
            let end = i + len
            if end <= s.count {
                let chunk = String(s[i..<end])
                if let v = table[chunk] { return (chunk, v, end) }
            }
        }
        return nil
    }

    private static func convertWord(_ word: String) -> String {
        let w = Array(word)
        let n = w.count
        var out = ""
        var i = 0
        while i < n {
            let c = w[i]
            if let t = tones[String(c)] { out += t; i += 1; continue }
            if c == "N" { out += ANUSVARA; i += 1; continue }
            if !c.isLetter { out.append(c); i += 1; continue }

            // 1) onset
            guard let (_, ons, j) = matchLongest(onsets, w, i) else {
                out.append(c); i += 1; continue
            }
            var syl = ons
            i = j

            // 2) medials
            var found = Set<String>()
            while i < n, let m = medials[String(w[i])] {
                let nxt: Character? = (i + 1 < n) ? w[i + 1] : nil
                let isVowelNext = nxt != nil && vowelLetters.contains(nxt!)
                let isMedialNext = nxt != nil && medials[String(nxt!)] != nil
                if isVowelNext || isMedialNext { found.insert(m); i += 1 } else { break }
            }
            for code in medialOrder where found.contains(code) { syl += code }

            // 3) vowel
            if let (_, vval, j2) = matchLongest(vowels, w, i) { syl += vval; i = j2 }

            // 4) tone
            if i < n, let t = tones[String(w[i])] { syl += t; i += 1 }

            // 5) final
            if let (_, fval, j3) = matchLongest(finals, w, i) {
                let after: Character? = (j3 < n) ? w[j3] : nil
                let vowelAfter = after != nil && vowelLetters.contains(after!)
                let medialAfter = after != nil && medials[String(after!)] != nil
                if !vowelAfter && !medialAfter {
                    syl += fval + ASAT
                    i = j3
                    if i < n, let t = tones[String(w[i])] { syl += t; i += 1 }
                }
            }
            out += syl
        }
        return out
    }

    /// Convert a full roman string to Myanmar Unicode, preserving whitespace.
    static func convert(_ text: String) -> String {
        var result = ""
        var token = ""
        for ch in text {
            if ch.isWhitespace {
                if !token.isEmpty { result += convertWord(token); token = "" }
                result.append(ch)
            } else {
                token.append(ch)
            }
        }
        if !token.isEmpty { result += convertWord(token) }
        return result
    }
}
