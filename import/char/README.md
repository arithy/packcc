# Import Files of Character Matching Rules

## Overview

In this directory, import files that define character matching rules are stored.
These import files are recommended to **be imported after the last rule** in the PEG file that imports them.

## Import Files

### `char/ascii_character_group.peg`

#### Synopsis

An import file that defines rules to match an ASCII character belonging to a specific character group.

#### PEG Rules

The following PEG rules are available.

| Rule Name | Description |
| --- | --- |
| `ASCII_Printable_Character` | Matches a printable character, i.e. a character other than control characters. |
| `ASCII_Letter`              | Matches an alphabet character (`[A-Za-z]`). |
| `ASCII_Control_Character`   | Matches a control character (`[\x00-\x1f\x7f]`). |
| `ASCII_Special_Character`   | Matches a character other than control characters, number characters, and alphabet characters. |
| `ASCII_Number`              | Matches a number character (`[0-9]`). |
| `ASCII_Uppercase_Letter`    | Matches an uppercase alphabet character (`[A-Z]`). |
| `ASCII_Lowercase_Letter`    | Matches a lowercase alphabet character (`[a-z]`). |
| `ASCII_C_alnum`             | Matches a character for which the standard C function `isalnum()` returns a non-zero value (`[0-9A-Za-z]`). |
| `ASCII_C_alpha`             | Matches a character for which the standard C function `isalpha()` returns a non-zero value (= `ASCII_Letter`). |
| `ASCII_C_blank`             | Matches a character for which the standard C function `isblank()` returns a non-zero value (`[ \t]`). |
| `ASCII_C_cntrl`             | Matches a character for which the standard C function `iscntrl()` returns a non-zero value (= `ASCII_Control_Character`). |
| `ASCII_C_digit`             | Matches a character for which the standard C function `isdigit()` returns a non-zero value (= `ASCII_Number`). |
| `ASCII_C_graph`             | Matches a character for which the standard C function `isgraph()` returns a non-zero value (= `ASCII_Printable_Character` excluding the space character `' '`). |
| `ASCII_C_lower`             | Matches a character for which the standard C function `islower()` returns a non-zero value (= `ASCII_Lowercase_Letter`). |
| `ASCII_C_print`             | Matches a character for which the standard C function `isprint()` returns a non-zero value (= `ASCII_Printable_Character`). |
| `ASCII_C_punct`             | Matches a character for which the standard C function `ispunct()` returns a non-zero value (= `ASCII_Special_Character` excluding the space character `' '`). |
| `ASCII_C_space`             | Matches a character for which the standard C function `isspace()` returns a non-zero value (`[ \t\n\v\f\r]`). |
| `ASCII_C_upper`             | Matches a character for which the standard C function `isupper()` returns a non-zero value (= `ASCII_Uppercase_Letter`). |
| `ASCII_C_xdigit`            | Matches a character for which the standard C function `isxdigit()` returns a non-zero value (`[0-9A-Fa-f]`). |

### `char/unicode_general_category.peg`

#### Synopsis

An import file that defines rules to match a Unicode character belonging to a specific [general category](https://unicode.org/reports/tr44/#General_Category_Values).

#### PEG Rules

The following PEG rules are available.

| Rule Name | Description |
| --- | --- |
| `Unicode_Uppercase_Letter`      | Matches an uppercase letter. |
| `Unicode_Lowercase_Letter`      | Matches a lowercase letter. |
| `Unicode_Titlecase_Letter`      | Matches a digraph encoded as a single character, with the first part uppercase. |
| `Unicode_Cased_Letter`          | Matches a cased letter (= `Unicode_Uppercase_Letter / Unicode_Lowercase_Letter / Unicode_Titlecase_Letter`). |
| `Unicode_Modifier_Letter`       | Matches a modifier letter. |
| `Unicode_Other_Letter`          | Matches a letter of other type, including syllables and ideographs. |
| `Unicode_Letter`                | Matches a letter (= `Unicode_Cased_Letter / Unicode_Modifier_Letter / Unicode_Other_Letter`). |
| `Unicode_Nonspacing_Mark`       | Matches a nonspacing combining mark (zero advance width). |
| `Unicode_Spacing_Mark`          | Matches a spacing combining mark (positive advance width). |
| `Unicode_Enclosing_Mark`        | Matches an enclosing combining mark. |
| `Unicode_Mark`                  | Matches a mark (= `Unicode_Nonspacing_Mark / Unicode_Spacing_Mark / Unicode_Enclosing_Mark`). |
| `Unicode_Decimal_Number`        | Matches a decimal digit. |
| `Unicode_Letter_Number`         | Matches a letterlike numeric character. |
| `Unicode_Other_Number`          | Matches a numeric character of other type. |
| `Unicode_Number`                | Matches a numeric character (= `Unicode_Decimal_Number / Unicode_Letter_Number / Unicode_Other_Number`). |
| `Unicode_Connector_Punctuation` | Matches a connecting punctuation mark, like a tie. |
| `Unicode_Dash_Punctuation`      | Matches a dash or hyphen punctuation mark. |
| `Unicode_Open_Punctuation`      | Matches an opening punctuation mark (of a pair). |
| `Unicode_Close_Punctuation`     | Matches a closing punctuation mark (of a pair). |
| `Unicode_Initial_Punctuation`   | Matches an initial quotation mark. |
| `Unicode_Final_Punctuation`     | Matches a final quotation mark. |
| `Unicode_Other_Punctuation`     | Matches a punctuation mark of other type. |
| `Unicode_Punctuation`           | Matches a punctuation mark (= `Unicode_Connector_Punctuation / Unicode_Dash_Punctuation / Unicode_Open_Punctuation / Unicode_Close_Punctuation / Unicode_Initial_Punctuation / Unicode_Final_Punctuation / Unicode_Other_Punctuation`). |
| `Unicode_Math_Symbol`           | Matches a symbol of mathematical use. |
| `Unicode_Currency_Symbol`       | Matches a currency sign. |
| `Unicode_Modifier_Symbol`       | Matches a non-letterlike modifier symbol. |
| `Unicode_Other_Symbol`          | Matches a symbol of other type. |
| `Unicode_Symbol`                | Matches a symbol (= `Unicode_Math_Symbol / Unicode_Currency_Symbol / Unicode_Modifier_Symbol / Unicode_Other_Symbol`). |
| `Unicode_Space_Separator`       | Matches a space character (of various non-zero widths). |
| `Unicode_Line_Separator`        | Matches U+2028 "LINE SEPARATOR" only. |
| `Unicode_Paragraph_Separator`   | Matches U+2029 "PARAGRAPH SEPARATOR" only. |
| `Unicode_Separator`             | Matches a space character (= `Unicode_Space_Separator / Unicode_Line_Separator / Unicode_Paragraph_Separator`). |
| `Unicode_Control`               | Matches a [C0](https://www.unicode.org/charts/nameslist/n_0000.html) or [C1](https://www.unicode.org/charts/nameslist/n_0080.html) control code. |
| `Unicode_Format`                | Matches a format control character. |
| `Unicode_Surrogate`             | Matches a surrogate code point. |
| `Unicode_Private_Use`           | Matches a private-use character. |
| `Unicode_Other`                 | Matches a character of other type (= `Unicode_Control / Unicode_Format / Unicode_Surrogate / Unicode_Private_Use`). |
