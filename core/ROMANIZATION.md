# Romanization scheme / ရိုက်နည်း

Type Burmese using Latin letters. Each syllable is built as:

```
onset  +  medial(s)  +  vowel  +  tone  +  final
```

> Output is clean **Myanmar Unicode** (the encoding every modern phone, website
> and computer uses). The same scheme works identically on Windows and macOS.

---

## 1. Consonants (onset) / ဗျည်း

| type | Myanmar | type | Myanmar | type | Myanmar |
|------|---------|------|---------|------|---------|
| `k`  | က | `kh` | ခ | `g`  | ဂ |
| `ng` | င | `s`  | စ | `hs` | ဆ |
| `z`  | ဇ | `ny` | ည | `t`  | တ |
| `ht` | ထ | `d`  | ဒ | `dh` | ဓ |
| `n`  | န | `p`  | ပ | `hp` | ဖ |
| `b`  | ဗ | `hb` | ဘ | `m`  | မ |
| `y`  | ယ | `r`  | ရ | `l`  | လ |
| `w`  | ဝ | `th` | သ | `h`  | ဟ |
| `a`  | အ | `gh` | ဃ |    |   |

## 2. Medials / ဗျည်းတွဲ  (type right after the consonant)

| type | sign | example |
|------|------|---------|
| `y` | ◌ျ | `kya` → ကျ |
| `r` | ◌ြ | `kra` → ကြ |
| `w` | ◌ွ | `kwa` → ကွ |
| `h` | ◌ှ | `hma` → … (m+h) |

You can combine them: `kywa` → ကျွ

## 3. Vowels / သရ

| type | sign | example |
|------|------|---------|
| `a`        | (none) | `ka` → က |
| `aa`, `ar` | ◌ာ | `kaa` → ကာ |
| `i`        | ◌ိ | `ki` → ကိ |
| `ii`, `ee` | ◌ီ | `kii` → ကီ |
| `u`        | ◌ု | `ku` → ကု |
| `uu`, `oo` | ◌ူ | `kuu` → ကူ |
| `e`, `ay`  | ◌ေ | `ke` → ကေ |
| `ai`       | ◌ဲ | `kai` → ကဲ |
| `aw`, `au` | ◌ော | `kaw` → ကော |
| `o`        | ◌ို | `ko` → ကို |
| `aung`     | ◌ောင် | `kaung` → ကောင် |

## 4. Tones / အသံ

| type | sign | example |
|------|------|---------|
| `:` | ◌း (high) | `kaa:` → ကား |
| `.` | ◌့ (creaky) | `ku.` → ကု့ |

## 5. Finals / သတ်  (consonant that ends a syllable)

A consonant that is **not** followed by a vowel becomes a final (gets ◌်):

| type | example |
|------|---------|
| `t`  | `kat` → ကတ် |
| `n`  | `kan` → ကန် |
| `ng` | `kaung` → ကောင် |
| `k`  | `kak` → ကက် |
| `p`  | `kap` → ကပ် |
| `m`  | `kam` → ကမ် |

**Anusvara** (ံ): type a capital **`N`** → `kaN` → ကံ

## 6. Stacked consonants & kinzi / ပါဌ်ဆင့် နှင့် ကင်းစီး

Use an underscore **`_`** to stack the next consonant under the previous one.

| type | result | note |
|------|--------|------|
| `mng_galar` | မင်္ဂလာ | **kinzi** — `ng` keeps its ◌် and the next letter sits under it |
| `bud_dha`   | ဗုဒ္ဓ | stacked ဒ္ဓ (Pali) |
| `pat_ti`    | ပတ္တိ | stacked တ္တ |
| `kak_ka`    | ကက္က | stacked က္က |

Rule: write the syllable up to the consonant, then `_`, then the next
consonant. **Kinzi** (the little ◌င်္ on top, as in မင်္ဂလာ) happens
automatically when the consonant before `_` is **`ng`**.

---

## Tips / အကြံပြုချက်

- Type **one syllable at a time**, then **space** to commit.
- `kya naw` → ကျ နော
- If something looks wrong, press **Backspace** to edit before committing.

## 7. Customise your own rules / ကိုယ်ပိုင်ဇယား ထည့်ခြင်း

You can add your own shortcuts and override any mapping — no rebuild needed.

1. Copy **[custom.sample.txt](custom.sample.txt)** to:
   - macOS: `~/.myanmar-ime/custom.txt`
   - Windows: `%USERPROFILE%\.myanmar-ime\custom.txt`
2. Edit it. Each line is `roman = မြန်မာ`. A `[section]` header chooses what
   you edit: `[words]` (whole-word shortcuts, the default), `[onset]`,
   `[vowel]`, `[final]`.
3. Restart the keyboard (re-enable the input source / re-run the .exe).

Example:
```
[words]
mingalar = မင်္ဂလာ
mm       = မြန်မာ

[onset]
sh = ရှ
```
Now `mingalar` → မင်္ဂလာ, `mm` → မြန်မာ, `sha` → ရှ. Whole-word shortcuts always
win over the built-in rules; everything else still works as normal.

---

## Known limitations

- This is a rule-based transliterator, not a dictionary — spelling follows the
  rules above, so a few words with irregular spelling must be typed by their
  letters (and stacking uses the explicit `_` notation in section 6). For words
  you type often, add them as `[words]` shortcuts (section 7).
