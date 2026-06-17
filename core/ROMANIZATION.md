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

## Known limitations

- This is a rule-based transliterator, not a dictionary — spelling follows the
  rules above, so a few words with irregular spelling must be typed by their
  letters (and stacking uses the explicit `_` notation in section 6).
