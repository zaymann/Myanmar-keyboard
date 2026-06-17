import Foundation

/// Myanmar Romanized -> Unicode conversion engine (Swift port).
///
/// This is a 1:1 port of core/romanizer.py and the C++ engine. Keep all three
/// in sync — see ROMANIZATION.md for the human-readable specification.
struct Romanizer {

    // MARK: Unicode building blocks
    static let ASAT     = "\u{103A}"   // ်  killer
    static let VIRAMA   = "\u{1039}"   // ္  stacking virama (pat-sint / kinzi)
    static let ANUSVARA = "\u{1036}"   // ံ
    static let DOT      = "\u{1037}"   // ့  creaky tone
    static let VISARGA  = "\u{1038}"   // း  high tone
    static let NGA      = "\u{1004}"   // င  (kinzi base)

    // MARK: Tables (longest key matched first)
    static var words: [String: String] = [:]   // whole-word overrides

    static var onsets: [String: String] = [
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

    static var vowels: [String: String] = [
        "aung": "ောင်", "aa": "ာ", "ar": "ာ",
        "ai": "ဲ", "au": "ော", "aw": "ော",
        "ay": "ေ", "ee": "ီ", "ii": "ီ",
        "oo": "ူ", "uu": "ူ",
        "o": "ို", "e": "ေ",
        "i": "ိ", "u": "ု", "a": "",
    ]

    static var finals: [String: String] = [
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
        if let override = words[word] { return override }   // whole-word wins
        let w = Array(word)
        let n = w.count
        var out = ""
        var i = 0
        while i < n {
            let c = w[i]
            if let t = tones[String(c)] { out += t; i += 1; continue }
            if c == "N" { out += ANUSVARA; i += 1; continue }
            if c == "_" { out += VIRAMA; i += 1; continue }  // stacker
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
                if after == "_" {
                    // stacks onto the next consonant; kinzi (င) keeps its asat.
                    syl += fval
                    if fval == NGA { syl += ASAT }
                    i = j3
                } else {
                    let vowelAfter = after != nil && vowelLetters.contains(after!)
                    let medialAfter = after != nil && medials[String(after!)] != nil
                    if !vowelAfter && !medialAfter {
                        syl += fval + ASAT
                        i = j3
                        if i < n, let t = tones[String(w[i])] { syl += t; i += 1 }
                    }
                }
            }
            out += syl
        }
        return out
    }

    /// Load a custom-rules file (see core/custom.sample.txt). Returns rules loaded.
    @discardableResult
    static func loadCustom(path: String) -> Int {
        guard let content = try? String(contentsOfFile: path, encoding: .utf8) else { return 0 }
        var section = "words"
        var count = 0
        for raw in content.split(separator: "\n", omittingEmptySubsequences: false) {
            let line = raw.trimmingCharacters(in: .whitespacesAndNewlines)
            if line.isEmpty || line.hasPrefix("#") { continue }
            if line.hasPrefix("[") && line.hasSuffix("]") {
                section = String(line.dropFirst().dropLast()).trimmingCharacters(in: .whitespaces).lowercased()
                continue
            }
            guard let eq = line.firstIndex(of: "=") else { continue }
            let key = String(line[..<eq]).trimmingCharacters(in: .whitespaces)
            let val = String(line[line.index(after: eq)...]).trimmingCharacters(in: .whitespaces)
            if key.isEmpty { continue }
            switch section {
            case "onset": onsets[key] = val
            case "vowel": vowels[key] = val
            case "final": finals[key] = val
            default:      words[key]  = val
            }
            count += 1
        }
        return count
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
