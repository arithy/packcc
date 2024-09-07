/*
 * PackCC: a packrat parser generator for C.
 *
 * Copyright (c) 2014, 2019-2024 Arihiro Yoshida. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * The algorithm is based on the paper "Packrat Parsers Can Support Left Recursion"
 * authored by A. Warth, J. R. Douglass, and T. Millstein.
 *
 * The specification is determined by referring to peg/leg developed by Ian Piumarta.
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifndef _MSC_VER
#if defined __GNUC__ && defined _WIN32 /* MinGW */
#ifndef PCC_USE_SYSTEM_STRNLEN
#define strnlen(str, maxlen) strnlen_(str, maxlen)
static size_t strnlen_(const char *str, size_t maxlen) {
    size_t i;
    for (i = 0; i < maxlen && str[i]; i++);
    return i;
}
#endif /* !PCC_USE_SYSTEM_STRNLEN */
#endif /* defined __GNUC__ && defined _WIN32 */
#endif /* !_MSC_VER */

#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define unlink _unlink
#else
#include <unistd.h> /* for unlink() */
#endif

#ifdef _WIN32 /* Windows including MSVC and MinGW */
#include <io.h> /* _get_osfhandle() */
/* NOTE: The header "fileapi.h" causes a compiler error due to an illegal anonymous union. */
#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINAPI __stdcall
#define S_OK 0
#define CSIDL_PROFILE 0x0028
#define CSIDL_COMMON_APPDATA 0x0023
#define SHGFP_TYPE_DEFAULT 1
#define MAX_PATH 260
typedef int BOOL;
typedef unsigned long DWORD;
typedef char *LPSTR;
typedef long HRESULT;
typedef void *HANDLE;
typedef void *HWND;
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;
DECLSPEC_IMPORT BOOL WINAPI GetFileInformationByHandle(HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation);
DECLSPEC_IMPORT HRESULT WINAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
#else /* !_WIN32 */
#include <sys/stat.h> /* for fstat() */
#endif

#if !defined __has_attribute || defined _MSC_VER
#define __attribute__(x)
#endif

#undef TRUE  /* to avoid macro definition conflicts with the system header file of IBM AIX */
#undef FALSE

#ifdef _MSC_VER
#define IMPORT_DIR_SYSTEM "packcc/import" /* should be a relative path */
#else
#define IMPORT_DIR_SYSTEM "/usr/share/packcc/import" /* should be an absolute path */
#endif

#define IMPORT_DIR_USER ".packcc/import"

#ifdef _WIN32 /* Windows including MSVC and MinGW (MinGW automatically converts paths to those in Windows style) */
#define PATH_SEP ';'
#else
#define PATH_SEP ':'
#endif

#define ENVVAR_IMPORT_PATH "PCC_IMPORT_PATH"

#define WEBSITE "https://github.com/arithy/packcc"

#define VERSION "3.0.0"

#ifndef BUFFER_MIN_SIZE
#define BUFFER_MIN_SIZE 256
#endif
#ifndef ARRAY_MIN_SIZE
#define ARRAY_MIN_SIZE 2
#endif

#define SUBST_MARKER '$'

#define VOID_VALUE (~(size_t)0)

#ifdef _WIN64 /* 64-bit Windows including MSVC and MinGW-w64 */
#define FMT_LU "%llu"
typedef unsigned long long ulong_t;
/* NOTE: "%llu" and "long long" are not C89-compliant, but we cannot help using them to deal with a 64-bit integer value in 64-bit Windows. */
#else
#define FMT_LU "%lu"
typedef unsigned long ulong_t;
#endif
/* FMT_LU and ulong_t are used to print size_t values safely (ex. printf(FMT_LU "\n", (ulong_t)value);) */
/* NOTE: Neither "%zu" nor <inttypes.h> is used since PackCC complies with the C89 standard as much as possible. */

typedef enum bool_tag {
    FALSE = 0,
    TRUE
} bool_t;

#ifdef _WIN32 /* Windows including MSVC and MinGW */
typedef BY_HANDLE_FILE_INFORMATION file_id_t;
#else
typedef struct stat file_id_t;
#endif

typedef struct file_id_array_tag {
    size_t m, n;
    file_id_t *p;
} file_id_array_t;

typedef struct file_pos_tag {
    char *path;  /* the file path name */
    size_t line; /* the line number (0-based); VOID_VALUE if not available */
    size_t col;  /* the column number (0-based); VOID_VALUE if not available */
} file_pos_t;

typedef struct stream_tag {
    FILE *file;       /* the file stream; just a reference */
    const char *path; /* the file path name */
    size_t line;      /* the current line number (0-based); line counting is disabled if VOID_VALUE */
} stream_t;

typedef struct char_array_tag {
    size_t m, n;
    char *p;
} char_array_t;

typedef struct string_array_tag {
    size_t m, n;
    char **p;
} string_array_t;

typedef struct code_block_tag {
    char *text;
    size_t len;
    file_pos_t fpos;
} code_block_t;

typedef struct code_block_array_tag {
    size_t m, n;
    code_block_t *p;
} code_block_array_t;

typedef enum node_type_tag {
    NODE_RULE = 0,
    NODE_REFERENCE,
    NODE_STRING,
    NODE_CHARCLASS,
    NODE_POSITION,
    NODE_QUANTITY,
    NODE_PREDICATE,
    NODE_PROGPRED,
    NODE_SEQUENCE,
    NODE_ALTERNATE,
    NODE_CAPTURE,
    NODE_EXPAND,
    NODE_ACTION,
    NODE_ERROR
} node_type_t;

typedef struct node_tag node_t;

typedef struct node_array_tag {
    size_t m, n;
    node_t **p;
} node_array_t;

typedef struct node_const_array_tag {
    size_t m, n;
    const node_t **p;
} node_const_array_t;

typedef struct node_hash_table_tag {
    size_t d;
    size_t m;
    const node_t **p;
} node_hash_table_t;

typedef struct node_rule_tag {
    char *name;
    node_t *expr;
    int ref; /* mutable under make_rulehash(), link_references(), and unreference_rules_from_unused_rule() */
    bool_t used; /* mutable under mark_rules_if_used() */
    node_const_array_t rvars;
    node_const_array_t capts;
    node_const_array_t codes;
    node_const_array_t preds;
    file_pos_t fpos;
} node_rule_t;

typedef struct node_reference_tag {
    char *rvar; /* NULL if no rule variable identifier */
    size_t index;
    char *name;
    const node_t *rule;
    file_pos_t fpos;
} node_reference_t;

typedef struct node_string_tag {
    char *value;
} node_string_t;

typedef struct node_charclass_tag {
    char *value; /* NULL means any character */
} node_charclass_t;

typedef struct node_position_tag {
    size_t value;
} node_position_t;

typedef struct node_quantity_tag {
    int min;
    int max;
    node_t *expr;
} node_quantity_t;

typedef struct node_predicate_tag {
    bool_t neg;
    node_t *expr;
} node_predicate_t;

typedef struct node_progpred_tag {
    bool_t neg;
    code_block_t code;
    size_t index;
    node_const_array_t capts;
} node_progpred_t;

typedef struct node_sequence_tag {
    node_array_t nodes;
} node_sequence_t;

typedef struct node_alternate_tag {
    node_array_t nodes;
} node_alternate_t;

typedef struct node_capture_tag {
    node_t *expr;
    size_t index;
} node_capture_t;

typedef struct node_expand_tag {
    size_t index;
    file_pos_t fpos;
} node_expand_t;

typedef struct node_action_tag {
    code_block_t code;
    size_t index;
    node_const_array_t rvars;
    node_const_array_t capts;
} node_action_t;

typedef struct node_error_tag {
    node_t *expr;
    code_block_t code;
    size_t index;
    node_const_array_t rvars;
    node_const_array_t capts;
} node_error_t;

typedef union node_data_tag {
    node_rule_t      rule;
    node_reference_t reference;
    node_string_t    string;
    node_charclass_t charclass;
    node_position_t  position;
    node_quantity_t  quantity;
    node_predicate_t predicate;
    node_progpred_t  progpred;
    node_sequence_t  sequence;
    node_alternate_t alternate;
    node_capture_t   capture;
    node_expand_t    expand;
    node_action_t    action;
    node_error_t     error;
} node_data_t;

struct node_tag {
    node_type_t type;
    node_data_t data;
};

typedef struct options_tag {
    bool_t ascii; /* UTF-8 support is disabled if true  */
    bool_t lines; /* #line directives are output if true */
    bool_t debug; /* debug information is output if true */
} options_t;

typedef enum code_flag_tag {
    CODE_FLAG__NONE = 0,
    CODE_FLAG__UTF8_CHARCLASS_USED = 1
} code_flag_t;

typedef struct subst_entry_tag {
    char *istr; /* the string to be replaced */
    char *ostr; /* the string to be used for replacement */
} subst_entry_t;

typedef struct subst_map_tag {
    size_t m, n;
    subst_entry_t *p;
} subst_map_t;

typedef struct input_state_tag input_state_t;

struct input_state_tag {
    char *path;        /* the path name of the PEG file being parsed; "<stdin>" if stdin */
    FILE *file;        /* the input file stream of the PEG file */
    bool_t ascii;      /* UTF-8 support is disabled if true  */
    code_flag_t flags; /* the bitwise flags to control code generation; updated during PEG parsing */
    size_t errnum;     /* the current number of PEG parsing errors */
    size_t linenum;    /* the current line number (0-based) */
    size_t charnum;    /* the number of characters in the current line that are already flushed (0-based, UTF-8 support if not disabled) */
    size_t linepos;    /* the beginning position in the PEG file of the current line */
    size_t bufpos;     /* the position in the PEG file of the first character currently buffered */
    size_t bufcur;     /* the current parsing position in the character buffer */
    char_array_t buffer;   /* the character buffer */
    input_state_t *parent; /* the input state of the parent PEG file that imports the input; just a reference */
};

typedef struct context_tag {
    char *spath;  /* the path name of the C source file being generated */
    char *hpath;  /* the path name of the C header file being generated */
    char *hid;    /* the macro name for the include guard of the C header file */
    char *vtype;  /* the type name of the data output by the parsing API function (NULL means the default) */
    char *atype;  /* the type name of the user-defined data passed to the parser creation API function (NULL means the default) */
    char *prefix; /* the prefix of the API function names (NULL means the default) */
    string_array_t mvars; /* the marker variable identifiers */
    const string_array_t *dirs; /* the path names of directories to search for import files */
    options_t opts;       /* the options */
    code_flag_t flags;    /* the bitwise flags to control code generation; updated during PEG parsing */
    size_t errnum;        /* the current number of PEG parsing errors */
    input_state_t *input; /* the current input state */
    file_id_array_t done; /* the unique identifiers of the PEG file already parsed or being parsed */
    subst_map_t subst;    /* the text substitution data */
    node_array_t rules;   /* the PEG rules */
    node_hash_table_t rulehash; /* the hash table to accelerate access of desired PEG rules */
    code_block_array_t esource; /* the code blocks from %earlysource and %earlycommon directives to be added into the generated source file */
    code_block_array_t eheader; /* the code blocks from %earlyheader and %earlycommon directives to be added into the generated header file */
    code_block_array_t source;  /* the code blocks from %source and %common directives to be added into the generated source file */
    code_block_array_t header;  /* the code blocks from %header and %common directives to be added into the generated header file */
    code_block_array_t fsource; /* the code fragments after %% directive to be added into the generated source file */
} context_t;

typedef struct generate_tag {
    stream_t *stream;
    const node_t *rule;
    int label;
    bool_t ascii;
} generate_t;

typedef enum string_flag_tag {
    STRING_FLAG__NONE = 0,
    STRING_FLAG__NOTEMPTY = 1,
    STRING_FLAG__NOTVOID = 2,
    STRING_FLAG__IDENTIFIER = 4
} string_flag_t;

typedef enum code_reach_tag {
    CODE_REACH__BOTH = 0,
    CODE_REACH__ALWAYS_SUCCEED = 1,
    CODE_REACH__ALWAYS_FAIL = -1
} code_reach_t;

static const char *g_cmdname = "packcc"; /* replaced later with actual one */

__attribute__((format(printf, 1, 2)))
static int print_error(const char *format, ...) {
    int n;
    va_list a;
    va_start(a, format);
    n = fprintf(stderr, "%s: ", g_cmdname);
    if (n >= 0) {
        const int k = vfprintf(stderr, format, a);
        if (k < 0) n = k; else n += k;
    }
    va_end(a);
    return n;
}

static FILE *fopen_rb_e(const char *path) {
    FILE *const f = fopen(path, "rb");
    if (f == NULL) {
        print_error("Cannot open file to read: %s\n", path);
        exit(2);
    }
    return f;
}

static FILE *fopen_wt_e(const char *path) {
    FILE *const f = fopen(path, "wt");
    if (f == NULL) {
        print_error("Cannot open file to write: %s\n", path);
        exit(2);
    }
    return f;
}

static int fclose_e(FILE *file, const char *path) {
    const int r = fclose(file);
    if (r == EOF) {
        print_error("File closing error: %s\n", path);
        exit(2);
    }
    return r;
}

static int fgetc_e(FILE *file, const char *path) {
    const int c = fgetc(file);
    if (c == EOF && ferror(file)) {
        print_error("File read error: %s\n", path);
        exit(2);
    }
    return c;
}

static void *malloc_e(size_t size) {
    void *const p = malloc(size);
    if (p == NULL) {
        print_error("Out of memory\n");
        exit(3);
    }
    return p;
}

static void *realloc_e(void *ptr, size_t size) {
    void *const p = realloc(ptr, size);
    if (p == NULL) {
        print_error("Out of memory\n");
        exit(3);
    }
    return p;
}

static char *strdup_e(const char *str) {
    const size_t m = strlen(str);
    char *const s = (char *)malloc_e(m + 1);
    memcpy(s, str, m);
    s[m] = '\0';
    return s;
}

static char *strndup_e(const char *str, size_t len) {
    const size_t m = strnlen(str, len);
    char *const s = (char *)malloc_e(m + 1);
    memcpy(s, str, m);
    s[m] = '\0';
    return s;
}

static char *to_uppercase(char *str) {
    size_t i;
    for (i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 'a' - 'A';
    }
    return str;
}

static size_t string_to_size_t(const char *str) {
#define N (~(size_t)0 / 10)
#define M (~(size_t)0 - 10 * N)
    size_t n = 0, i, k;
    for (i = 0; str[i]; i++) {
        const char c = str[i];
        if (c < '0' || c > '9') return VOID_VALUE;
        k = (size_t)(c - '0');
        if (n >= N && k > M) return VOID_VALUE; /* overflow */
        n = k + 10 * n;
    }
    return n;
#undef N
#undef M
}

static size_t find_first_trailing_space(const char *str, size_t start, size_t end, size_t *next) {
    size_t j = start, i;
    for (i = start; i < end; i++) {
        switch (str[i]) {
        case ' ':
        case '\v':
        case '\f':
        case '\t':
            continue;
        case '\n':
            if (next) *next = i + 1;
            return j;
        case '\r':
            if (i + 1 < end && str[i + 1] == '\n') i++;
            if (next) *next = i + 1;
            return j;
        default:
            j = i + 1;
        }
    }
    if (next) *next = end;
    return j;
}

static size_t count_indent_spaces(const char *str, size_t start, size_t end, size_t *next) {
    size_t n = 0, i;
    for (i = start; i < end; i++) {
        switch (str[i]) {
        case ' ':
        case '\v':
        case '\f':
            n++;
            break;
        case '\t':
            n = (n + 8) & ~7;
            break;
        default:
            if (next) *next = i;
            return n;
        }
    }
    if (next) *next = end;
    return n;
}

static bool_t is_filled_string(const char *str) {
    size_t i;
    for (i = 0; str[i]; i++) {
        if (
            str[i] != ' '  &&
            str[i] != '\v' &&
            str[i] != '\f' &&
            str[i] != '\t' &&
            str[i] != '\n' &&
            str[i] != '\r'
        ) return TRUE;
    }
    return FALSE;
}

static bool_t is_identifier_string(const char *str) {
    size_t i;
    if (!(
        (str[0] >= 'a' && str[0] <= 'z') ||
        (str[0] >= 'A' && str[0] <= 'Z') ||
        str[0] == '_'
    )) return FALSE;
    for (i = 1; str[i]; i++) {
        if (!(
            (str[i] >= 'a' && str[i] <= 'z') ||
            (str[i] >= 'A' && str[i] <= 'Z') ||
            (str[i] >= '0' && str[i] <= '9') ||
            str[i] == '_'
        )) return FALSE;
    }
    return TRUE;
}

static bool_t is_pointer_type(const char *str) {
    const size_t n = strlen(str);
    return (n > 0 && str[n - 1] == '*') ? TRUE : FALSE;
}

static bool_t is_valid_utf8_string(const char *str) {
    int k = 0, n = 0, u = 0;
    size_t i;
    for (i = 0; str[i]; i++) {
        const int c = (int)(unsigned char)str[i];
        switch (k) {
        case 0:
            if (c >= 0x80) {
                if ((c & 0xe0) == 0xc0) {
                    u = c & 0x1f;
                    n = k = 1;
                }
                else if ((c & 0xf0) == 0xe0) {
                    u = c & 0x0f;
                    n = k = 2;
                }
                else if ((c & 0xf8) == 0xf0) {
                    u = c & 0x07;
                    n = k = 3;
                }
                else {
                    return FALSE;
                }
            }
            break;
        case 1:
        case 2:
        case 3:
            if ((c & 0xc0) == 0x80) {
                u <<= 6;
                u |= c & 0x3f;
                k--;
                if (k == 0) {
                    switch (n) {
                    case 1:
                        if (u < 0x80) return FALSE;
                        break;
                    case 2:
                        if (u < 0x800) return FALSE;
                        break;
                    case 3:
                        if (u < 0x10000 || u > 0x10ffff) return FALSE;
                        break;
                    default:
                        assert(((void)"unexpected control flow", 0));
                        return FALSE; /* never reached */
                    }
                    u = 0;
                    n = 0;
                }
            }
            else {
                return FALSE;
            }
            break;
        default:
            assert(((void)"unexpected control flow", 0));
            return FALSE; /* never reached */
        }
    }
    return (k == 0) ? TRUE : FALSE;
}

static size_t utf8_to_utf32(const char *seq, int *out) { /* without checking UTF-8 validity */
    const int c = (int)(unsigned char)seq[0];
    const size_t n =
        (c == 0) ? 0 : (c < 0x80) ? 1 :
        ((c & 0xe0) == 0xc0) ? 2 :
        ((c & 0xf0) == 0xe0) ? 3 :
        ((c & 0xf8) == 0xf0) ? 4 : 1;
    int u = 0;
    switch (n) {
    case 0:
    case 1:
        u = c;
        break;
    case 2:
        u = ((c & 0x1f) << 6) |
            ((int)(unsigned char)seq[1] & 0x3f);
        break;
    case 3:
        u = ((c & 0x0f) << 12) |
            (((int)(unsigned char)seq[1] & 0x3f) << 6) |
            (seq[1] ? ((int)(unsigned char)seq[2] & 0x3f) : 0);
        break;
    default:
        u = ((c & 0x07) << 18) |
            (((int)(unsigned char)seq[1] & 0x3f) << 12) |
            (seq[1] ? (((int)(unsigned char)seq[2] & 0x3f) << 6) : 0) |
            (seq[2] ? ((int)(unsigned char)seq[3] & 0x3f) : 0);
    }
    if (out) *out = u;
    return n;
}

static bool_t unescape_string(char *str, bool_t cls) { /* cls: TRUE if used for character class matching */
    bool_t b = TRUE;
    size_t i, j;
    for (j = 0, i = 0; str[i]; i++) {
        if (str[i] == '\\') {
            i++;
            switch (str[i]) {
            case '\0': str[j++] = '\\'; str[j] = '\0'; return FALSE;
            case '\'': str[j++] = '\''; break;
            case '\"': str[j++] = '\"'; break;
            case '0': str[j++] = '\x00'; break;
            case 'a': str[j++] = '\x07'; break;
            case 'b': str[j++] = '\x08'; break;
            case 'f': str[j++] = '\x0c'; break;
            case 'n': str[j++] = '\x0a'; break;
            case 'r': str[j++] = '\x0d'; break;
            case 't': str[j++] = '\x09'; break;
            case 'v': str[j++] = '\x0b'; break;
            case 'x':
                {
                    char s = 0, c;
                    size_t k;
                    for (k = 0; k < 2; k++) {
                        char d;
                        c = str[i + k + 1];
                        d = (c >= '0' && c <= '9') ? c - '0' :
                            (c >= 'a' && c <= 'f') ? c - 'a' + 10 :
                            (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                        if (d < 0) break;
                        s = (s << 4) | d;
                    }
                    if (k < 2) {
                        const size_t l = i + k;
                        str[j++] = '\\'; str[j++] = 'x';
                        while (i <= l) str[j++] = str[++i];
                        if (c == '\0') return FALSE;
                        b = FALSE;
                        continue;
                    }
                    str[j++] = s;
                    i += 2;
                }
                break;
            case 'u':
                {
                    int s = 0, t = 0;
                    char c;
                    size_t k;
                    for (k = 0; k < 4; k++) {
                        char d;
                        c = str[i + k + 1];
                        d = (c >= '0' && c <= '9') ? c - '0' :
                            (c >= 'a' && c <= 'f') ? c - 'a' + 10 :
                            (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                        if (d < 0) break;
                        s = (s << 4) | d;
                    }
                    if (k < 4 || (s & 0xfc00) == 0xdc00) { /* invalid character or invalid surrogate code point */
                        const size_t l = i + k;
                        str[j++] = '\\'; str[j++] = 'u';
                        while (i <= l) str[j++] = str[++i];
                        if (c == '\0') return FALSE;
                        b = FALSE;
                        continue;
                    }
                    if ((s & 0xfc00) == 0xd800) { /* surrogate pair */
                        for (k = 4; k < 10; k++) {
                            c = str[i + k + 1];
                            if (k == 4) {
                                if (c != '\\') break;
                            }
                            else if (k == 5) {
                                if (c != 'u') break;
                            }
                            else {
                                const char d =
                                    (c >= '0' && c <= '9') ? c - '0' :
                                    (c >= 'a' && c <= 'f') ? c - 'a' + 10 :
                                    (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                                if (d < 0) break;
                                t = (t << 4) | d;
                            }
                        }
                        if (k < 10 || (t & 0xfc00) != 0xdc00) { /* invalid character or invalid surrogate code point */
                            const size_t l = i + 4; /* NOTE: Not i + k to redo with recovery. */
                            str[j++] = '\\'; str[j++] = 'u';
                            while (i <= l) str[j++] = str[++i];
                            b = FALSE;
                            continue;
                        }
                    }
                    {
                        const int u = t ? ((((s & 0x03ff) + 0x0040) << 10) | (t & 0x03ff)) : s;
                        if (u < 0x0080) {
                            str[j++] = (char)u;
                        }
                        else if (u < 0x0800) {
                            str[j++] = (char)(0xc0 | (u >> 6));
                            str[j++] = (char)(0x80 | (u & 0x3f));
                        }
                        else if (u < 0x010000) {
                            str[j++] = (char)(0xe0 | (u >> 12));
                            str[j++] = (char)(0x80 | ((u >> 6) & 0x3f));
                            str[j++] = (char)(0x80 | (u & 0x3f));
                        }
                        else if (u < 0x110000) {
                            str[j++] = (char)(0xf0 | (u >> 18));
                            str[j++] = (char)(0x80 | ((u >> 12) & 0x3f));
                            str[j++] = (char)(0x80 | ((u >>  6) & 0x3f));
                            str[j++] = (char)(0x80 | (u & 0x3f));
                        }
                        else { /* never reached theoretically; in case */
                            const size_t l = i + 10;
                            str[j++] = '\\'; str[j++] = 'u';
                            while (i <= l) str[j++] = str[++i];
                            b = FALSE;
                            continue;
                        }
                    }
                    i += t ? 10 : 4;
                }
                break;
            case '\n': break;
            case '\r': if (str[i + 1] == '\n') i++; break;
            case '\\':
                if (cls) str[j++] = '\\'; /* left for character class matching (ex. considering [\^\]\\]) */
                str[j++] = '\\';
                break;
            default: str[j++] = '\\'; str[j++] = str[i];
            }
        }
        else {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
    return b;
}

static const char *escape_character(char ch, char (*buf)[5]) {
    switch (ch) {
    case '\x00': strncpy(*buf, "\\0", 5); break;
    case '\x07': strncpy(*buf, "\\a", 5); break;
    case '\x08': strncpy(*buf, "\\b", 5); break;
    case '\x0c': strncpy(*buf, "\\f", 5); break;
    case '\x0a': strncpy(*buf, "\\n", 5); break;
    case '\x0d': strncpy(*buf, "\\r", 5); break;
    case '\x09': strncpy(*buf, "\\t", 5); break;
    case '\x0b': strncpy(*buf, "\\v", 5); break;
    case '\\':  strncpy(*buf, "\\\\", 5); break;
    case '\'':  strncpy(*buf, "\\\'", 5); break;
    case '\"':  strncpy(*buf, "\\\"", 5); break;
    default:
        if (ch >= '\x20' && ch < '\x7f')
            snprintf(*buf, 5, "%c", ch);
        else
            snprintf(*buf, 5, "\\x%02x", (int)(unsigned char)ch);
    }
    (*buf)[4] = '\0';
    return *buf;
}

static void remove_leading_blanks(char *str) {
    size_t i, j;
    for (i = 0; str[i]; i++) {
        if (
            str[i] != ' '  &&
            str[i] != '\v' &&
            str[i] != '\f' &&
            str[i] != '\t' &&
            str[i] != '\n' &&
            str[i] != '\r'
        ) break;
    }
    for (j = 0; str[i]; i++) {
        str[j++] = str[i];
    }
    str[j] = '\0';
}

static void remove_trailing_blanks(char *str) {
    size_t i, j;
    for (j = 0, i = 0; str[i]; i++) {
        if (
            str[i] != ' '  &&
            str[i] != '\v' &&
            str[i] != '\f' &&
            str[i] != '\t' &&
            str[i] != '\n' &&
            str[i] != '\r'
        ) j = i + 1;
    }
    str[j] = '\0';
}

static size_t find_trailing_blanks(const char *str) {
    size_t i, j;
    for (j = 0, i = 0; str[i]; i++) {
        if (
            str[i] != ' '  &&
            str[i] != '\v' &&
            str[i] != '\f' &&
            str[i] != '\t' &&
            str[i] != '\n' &&
            str[i] != '\r'
        ) j = i + 1;
    }
    return j;
}

static size_t count_characters(const char *str, size_t start, size_t end) {
    /* UTF-8 multibyte character support but without checking UTF-8 validity */
    size_t n = 0, i = start;
    while (i < end) {
        const int c = (int)(unsigned char)str[i];
        if (c == 0) break;
        n++;
        i += (c < 0x80) ? 1 : ((c & 0xe0) == 0xc0) ? 2 : ((c & 0xf0) == 0xe0) ? 3 : ((c & 0xf8) == 0xf0) ? 4 : /* invalid code */ 1;
    }
    return n;
}

static void make_header_identifier(char *str) {
    size_t i;
    for (i = 0; str[i]; i++) {
        str[i] =
            ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9')) ? str[i] :
            (str[i] >= 'a' && str[i] <= 'z') ? str[i] - 'a' + 'A' : '_';
    }
}

static void file_pos__initialize(file_pos_t *obj) {
    obj->path = NULL;
    obj->line = VOID_VALUE;
    obj->col = VOID_VALUE;
}

static void file_pos__finalize(file_pos_t *obj) {
    free(obj->path);
}

static void file_pos__set(file_pos_t *obj, const char *path, size_t line, size_t col) {
    free(obj->path);
    obj->path = path ? strdup_e(path) : NULL;
    obj->line = line;
    obj->col = col;
}

static void file_id__get(FILE *file, const char *path, file_id_t *id) {
#ifdef _WIN32 /* Windows including MSVC and MinGW */
    if (GetFileInformationByHandle((HANDLE)_get_osfhandle(_fileno(file)), id) == 0) {
        print_error("Cannot get file information: %s\n", path);
        exit(2);
    }
#else
    if (fstat(fileno(file), id) != 0) {
        print_error("Cannot get file information: %s\n", path);
        exit(2);
    }
#endif
}

static bool_t file_id__equals(const file_id_t *id0, const file_id_t *id1) {
#ifdef _WIN32 /* Windows including MSVC and MinGW */
    return (
        id0->dwVolumeSerialNumber == id1->dwVolumeSerialNumber &&
        id0->nFileIndexHigh == id1->nFileIndexHigh &&
        id0->nFileIndexLow == id1->nFileIndexLow
    ) ? TRUE : FALSE;
#else
    return (id0->st_dev == id1->st_dev && id0->st_ino == id1->st_ino) ? TRUE : FALSE;
#endif
}

static stream_t stream__wrap(FILE *file, const char *path, size_t line) {
    stream_t obj;
    obj.file = file;
    obj.path = path;
    obj.line = line;
    return obj;
}

static int stream__putc(stream_t *obj, int c) {
    const int r = fputc(c, obj->file);
    if (r == EOF) {
        print_error("File write error: %s\n", obj->path);
        exit(2);
    }
    if (obj->line != VOID_VALUE) {
        if (c == '\n') obj->line++;
    }
    return r;
}

static int stream__puts(stream_t *obj, const char *s) {
    const int r = fputs(s, obj->file);
    if (r == EOF) {
        print_error("File write error: %s\n", obj->path);
        exit(2);
    }
    if (obj->line != VOID_VALUE) {
        size_t i = 0;
        for (i = 0; s[i]; i++) {
            if (s[i] == '\n') obj->line++;
        }
    }
    return r;
}

__attribute__((format(printf, 2, 3)))
static int stream__printf(stream_t *obj, const char *format, ...) {
    if (obj->line != VOID_VALUE) {
#define M 1024
        char s[M], *p = NULL;
        int n = 0;
        size_t l = 0;
        {
            va_list a;
            va_start(a, format);
            n = vsnprintf(NULL, 0, format, a);
            va_end(a);
            if (n < 0) {
                print_error("Internal error [%d]\n", __LINE__);
                exit(2);
            }
            l = (size_t)n + 1;
        }
        p = (l > M) ? (char *)malloc_e(l) : s;
        {
            va_list a;
            va_start(a, format);
            n = vsnprintf(p, l, format, a);
            va_end(a);
            if (n < 0 || (size_t)n >= l) {
                print_error("Internal error [%d]\n", __LINE__);
                exit(2);
            }
        }
        stream__puts(obj, p);
        if (p != s) free(p);
        return n;
#undef M
    }
    else {
        int n;
        va_list a;
        va_start(a, format);
        n = vfprintf(obj->file, format, a);
        va_end(a);
        if (n < 0) {
            print_error("File write error: %s\n", obj->path);
            exit(2);
        }
        return n;
    }
}

static void stream__write_characters(stream_t *obj, char ch, size_t len) {
    size_t i;
    if (len == VOID_VALUE) return; /* for safety */
    for (i = 0; i < len; i++) stream__putc(obj, ch);
}

static bool_t can_character_follow_reserved_identifier_(char ch) {
    return (
        ch != SUBST_MARKER && ch != '_' && !(ch >= 'A' && ch <= 'Z') && !(ch >= 'a' && ch <= 'z') && !(ch >= '0' && ch<= '9')
    ) ? TRUE : FALSE;
}

static void stream__write_text(stream_t *obj, const char *ptr, size_t len, bool_t isact, const subst_map_t *subst) {
    size_t i;
    if (len == VOID_VALUE) return; /* for safety */
    for (i = 0; i < len; i++) {
        if (ptr[i] == '\r') {
            if (i + 1 < len && ptr[i + 1] == '\n') i++;
            stream__putc(obj, '\n');
        }
        else if (ptr[i] == SUBST_MARKER) {
            bool_t b = FALSE;
            size_t j = i + 1;
            if (j < len) {
                char c = ptr[j++];
                if (isact && c == SUBST_MARKER) { /* "$$" */
                    if (j >= len || can_character_follow_reserved_identifier_(ptr[j])) {
                        stream__putc(obj, '_');
                        stream__putc(obj, '_');
                        b = TRUE; i++;
                    }
                }
                else if (isact && c == '0') { /* "$0" */
                    if (j < len && (ptr[j] == 's' || ptr[j] == 'e')) j++;
                    if (j >= len || can_character_follow_reserved_identifier_(ptr[j])) {
                        stream__putc(obj, '_');
                        while (i + 1 < j) stream__putc(obj, ptr[++i]);
                        b = TRUE;
                    }
                }
                else if (isact && c >= '1' && c <= '9') { /* "$n" (n: a string to express a natural number) */
                    while (j < len && ptr[j] >= '0' && ptr[j] <= '9') j++;
                    if (j < len && (ptr[j] == 's' || ptr[j] == 'e')) j++;
                    if (j >= len || can_character_follow_reserved_identifier_(ptr[j])) {
                        stream__putc(obj, '_');
                        while (i + 1 < j) stream__putc(obj, ptr[++i]);
                        b = TRUE;
                    }
                }
                else if (subst && c == '{') { /* "${str}" (str: a string to be replaced) */
                    size_t is;
                    for (is = 0; is < subst->n; is++) {
                        const subst_entry_t *const p = &(subst->p[is]);
                        size_t k;
                        for (k = 0; p->istr[k] && j + k < len && p->istr[k] == ptr[j + k]; k++);
                        if (!p->istr[k] && j + k < len && ptr[j + k] == '}') {
                            size_t l;
                            for (l = 0; p->ostr[l]; l++) stream__putc(obj, p->ostr[l]);
                            b = TRUE; i = j + k;
                            break;
                        }
                    }
                }
            }
            if (!b) stream__putc(obj, '$');
        }
        else {
            stream__putc(obj, ptr[i]);
        }
    }
}

static void stream__write_escaped_string(stream_t *obj, const char *ptr, size_t len) {
    char s[5];
    size_t i;
    if (len == VOID_VALUE) return; /* for safety */
    for (i = 0; i < len; i++) {
        stream__puts(obj, escape_character(ptr[i], &s));
    }
}

static void stream__write_line_directive(stream_t *obj, const char *path, size_t lineno) {
    stream__printf(obj, "#line " FMT_LU " \"", (ulong_t)(lineno + 1));
    stream__write_escaped_string(obj, path, strlen(path));
    stream__puts(obj, "\"\n");
}

static void stream__write_code_block(
    stream_t *obj, const char *ptr, size_t len, size_t indent, const char *path, size_t lineno, bool_t isact, const subst_map_t *subst
) {
    bool_t b = FALSE;
    size_t i, j, k;
    if (len == VOID_VALUE) return; /* for safety */
    j = find_first_trailing_space(ptr, 0, len, &k);
    for (i = 0; i < j; i++) {
        if (
            ptr[i] != ' '  &&
            ptr[i] != '\v' &&
            ptr[i] != '\f' &&
            ptr[i] != '\t'
        ) break;
    }
    if (i < j) {
        if (obj->line != VOID_VALUE)
            stream__write_line_directive(obj, path, lineno);
        if (ptr[i] != '#')
            stream__write_characters(obj, ' ', indent);
        stream__write_text(obj, ptr + i, j - i, isact, subst);
        stream__putc(obj, '\n');
        b = TRUE;
    }
    else {
        lineno++;
    }
    if (k < len) {
        size_t m = VOID_VALUE;
        size_t h;
        for (i = k; i < len; i = h) {
            j = find_first_trailing_space(ptr, i, len, &h);
            if (i < j) {
                if (obj->line != VOID_VALUE && !b)
                    stream__write_line_directive(obj, path, lineno);
                if (ptr[i] != '#') {
                    const size_t l = count_indent_spaces(ptr, i, j, NULL);
                    if (m == VOID_VALUE || m > l) m = l;
                }
                b = TRUE;
            }
            else {
                if (!b) {
                    k = h;
                    lineno++;
                }
            }
        }
        for (i = k; i < len; i = h) {
            j = find_first_trailing_space(ptr, i, len, &h);
            if (i < j) {
                const size_t l = count_indent_spaces(ptr, i, j, &i);
                if (ptr[i] != '#') {
                    assert(m != VOID_VALUE); /* m must have a valid value */
                    assert(l >= m);
                    stream__write_characters(obj, ' ', l - m + indent);
                }
                stream__write_text(obj, ptr + i, j - i, isact, subst);
                stream__putc(obj, '\n');
                b = TRUE;
            }
            else if (h < len) {
                stream__putc(obj, '\n');
            }
        }
    }
    if (obj->line != VOID_VALUE && b)
        stream__write_line_directive(obj, obj->path, obj->line);
}

static void stream__write_footer(stream_t *obj, const char *ptr, size_t len, const char *path, size_t lineno, const subst_map_t *subst) {
    bool_t b = FALSE;
    size_t i, j, k;
    if (len == VOID_VALUE) return; /* for safety */
    j = find_first_trailing_space(ptr, 0, len, &k);
    for (i = 0; i < j; i++) {
        if (
            ptr[i] != ' '  &&
            ptr[i] != '\v' &&
            ptr[i] != '\f' &&
            ptr[i] != '\t'
        ) break;
    }
    if (i < j) {
        if (obj->line != VOID_VALUE)
            stream__write_line_directive(obj, path, lineno);
        stream__write_text(obj, ptr + i, j - i, FALSE, subst);
        stream__putc(obj, '\n');
        b = TRUE;
    }
    else {
        lineno++;
    }
    if (k < len) {
        size_t h;
        for (i = k; i < len; i = h) {
            j = find_first_trailing_space(ptr, i, len, &h);
            if (i < j) {
                if (obj->line != VOID_VALUE && !b)
                    stream__write_line_directive(obj, path, lineno);
                b = TRUE;
                break;
            }
            else {
                if (!b) {
                    k = h;
                    lineno++;
                }
            }
        }
        for (i = k; i < len; i = h) {
            j = find_first_trailing_space(ptr, i, len, &h);
            if (i < j) {
                stream__write_text(obj, ptr + i, j - i, FALSE, subst);
                stream__putc(obj, '\n');
            }
            else if (h < len) {
                stream__putc(obj, '\n');
            }
        }
    }
}

static char *get_home_directory(void) {
#ifdef _MSC_VER
    char s[MAX_PATH];
    return (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_DEFAULT, s) == S_OK) ? strdup_e(s) : NULL;
#else
    const char *const s = getenv("HOME");
    return (s && s[0]) ? strdup_e(s) : NULL;
#endif
}

#ifdef _MSC_VER

static char *get_appdata_directory(void) {
    char s[MAX_PATH];
    return (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, s) == S_OK) ? strdup_e(s) : NULL;
}

#endif /* _MSC_VER */

static bool_t is_absolute_path(const char *path) {
#ifdef _WIN32
    return (
        path[0] == '\\' ||
        (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':')
    ) ? TRUE : FALSE;
#else
    return (path[0] == '/') ? TRUE : FALSE;
#endif
}

static const char *extract_filename(const char *path) {
    size_t i = strlen(path);
    while (i > 0) {
        i--;
#ifdef _WIN32
        if (strchr("/\\:", path[i])) return path + i + 1;
#else
        if (path[i] == '/') return path + i + 1;
#endif
    }
    return path;
}

static char *replace_filename(const char *path, const char *name) {
    const char *const p = extract_filename(path);
    const size_t m = p - path;
    const size_t n = strlen(name);
    char *const s = (char *)malloc_e(m + n + 1);
    memcpy(s, path, m);
    memcpy(s + m, name, n + 1);
    return s;
}

static char *add_filename(const char *path, const char *name) {
    const size_t m = strlen(path);
    const size_t n = strlen(name);
#ifdef _WIN32
    const size_t d = (m > 0 && strchr("/\\:", path[m - 1]) == NULL) ? 1 : 0;
#else
    const size_t d = (m > 0 && path[m - 1] != '/') ? 1 : 0;
#endif
    char *const s = (char *)malloc_e(m + d + n + 1);
    memcpy(s, path, m);
    if (d) s[m] = '/';
    memcpy(s + m + d, name, n + 1);
    return s;
}

static const char *extract_fileext(const char *path) {
    const size_t n = strlen(path);
    size_t i = n;
    while (i > 0) {
        i--;
#ifdef _WIN32
        if (strchr("/\\:", path[i])) break;
#else
        if (path[i] == '/') break;
#endif
        if (path[i] == '.') return path + i;
    }
    return path + n;
}

static char *replace_fileext(const char *path, const char *ext) {
    const char *const p = extract_fileext(path);
    const size_t m = p - path;
    const size_t n = strlen(ext);
    char *const s = (char *)malloc_e(m + n + 2);
    memcpy(s, path, m);
    s[m] = '.';
    memcpy(s + m + 1, ext, n + 1);
    return s;
}

static char *add_fileext(const char *path, const char *ext) {
    const size_t m = strlen(path);
    const size_t n = strlen(ext);
    char *const s = (char *)malloc_e(m + n + 2);
    memcpy(s, path, m);
    s[m] = '.';
    memcpy(s + m + 1, ext, n + 1);
    return s;
}

static size_t hash_string(const char *str) {
    size_t i, h = 0;
    for (i = 0; str[i]; i++) {
        h = h * 31 + str[i];
    }
    return h;
}

static size_t populate_bits(size_t x) {
    x |= x >>  1;
    x |= x >>  2;
    x |= x >>  4;
    x |= x >>  8;
    x |= x >> 16;
#if (defined __SIZEOF_SIZE_T__ && __SIZEOF_SIZE_T__ == 8) /* gcc or clang */ || defined _WIN64 /* MSVC */
    x |= x >> 32;
#endif
    return x;
}

static size_t column_number(const input_state_t *input) { /* 0-based */
    assert(input->bufpos + input->bufcur >= input->linepos);
    if (input->ascii)
        return input->charnum + input->bufcur - ((input->linepos > input->bufpos) ? input->linepos - input->bufpos : 0);
    else
        return input->charnum + count_characters(
            input->buffer.p, (input->linepos > input->bufpos) ? input->linepos - input->bufpos : 0, input->bufcur
        );
}

static void file_id_array__initialize(file_id_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void file_id_array__finalize(file_id_array_t *obj) {
    free(obj->p);
}

static bool_t file_id_array__add_if_not_yet(file_id_array_t *obj, const file_id_t *id) {
    size_t i;
    for (i = 0; i < obj->n; i++) {
        if (file_id__equals(id, &(obj->p[i]))) return FALSE; /* already added */
    }
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = BUFFER_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (file_id_t *)realloc_e(obj->p, sizeof(file_id_t) * m);
        obj->m = m;
    }
    obj->p[obj->n++] = *id;
    return TRUE; /* newly added */
}

static void char_array__initialize(char_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void char_array__finalize(char_array_t *obj) {
    free(obj->p);
}

static void char_array__add(char_array_t *obj, char ch) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = BUFFER_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (char *)realloc_e(obj->p, m);
        obj->m = m;
    }
    obj->p[obj->n++] = ch;
}

static void string_array__initialize(string_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void string_array__finalize(string_array_t *obj) {
    size_t i;
    for (i = 0; i < obj->n; i++) free(obj->p[i]);
    free(obj->p);
}

static void string_array__add(string_array_t *obj, const char *str, size_t len) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = BUFFER_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (char **)realloc_e(obj->p, sizeof(char *) * m);
        obj->m = m;
    }
    obj->p[obj->n++] = (len == VOID_VALUE) ? strdup_e(str) : strndup_e(str, len);
}

static void code_block__initialize(code_block_t *obj) {
    obj->text = NULL;
    obj->len = 0;
    file_pos__initialize(&(obj->fpos));
}

static void code_block__finalize(code_block_t *obj) {
    file_pos__finalize(&(obj->fpos));
    free(obj->text);
}

static void code_block_array__initialize(code_block_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void code_block_array__finalize(code_block_array_t *obj) {
    while (obj->n > 0) {
        obj->n--;
        code_block__finalize(&(obj->p[obj->n]));
    }
    free(obj->p);
}

static code_block_t *code_block_array__create_entry(code_block_array_t *obj) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (code_block_t *)realloc_e(obj->p, sizeof(code_block_t) * m);
        obj->m = m;
    }
    code_block__initialize(&(obj->p[obj->n]));
    return &(obj->p[obj->n++]);
}

static void destroy_node(node_t *node);

static void node_array__initialize(node_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void node_array__finalize(node_array_t *obj) {
    while (obj->n > 0) {
        obj->n--;
        destroy_node(obj->p[obj->n]);
    }
    free(obj->p);
}

static void node_array__add(node_array_t *obj, node_t *node) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (node_t **)realloc_e(obj->p, sizeof(node_t *) * m);
        obj->m = m;
    }
    obj->p[obj->n++] = node;
}

static void node_const_array__initialize(node_const_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void node_const_array__finalize(node_const_array_t *obj) {
    free((node_t **)obj->p);
}

static void node_const_array__clear(node_const_array_t *obj) {
    obj->n = 0;
}

static void node_const_array__add(node_const_array_t *obj, const node_t *node) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (const node_t **)realloc_e((node_t **)obj->p, sizeof(const node_t *) * m);
        obj->m = m;
    }
    obj->p[obj->n++] = node;
}

static void node_const_array__copy(node_const_array_t *obj, const node_const_array_t *src) {
    size_t i;
    if (obj == src) return;
    node_const_array__clear(obj);
    for (i = 0; i < src->n; i++) {
        node_const_array__add(obj, src->p[i]);
    }
}

static void subst_map__initialize(subst_map_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

static void subst_map__finalize(subst_map_t *obj) {
    while (obj->n > 0) {
        subst_entry_t *const p = &(obj->p[--obj->n]);
        free(p->istr);
        free(p->ostr);
    }
    free(obj->p);
}

static void subst_map__clear(subst_map_t *obj) {
    obj->n = 0;
}

static void subst_map__add(subst_map_t *obj, const char *istr, const char *ostr) {
    if (obj->m <= obj->n) {
        const size_t n = obj->n + 1;
        size_t m = obj->m;
        if (m == 0) m = ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n; /* in case of shift overflow */
        obj->p = (subst_entry_t *)realloc_e(obj->p, sizeof(subst_entry_t) * m);
        obj->m = m;
    }
    {
        subst_entry_t *const p = &(obj->p[obj->n++]);
        p->istr = strdup_e(istr);
        p->ostr = strdup_e(ostr);
    }
}

static input_state_t *input_state__create(const char *path, FILE *file, input_state_t *parent, const options_t *opts) {
    input_state_t *const obj = (input_state_t *)malloc_e(sizeof(input_state_t));
    obj->path = strdup_e((path && path[0]) ? path : "<stdin>");
    obj->file = file ? file : (path && path[0]) ? fopen_rb_e(path) : stdin;
    obj->ascii = opts->ascii;
    obj->flags = CODE_FLAG__NONE;
    obj->errnum = 0;
    obj->linenum = 0;
    obj->charnum = 0;
    obj->linepos = 0;
    obj->bufpos = 0;
    obj->bufcur = 0;
    char_array__initialize(&(obj->buffer));
    obj->parent = parent;
    return obj;
}

static input_state_t *input_state__destroy(input_state_t *obj) {
    input_state_t *parent;
    if (obj == NULL) return NULL;
    parent = obj->parent;
    char_array__finalize(&(obj->buffer));
    fclose_e(obj->file, obj->path);
    free(obj->path);
    free(obj);
    return parent;
}

static bool_t input_state__is_in_imported(const input_state_t *obj) {
    return obj->parent ? TRUE : FALSE;
}

static context_t *create_context(const char *ipath, const char *opath, const string_array_t *dirs, const options_t *opts) {
    context_t *const ctx = (context_t *)malloc_e(sizeof(context_t));
    ctx->spath = (opath && opath[0]) ? add_fileext(opath, "c") : replace_fileext((ipath && ipath[0]) ? ipath : "-", "c");
    ctx->hpath = (opath && opath[0]) ? add_fileext(opath, "h") : replace_fileext((ipath && ipath[0]) ? ipath : "-", "h");
    ctx->hid = strdup_e(extract_filename(ctx->hpath)); make_header_identifier(ctx->hid);
    ctx->vtype = NULL;
    ctx->atype = NULL;
    ctx->prefix = NULL;
    string_array__initialize(&(ctx->mvars));
    ctx->dirs = dirs;
    ctx->opts = *opts;
    ctx->flags = CODE_FLAG__NONE;
    ctx->errnum = 0;
    ctx->input = input_state__create(ipath, NULL, NULL, opts);
    file_id_array__initialize(&(ctx->done));
    subst_map__initialize(&(ctx->subst));
    node_array__initialize(&(ctx->rules));
    ctx->rulehash.d = 0;
    ctx->rulehash.m = 0;
    ctx->rulehash.p = NULL;
    code_block_array__initialize(&(ctx->esource));
    code_block_array__initialize(&(ctx->eheader));
    code_block_array__initialize(&(ctx->source));
    code_block_array__initialize(&(ctx->header));
    code_block_array__initialize(&(ctx->fsource));
    return ctx;
}

static void destroy_context(context_t *ctx) {
    if (ctx == NULL) return;
    free(ctx->spath);
    free(ctx->hpath);
    free(ctx->hid);
    free(ctx->vtype);
    free(ctx->atype);
    free(ctx->prefix);
    string_array__finalize(&(ctx->mvars));
    while (ctx->input) ctx->input = input_state__destroy(ctx->input);
    file_id_array__finalize(&(ctx->done));
    subst_map__finalize(&(ctx->subst));
    node_array__finalize(&(ctx->rules));
    free((node_t **)ctx->rulehash.p);
    code_block_array__finalize(&(ctx->esource));
    code_block_array__finalize(&(ctx->eheader));
    code_block_array__finalize(&(ctx->source));
    code_block_array__finalize(&(ctx->header));
    code_block_array__finalize(&(ctx->fsource));
    free(ctx);
}

static node_t *create_node(node_type_t type) {
    node_t *const node = (node_t *)malloc_e(sizeof(node_t));
    node->type = type;
    switch (node->type) {
    case NODE_RULE:
        node->data.rule.name = NULL;
        node->data.rule.expr = NULL;
        node->data.rule.ref = 0;
        node->data.rule.used = FALSE;
        node_const_array__initialize(&(node->data.rule.rvars));
        node_const_array__initialize(&(node->data.rule.capts));
        node_const_array__initialize(&(node->data.rule.codes));
        node_const_array__initialize(&(node->data.rule.preds));
        file_pos__initialize(&(node->data.rule.fpos));
        break;
    case NODE_REFERENCE:
        node->data.reference.rvar = NULL;
        node->data.reference.index = VOID_VALUE;
        node->data.reference.name = NULL;
        node->data.reference.rule = NULL;
        file_pos__initialize(&(node->data.reference.fpos));
        break;
    case NODE_STRING:
        node->data.string.value = NULL;
        break;
    case NODE_CHARCLASS:
        node->data.charclass.value = NULL;
        break;
    case NODE_POSITION:
        node->data.position.value = 0;
        break;
    case NODE_QUANTITY:
        node->data.quantity.min = node->data.quantity.max = 0;
        node->data.quantity.expr = NULL;
        break;
    case NODE_PREDICATE:
        node->data.predicate.neg = FALSE;
        node->data.predicate.expr = NULL;
        break;
    case NODE_PROGPRED:
        node->data.progpred.neg = FALSE;
        code_block__initialize(&(node->data.progpred.code));
        node->data.progpred.index = VOID_VALUE;
        node_const_array__initialize(&(node->data.progpred.capts));
        break;
    case NODE_SEQUENCE:
        node_array__initialize(&(node->data.sequence.nodes));
        break;
    case NODE_ALTERNATE:
        node_array__initialize(&(node->data.alternate.nodes));
        break;
    case NODE_CAPTURE:
        node->data.capture.expr = NULL;
        node->data.capture.index = VOID_VALUE;
        break;
    case NODE_EXPAND:
        node->data.expand.index = VOID_VALUE;
        file_pos__initialize(&(node->data.expand.fpos));
        break;
    case NODE_ACTION:
        code_block__initialize(&(node->data.action.code));
        node->data.action.index = VOID_VALUE;
        node_const_array__initialize(&(node->data.action.rvars));
        node_const_array__initialize(&(node->data.action.capts));
        break;
    case NODE_ERROR:
        node->data.error.expr = NULL;
        code_block__initialize(&(node->data.error.code));
        node->data.error.index = VOID_VALUE;
        node_const_array__initialize(&(node->data.error.rvars));
        node_const_array__initialize(&(node->data.error.capts));
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
    return node;
}

static void destroy_node(node_t *node) {
    if (node == NULL) return;
    switch (node->type) {
    case NODE_RULE:
        free(node->data.rule.name);
        destroy_node(node->data.rule.expr);
        node_const_array__finalize(&(node->data.rule.rvars));
        node_const_array__finalize(&(node->data.rule.capts));
        node_const_array__finalize(&(node->data.rule.codes));
        node_const_array__finalize(&(node->data.rule.preds));
        file_pos__finalize(&(node->data.rule.fpos));
        break;
    case NODE_REFERENCE:
        free(node->data.reference.rvar);
        free(node->data.reference.name);
        file_pos__finalize(&(node->data.reference.fpos));
        break;
    case NODE_STRING:
        free(node->data.string.value);
        break;
    case NODE_CHARCLASS:
        free(node->data.charclass.value);
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        destroy_node(node->data.quantity.expr);
        break;
    case NODE_PREDICATE:
        destroy_node(node->data.predicate.expr);
        break;
    case NODE_PROGPRED:
        code_block__finalize(&(node->data.progpred.code));
        node_const_array__finalize(&(node->data.progpred.capts));
        break;
    case NODE_SEQUENCE:
        node_array__finalize(&(node->data.sequence.nodes));
        break;
    case NODE_ALTERNATE:
        node_array__finalize(&(node->data.alternate.nodes));
        break;
    case NODE_CAPTURE:
        destroy_node(node->data.capture.expr);
        break;
    case NODE_EXPAND:
        file_pos__finalize(&(node->data.expand.fpos));
        break;
    case NODE_ACTION:
        code_block__finalize(&(node->data.action.code));
        node_const_array__finalize(&(node->data.action.rvars));
        node_const_array__finalize(&(node->data.action.capts));
        break;
    case NODE_ERROR:
        destroy_node(node->data.error.expr);
        code_block__finalize(&(node->data.error.code));
        node_const_array__finalize(&(node->data.error.rvars));
        node_const_array__finalize(&(node->data.error.capts));
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
    free(node);
}

static void make_rulehash(context_t *ctx) {
    size_t i, j;
    ctx->rulehash.d = populate_bits(ctx->rules.n * 4);
    ctx->rulehash.m = ctx->rulehash.d + 1;
    ctx->rulehash.p = (const node_t **)realloc_e((node_t **)ctx->rulehash.p, sizeof(const node_t *) * ctx->rulehash.m);
    for (i = 0; i < ctx->rulehash.m; i++) {
        ctx->rulehash.p[i] = NULL;
    }
    for (i = 0; i < ctx->rules.n; i++) {
        node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
        assert(ctx->rules.p[i]->type == NODE_RULE);
        j = hash_string(rule->name) & ctx->rulehash.d;
        while (ctx->rulehash.p[j] != NULL) {
            if (strcmp(rule->name, ctx->rulehash.p[j]->data.rule.name) == 0) {
                assert(rule->ref == 0);
                assert(ctx->rulehash.p[j]->data.rule.ref <= 0); /* always 0 or -1 */
                rule->ref = -1; /* marks as duplicate */
                ((node_t *)ctx->rulehash.p[j])->data.rule.ref = -1; /* marks as duplicate */
                goto EXCEPTION;
            }
            j = (j + 1) & ctx->rulehash.d;
        }
        ctx->rulehash.p[j] = ctx->rules.p[i];

    EXCEPTION:;
    }
}

static const node_t *lookup_rulehash(const context_t *ctx, const char *name) {
    size_t j = hash_string(name) & ctx->rulehash.d;
    while (ctx->rulehash.p[j] != NULL && strcmp(name, ctx->rulehash.p[j]->data.rule.name) != 0) {
        j = (j + 1) & ctx->rulehash.d;
    }
    return (ctx->rulehash.p[j] != NULL) ? ctx->rulehash.p[j] : NULL;
}

static void link_references(context_t *ctx, node_t *node) {
    if (node == NULL) return;
    switch (node->type) {
    case NODE_RULE:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    case NODE_REFERENCE:
        node->data.reference.rule = lookup_rulehash(ctx, node->data.reference.name);
        if (node->data.reference.rule == NULL) {
            print_error(
                "%s:" FMT_LU ":" FMT_LU ": No definition of rule: '%s'\n",
                node->data.reference.fpos.path,
                (ulong_t)(node->data.reference.fpos.line + 1), (ulong_t)(node->data.reference.fpos.col + 1),
                node->data.reference.name
            );
            ctx->errnum++;
        }
        else if (node->data.reference.rule->data.rule.ref >= 0) { /* the target rule is not defined multiple times */
            assert(node->data.reference.rule->type == NODE_RULE);
            ((node_t *)node->data.reference.rule)->data.rule.ref++;
        }
        break;
    case NODE_STRING:
        break;
    case NODE_CHARCLASS:
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        link_references(ctx, node->data.quantity.expr);
        break;
    case NODE_PREDICATE:
        link_references(ctx, node->data.predicate.expr);
        break;
    case NODE_PROGPRED:
        break;
    case NODE_SEQUENCE:
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                link_references(ctx, node->data.sequence.nodes.p[i]);
            }
        }
        break;
    case NODE_ALTERNATE:
        {
            size_t i;
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                link_references(ctx, node->data.alternate.nodes.p[i]);
            }
        }
        break;
    case NODE_CAPTURE:
        link_references(ctx, node->data.capture.expr);
        break;
    case NODE_EXPAND:
        break;
    case NODE_ACTION:
        break;
    case NODE_ERROR:
        link_references(ctx, node->data.error.expr);
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
}

static void mark_rules_if_used(context_t *ctx, node_t *node) {
    if (node == NULL) return;
    switch (node->type) {
    case NODE_RULE:
        if (!node->data.rule.used) {
            node->data.rule.used = TRUE;
            mark_rules_if_used(ctx, node->data.rule.expr);
        }
        break;
    case NODE_REFERENCE:
        mark_rules_if_used(ctx, (node_t *)node->data.reference.rule);
        break;
    case NODE_STRING:
        break;
    case NODE_CHARCLASS:
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        mark_rules_if_used(ctx, node->data.quantity.expr);
        break;
    case NODE_PREDICATE:
        mark_rules_if_used(ctx, node->data.predicate.expr);
        break;
    case NODE_PROGPRED:
        break;
    case NODE_SEQUENCE:
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                mark_rules_if_used(ctx, node->data.sequence.nodes.p[i]);
            }
        }
        break;
    case NODE_ALTERNATE:
        {
            size_t i;
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                mark_rules_if_used(ctx, node->data.alternate.nodes.p[i]);
            }
        }
        break;
    case NODE_CAPTURE:
        mark_rules_if_used(ctx, node->data.capture.expr);
        break;
    case NODE_EXPAND:
        break;
    case NODE_ACTION:
        break;
    case NODE_ERROR:
        mark_rules_if_used(ctx, node->data.error.expr);
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
}

static void unreference_rules_from_unused_rule(context_t *ctx, node_t *node) {
    if (node == NULL) return;
    switch (node->type) {
    case NODE_RULE:
        unreference_rules_from_unused_rule(ctx, node->data.rule.expr);
        break;
    case NODE_REFERENCE:
        if (node->data.reference.rule && node->data.reference.rule->data.rule.ref > 0)
            ((node_t *)node->data.reference.rule)->data.rule.ref--;
        break;
    case NODE_STRING:
        break;
    case NODE_CHARCLASS:
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        unreference_rules_from_unused_rule(ctx, node->data.quantity.expr);
        break;
    case NODE_PREDICATE:
        unreference_rules_from_unused_rule(ctx, node->data.predicate.expr);
        break;
    case NODE_PROGPRED:
        break;
    case NODE_SEQUENCE:
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                unreference_rules_from_unused_rule(ctx, node->data.sequence.nodes.p[i]);
            }
        }
        break;
    case NODE_ALTERNATE:
        {
            size_t i;
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                unreference_rules_from_unused_rule(ctx, node->data.alternate.nodes.p[i]);
            }
        }
        break;
    case NODE_CAPTURE:
        unreference_rules_from_unused_rule(ctx, node->data.capture.expr);
        break;
    case NODE_EXPAND:
        break;
    case NODE_ACTION:
        break;
    case NODE_ERROR:
        unreference_rules_from_unused_rule(ctx, node->data.error.expr);
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
}

static void verify_rule_variables(context_t *ctx, node_t *node, node_const_array_t *rvars) {
    node_const_array_t a;
    const bool_t b = (rvars == NULL) ? TRUE : FALSE;
    if (node == NULL) return;
    if (b) {
        node_const_array__initialize(&a);
        rvars = &a;
    }
    switch (node->type) {
    case NODE_RULE:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    case NODE_REFERENCE:
        if (node->data.reference.index != VOID_VALUE) {
            size_t i;
            for (i = 0; i < rvars->n; i++) {
                assert(rvars->p[i]->type == NODE_REFERENCE);
                if (node->data.reference.index == rvars->p[i]->data.reference.index) break;
            }
            if (i == rvars->n) node_const_array__add(rvars, node);
        }
        break;
    case NODE_STRING:
        break;
    case NODE_CHARCLASS:
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        verify_rule_variables(ctx, node->data.quantity.expr, rvars);
        break;
    case NODE_PREDICATE:
        verify_rule_variables(ctx, node->data.predicate.expr, rvars);
        break;
    case NODE_PROGPRED:
        break;
    case NODE_SEQUENCE:
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                verify_rule_variables(ctx, node->data.sequence.nodes.p[i], rvars);
            }
        }
        break;
    case NODE_ALTERNATE:
        {
            size_t i, j, k, m = rvars->n;
            node_const_array_t v;
            node_const_array__initialize(&v);
            node_const_array__copy(&v, rvars);
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                v.n = m;
                verify_rule_variables(ctx, node->data.alternate.nodes.p[i], &v);
                for (j = m; j < v.n; j++) {
                    for (k = m; k < rvars->n; k++) {
                        if (v.p[j]->data.reference.index == rvars->p[k]->data.reference.index) break;
                    }
                    if (k == rvars->n) node_const_array__add(rvars, v.p[j]);
                }
            }
            node_const_array__finalize(&v);
        }
        break;
    case NODE_CAPTURE:
        verify_rule_variables(ctx, node->data.capture.expr, rvars);
        break;
    case NODE_EXPAND:
        break;
    case NODE_ACTION:
        node_const_array__copy(&(node->data.action.rvars), rvars);
        break;
    case NODE_ERROR:
        node_const_array__copy(&(node->data.error.rvars), rvars);
        verify_rule_variables(ctx, node->data.error.expr, rvars);
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
    if (b) {
        node_const_array__finalize(&a);
    }
}

static void verify_captures(context_t *ctx, node_t *node, node_const_array_t *capts) {
    node_const_array_t a;
    const bool_t b = (capts == NULL) ? TRUE : FALSE;
    if (node == NULL) return;
    if (b) {
        node_const_array__initialize(&a);
        capts = &a;
    }
    switch (node->type) {
    case NODE_RULE:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    case NODE_REFERENCE:
        break;
    case NODE_STRING:
        break;
    case NODE_CHARCLASS:
        break;
    case NODE_POSITION:
        break;
    case NODE_QUANTITY:
        verify_captures(ctx, node->data.quantity.expr, capts);
        break;
    case NODE_PREDICATE:
        verify_captures(ctx, node->data.predicate.expr, capts);
        break;
    case NODE_PROGPRED:
        node_const_array__copy(&(node->data.progpred.capts), capts);
        break;
    case NODE_SEQUENCE:
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                verify_captures(ctx, node->data.sequence.nodes.p[i], capts);
            }
        }
        break;
    case NODE_ALTERNATE:
        {
            size_t i, j, m = capts->n;
            node_const_array_t v;
            node_const_array__initialize(&v);
            node_const_array__copy(&v, capts);
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                v.n = m;
                verify_captures(ctx, node->data.alternate.nodes.p[i], &v);
                for (j = m; j < v.n; j++) {
                    node_const_array__add(capts, v.p[j]);
                }
            }
            node_const_array__finalize(&v);
        }
        break;
    case NODE_CAPTURE:
        verify_captures(ctx, node->data.capture.expr, capts);
        node_const_array__add(capts, node);
        break;
    case NODE_EXPAND:
        {
            size_t i;
            for (i = 0; i < capts->n; i++) {
                assert(capts->p[i]->type == NODE_CAPTURE);
                if (node->data.expand.index == capts->p[i]->data.capture.index) break;
            }
            if (i >= capts->n && node->data.expand.index != VOID_VALUE) {
                print_error(
                    "%s:" FMT_LU ":" FMT_LU ": Capture " FMT_LU " not available at this position\n",
                    node->data.expand.fpos.path,
                    (ulong_t)(node->data.expand.fpos.line + 1), (ulong_t)(node->data.expand.fpos.col + 1),
                    (ulong_t)(node->data.expand.index + 1)
                );
                ctx->errnum++;
            }
        }
        break;
    case NODE_ACTION:
        node_const_array__copy(&(node->data.action.capts), capts);
        break;
    case NODE_ERROR:
        node_const_array__copy(&(node->data.error.capts), capts);
        verify_captures(ctx, node->data.error.expr, capts);
        break;
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
    if (b) {
        node_const_array__finalize(&a);
    }
}

static void dump_escaped_string(const char *str) {
    char s[5];
    if (str == NULL) {
        fprintf(stdout, "null");
        return;
    }
    while (*str) {
        fprintf(stdout, "%s", escape_character(*str++, &s));
    }
}

static void dump_integer_value(size_t value) {
    if (value == VOID_VALUE) {
        fprintf(stdout, "void");
    }
    else {
        fprintf(stdout, FMT_LU, (ulong_t)value);
    }
}

static void dump_node(context_t *ctx, const node_t *node, const int indent) {
    if (node == NULL) return;
    switch (node->type) {
    case NODE_RULE:
        fprintf(
            stdout, "%*sRule(name:'%s', ref:%d, vars.len:" FMT_LU ", capts.len:" FMT_LU ", codes.len:" FMT_LU ", preds.len:" FMT_LU ") {\n",
            indent, "", node->data.rule.name, node->data.rule.ref,
            (ulong_t)node->data.rule.rvars.n, (ulong_t)node->data.rule.capts.n,
            (ulong_t)node->data.rule.codes.n, (ulong_t)node->data.rule.preds.n
        );
        dump_node(ctx, node->data.rule.expr, indent + 2);
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_REFERENCE:
        fprintf(stdout, "%*sReference(var:'%s', index:", indent, "", node->data.reference.rvar);
        dump_integer_value(node->data.reference.index);
        fprintf(
            stdout, ", name:'%s', rule:'%s')\n", node->data.reference.name,
            (node->data.reference.rule) ? node->data.reference.rule->data.rule.name : NULL
        );
        break;
    case NODE_STRING:
        fprintf(stdout, "%*sString(value:'", indent, "");
        dump_escaped_string(node->data.string.value);
        fprintf(stdout, "')\n");
        break;
    case NODE_CHARCLASS:
        fprintf(stdout, "%*sCharclass(value:'", indent, "");
        dump_escaped_string(node->data.charclass.value);
        fprintf(stdout, "')\n");
        break;
    case NODE_POSITION:
        fprintf(stdout, "%*sPosition(value:" FMT_LU ")\n", indent, "", (ulong_t)node->data.position.value);
        break;
    case NODE_QUANTITY:
        fprintf(stdout, "%*sQuantity(min:%d, max:%d) {\n", indent, "", node->data.quantity.min, node->data.quantity.max);
        dump_node(ctx, node->data.quantity.expr, indent + 2);
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_PREDICATE:
        fprintf(stdout, "%*sPredicate(neg:%d) {\n", indent, "", node->data.predicate.neg);
        dump_node(ctx, node->data.predicate.expr, indent + 2);
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_PROGPRED:
        fprintf(stdout, "%*sProgrammable Predicate(index:", indent, "");
        dump_integer_value(node->data.progpred.index);
        fprintf(stdout, ", neg:%d, code:{", node->data.progpred.neg);
        dump_escaped_string(node->data.progpred.code.text);
        fprintf(stdout, "}, capts:");
        if (node->data.progpred.capts.n > 0) {
            size_t i;
            fprintf(stdout, "\n");
            for (i = 0; i < node->data.progpred.capts.n; i++) {
                fprintf(stdout, "%*s$" FMT_LU "\n", indent + 2, "", (ulong_t)(node->data.progpred.capts.p[i]->data.capture.index + 1));
            }
            fprintf(stdout, "%*s)\n", indent, "");
        }
        else {
            fprintf(stdout, "none)\n");
        }
        break;
    case NODE_SEQUENCE:
        fprintf(
            stdout, "%*sSequence(max:" FMT_LU ", len:" FMT_LU ") {\n",
            indent, "", (ulong_t)node->data.sequence.nodes.m, (ulong_t)node->data.sequence.nodes.n
        );
        {
            size_t i;
            for (i = 0; i < node->data.sequence.nodes.n; i++) {
                dump_node(ctx, node->data.sequence.nodes.p[i], indent + 2);
            }
        }
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_ALTERNATE:
        fprintf(
            stdout, "%*sAlternate(max:" FMT_LU ", len:" FMT_LU ") {\n",
            indent, "", (ulong_t)node->data.alternate.nodes.m, (ulong_t)node->data.alternate.nodes.n
        );
        {
            size_t i;
            for (i = 0; i < node->data.alternate.nodes.n; i++) {
                dump_node(ctx, node->data.alternate.nodes.p[i], indent + 2);
            }
        }
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_CAPTURE:
        fprintf(stdout, "%*sCapture(index:", indent, "");
        dump_integer_value(node->data.capture.index);
        fprintf(stdout, ") {\n");
        dump_node(ctx, node->data.capture.expr, indent + 2);
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    case NODE_EXPAND:
        fprintf(stdout, "%*sExpand(index:", indent, "");
        dump_integer_value(node->data.expand.index);
        fprintf(stdout, ")\n");
        break;
    case NODE_ACTION:
        fprintf(stdout, "%*sAction(index:", indent, "");
        dump_integer_value(node->data.action.index);
        fprintf(stdout, ", code:{");
        dump_escaped_string(node->data.action.code.text);
        fprintf(stdout, "}, vars:");
        if (node->data.action.rvars.n + node->data.action.capts.n > 0) {
            size_t i;
            fprintf(stdout, "\n");
            for (i = 0; i < node->data.action.rvars.n; i++) {
                fprintf(stdout, "%*s'%s'\n", indent + 2, "", node->data.action.rvars.p[i]->data.reference.rvar);
            }
            for (i = 0; i < node->data.action.capts.n; i++) {
                fprintf(stdout, "%*s$" FMT_LU "\n", indent + 2, "", (ulong_t)(node->data.action.capts.p[i]->data.capture.index + 1));
            }
            fprintf(stdout, "%*s)\n", indent, "");
        }
        else {
            fprintf(stdout, "none)\n");
        }
        break;
    case NODE_ERROR:
        fprintf(stdout, "%*sError(index:", indent, "");
        dump_integer_value(node->data.error.index);
        fprintf(stdout, ", code:{");
        dump_escaped_string(node->data.error.code.text);
        fprintf(stdout, "}, vars:\n");
        {
            size_t i;
            for (i = 0; i < node->data.error.rvars.n; i++) {
                fprintf(stdout, "%*s'%s'\n", indent + 2, "", node->data.error.rvars.p[i]->data.reference.rvar);
            }
            for (i = 0; i < node->data.error.capts.n; i++) {
                fprintf(stdout, "%*s$" FMT_LU "\n", indent + 2, "", (ulong_t)(node->data.error.capts.p[i]->data.capture.index + 1));
            }
        }
        fprintf(stdout, "%*s) {\n", indent, "");
        dump_node(ctx, node->data.error.expr, indent + 2);
        fprintf(stdout, "%*s}\n", indent, "");
        break;
    default:
        print_error("%*sInternal error [%d]\n", indent, "", __LINE__);
        exit(-1);
    }
}

static size_t refill_buffer(input_state_t *input, size_t num) {
    if (input->buffer.n >= input->bufcur + num) return input->buffer.n - input->bufcur;
    while (input->buffer.n < input->bufcur + num) {
        const int c = fgetc_e(input->file, input->path);
        if (c == EOF) break;
        char_array__add(&(input->buffer), (char)c);
    }
    return input->buffer.n - input->bufcur;
}

static void commit_buffer(input_state_t *input) {
    assert(input->buffer.n >= input->bufcur);
    if (input->linepos < input->bufpos + input->bufcur)
        input->charnum += input->ascii ? input->bufcur : count_characters(input->buffer.p, 0, input->bufcur);
    memmove(input->buffer.p, input->buffer.p + input->bufcur, input->buffer.n - input->bufcur);
    input->buffer.n -= input->bufcur;
    input->bufpos += input->bufcur;
    input->bufcur = 0;
}

static bool_t match_eof(input_state_t *input) {
    return (refill_buffer(input, 1) < 1) ? TRUE : FALSE;
}

static bool_t match_eol(input_state_t *input) {
    if (refill_buffer(input, 1) >= 1) {
        switch (input->buffer.p[input->bufcur]) {
        case '\n':
            input->bufcur++;
            input->linenum++;
            input->charnum = 0;
            input->linepos = input->bufpos + input->bufcur;
            return TRUE;
        case '\r':
            input->bufcur++;
            if (refill_buffer(input, 1) >= 1) {
                if (input->buffer.p[input->bufcur] == '\n') input->bufcur++;
            }
            input->linenum++;
            input->charnum = 0;
            input->linepos = input->bufpos + input->bufcur;
            return TRUE;
        }
    }
    return FALSE;
}

static bool_t match_character(input_state_t *input, char ch) {
    if (refill_buffer(input, 1) >= 1) {
        if (input->buffer.p[input->bufcur] == ch) {
            input->bufcur++;
            return TRUE;
        }
    }
    return FALSE;
}

static bool_t match_character_range(input_state_t *input, char min, char max) {
    if (refill_buffer(input, 1) >= 1) {
        const char c = input->buffer.p[input->bufcur];
        if (c >= min && c <= max) {
            input->bufcur++;
            return TRUE;
        }
    }
    return FALSE;
}

static bool_t match_character_set(input_state_t *input, const char *chs) {
    if (refill_buffer(input, 1) >= 1) {
        const char c = input->buffer.p[input->bufcur];
        size_t i;
        for (i = 0; chs[i]; i++) {
            if (c == chs[i]) {
                input->bufcur++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static bool_t match_character_any(input_state_t *input) {
    if (refill_buffer(input, 1) >= 1) {
        input->bufcur++;
        return TRUE;
    }
    return FALSE;
}

static bool_t match_string(input_state_t *input, const char *str) {
    const size_t n = strlen(str);
    if (refill_buffer(input, n) >= n) {
        if (strncmp(input->buffer.p + input->bufcur, str, n) == 0) {
            input->bufcur += n;
            return TRUE;
        }
    }
    return FALSE;
}

static bool_t match_blank(input_state_t *input) {
    return match_character_set(input, " \t\v\f");
}

static bool_t match_section_line_(input_state_t *input, const char *head) {
    if (match_string(input, head)) {
        while (!match_eol(input) && !match_eof(input)) match_character_any(input);
        return TRUE;
    }
    return FALSE;
}

static bool_t match_section_line_continuable_(input_state_t *input, const char *head) {
    if (match_string(input, head)) {
        while (!match_eof(input)) {
            const size_t p = input->bufcur;
            if (match_eol(input)) {
                if (input->buffer.p[p - 1] != '\\') break;
            }
            else {
                match_character_any(input);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static bool_t match_section_block_(input_state_t *input, const char *left, const char *right, const char *name) {
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    if (match_string(input, left)) {
        while (!match_string(input, right)) {
            if (match_eof(input)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Premature EOF in %s\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
                input->errnum++;
                break;
            }
            if (!match_eol(input)) match_character_any(input);
        }
        return TRUE;
    }
    return FALSE;
}

static bool_t match_quotation_(input_state_t *input, const char *left, const char *right, const char *name) {
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    if (match_string(input, left)) {
        while (!match_string(input, right)) {
            if (match_eof(input)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Premature EOF in %s\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
                input->errnum++;
                break;
            }
            match_character(input, '\\');
            if (match_eol(input)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Premature EOL in %s\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
                input->errnum++;
                break;
            }
            match_character_any(input);
        }
        return TRUE;
    }
    return FALSE;
}

static bool_t match_comment(input_state_t *input) {
    return match_section_line_(input, "#");
}

static bool_t match_comment_c(input_state_t *input) {
    return match_section_block_(input, "/*", "*/", "C comment");
}

static bool_t match_comment_cxx(input_state_t *input) {
    return match_section_line_continuable_(input, "//");
}

static bool_t match_quotation_single(input_state_t *input) {
    return match_quotation_(input, "\'", "\'", "single quotation");
}

static bool_t match_quotation_double(input_state_t *input) {
    return match_quotation_(input, "\"", "\"", "double quotation");
}

static bool_t match_character_class(input_state_t *input) {
    return match_quotation_(input, "[", "]", "character class");
}

static bool_t match_directive_c(input_state_t *input) {
    if (match_character(input, '#')) {
        while (!match_eof(input)) {
            const size_t p = input->bufcur;
            if (match_eol(input)) {
                if (input->buffer.p[p - 1] != '\\') break;
            }
            else if (match_comment_cxx(input)) {
                break;
            }
            else if (
                !match_comment_c(input) &&
                !match_quotation_single(input) &&
                !match_quotation_double(input) &&
                !match_character_any(input)
            ) {
                break;
            }
        }
        return TRUE;
    }
    return FALSE;
}

static bool_t match_spaces(input_state_t *input) {
    size_t n = 0;
    while (match_blank(input) || match_eol(input) || match_comment(input)) n++;
    return (n > 0) ? TRUE : FALSE;
}

static bool_t match_number(input_state_t *input) {
    if (match_character_range(input, '0', '9')) {
        while (match_character_range(input, '0', '9'));
        return TRUE;
    }
    return FALSE;
}

static bool_t match_identifier(input_state_t *input) {
    if (
        match_character_range(input, 'a', 'z') ||
        match_character_range(input, 'A', 'Z') ||
        match_character(input, '_')
    ) {
        while (
            match_character_range(input, 'a', 'z') ||
            match_character_range(input, 'A', 'Z') ||
            match_character_range(input, '0', '9') ||
            match_character(input, '_')
        );
        return TRUE;
    }
    return FALSE;
}

static bool_t match_code_block(input_state_t *input) {
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    if (match_character(input, '{')) {
        int d = 1;
        for (;;) {
            if (match_eof(input)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Premature EOF in code block\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                input->errnum++;
                break;
            }
            if (
                match_directive_c(input) ||
                match_comment_c(input) ||
                match_comment_cxx(input) ||
                match_quotation_single(input) ||
                match_quotation_double(input)
            ) continue;
            if (match_character(input, '{')) {
                d++;
            }
            else if (match_character(input, '}')) {
                d--;
                if (d == 0) break;
            }
            else if (!match_eol(input)) {
                match_character_any(input);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static bool_t match_footer_start(input_state_t *input) {
    return match_string(input, "%%");
}

static node_t *parse_expression(input_state_t *input, node_t *rule);

static node_t *parse_primary(input_state_t *input, node_t *rule) {
    const size_t p = input->bufcur;
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    const size_t n = input->charnum;
    const size_t o = input->linepos;
    node_t *n_p = NULL;
    if (match_identifier(input)) {
        const size_t q = input->bufcur;
        size_t r = VOID_VALUE, s = VOID_VALUE;
        match_spaces(input);
        if (match_character(input, ':')) {
            match_spaces(input);
            r = input->bufcur;
            if (!match_identifier(input)) goto EXCEPTION;
            s = input->bufcur;
            match_spaces(input);
        }
        if (match_string(input, "<-")) goto EXCEPTION;
        n_p = create_node(NODE_REFERENCE);
        if (r == VOID_VALUE) {
            assert(q >= p);
            n_p->data.reference.rvar = NULL;
            n_p->data.reference.index = VOID_VALUE;
            n_p->data.reference.name = strndup_e(input->buffer.p + p, q - p);
        }
        else {
            assert(s != VOID_VALUE); /* s should have a valid value when r has a valid value */
            assert(q >= p);
            n_p->data.reference.rvar = strndup_e(input->buffer.p + p, q - p);
            if (n_p->data.reference.rvar[0] == '_') {
                print_error(
                    "%s:" FMT_LU ":" FMT_LU ": Leading underscore in rule variable identifier '%s'\n",
                    input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), n_p->data.reference.rvar
                );
                input->errnum++;
            }
            {
                size_t i;
                for (i = 0; i < rule->data.rule.rvars.n; i++) {
                    assert(rule->data.rule.rvars.p[i]->type == NODE_REFERENCE);
                    if (strcmp(n_p->data.reference.rvar, rule->data.rule.rvars.p[i]->data.reference.rvar) == 0) break;
                }
                if (i == rule->data.rule.rvars.n) node_const_array__add(&(rule->data.rule.rvars), n_p);
                n_p->data.reference.index = i;
            }
            assert(s >= r);
            n_p->data.reference.name = strndup_e(input->buffer.p + r, s - r);
        }
        file_pos__set(&(n_p->data.reference.fpos), input->path, l, m);
    }
    else if (match_character(input, '(')) {
        match_spaces(input);
        n_p = parse_expression(input, rule);
        if (n_p == NULL) goto EXCEPTION;
        if (!match_character(input, ')')) goto EXCEPTION;
        match_spaces(input);
    }
    else if (match_character(input, '<')) {
        match_spaces(input);
        n_p = create_node(NODE_CAPTURE);
        n_p->data.capture.index = rule->data.rule.capts.n;
        node_const_array__add(&(rule->data.rule.capts), n_p);
        n_p->data.capture.expr = parse_expression(input, rule);
        if (n_p->data.capture.expr == NULL || !match_character(input, '>')) {
            rule->data.rule.capts.n = n_p->data.capture.index;
            goto EXCEPTION;
        }
        match_spaces(input);
    }
    else if (match_character(input, '$')) {
        size_t p;
        match_spaces(input);
        p = input->bufcur;
        if (match_number(input)) {
            const size_t q = input->bufcur;
            char *s;
            match_spaces(input);
            n_p = create_node(NODE_EXPAND);
            assert(q >= p);
            s = strndup_e(input->buffer.p + p, q - p);
            n_p->data.expand.index = string_to_size_t(s);
            if (n_p->data.expand.index == VOID_VALUE) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Invalid unsigned number '%s'\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), s);
                input->errnum++;
            }
            else if (n_p->data.expand.index == 0) {
                print_error("%s:" FMT_LU ":" FMT_LU ": 0 not allowed\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                input->errnum++;
            }
            else if (s[0] == '0') {
                print_error("%s:" FMT_LU ":" FMT_LU ": 0-prefixed number not allowed\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                input->errnum++;
                n_p->data.expand.index = 0;
            }
            free(s);
            if (n_p->data.expand.index > 0 && n_p->data.expand.index != VOID_VALUE) {
                n_p->data.expand.index--;
                file_pos__set(&(n_p->data.expand.fpos), input->path, l, m);
            }
        }
        else {
            goto EXCEPTION;
        }
    }
    else if (match_character(input, '^')) {
        match_spaces(input);
        n_p = create_node(NODE_POSITION);
        n_p->data.position.value = 0;
    }
    else if (match_character(input, '.')) {
        match_spaces(input);
        n_p = create_node(NODE_CHARCLASS);
        n_p->data.charclass.value = NULL;
        if (!input->ascii) {
            input->flags |= CODE_FLAG__UTF8_CHARCLASS_USED;
        }
    }
    else if (match_character_class(input)) {
        const size_t q = input->bufcur;
        match_spaces(input);
        n_p = create_node(NODE_CHARCLASS);
        n_p->data.charclass.value = strndup_e(input->buffer.p + p + 1, q - p - 2);
        if (!unescape_string(n_p->data.charclass.value, TRUE)) {
            print_error("%s:" FMT_LU ":" FMT_LU ": Illegal escape sequence\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
            input->errnum++;
        }
        if (!input->ascii && !is_valid_utf8_string(n_p->data.charclass.value)) {
            print_error("%s:" FMT_LU ":" FMT_LU ": Invalid UTF-8 string\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
            input->errnum++;
        }
        if (!input->ascii && n_p->data.charclass.value[0] != '\0') {
            input->flags |= CODE_FLAG__UTF8_CHARCLASS_USED;
        }
    }
    else if (match_quotation_single(input) || match_quotation_double(input)) {
        const size_t q = input->bufcur;
        match_spaces(input);
        n_p = create_node(NODE_STRING);
        n_p->data.string.value = strndup_e(input->buffer.p + p + 1, q - p - 2);
        if (!unescape_string(n_p->data.string.value, FALSE)) {
            print_error("%s:" FMT_LU ":" FMT_LU ": Illegal escape sequence\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
            input->errnum++;
        }
        if (!input->ascii && !is_valid_utf8_string(n_p->data.string.value)) {
            print_error("%s:" FMT_LU ":" FMT_LU ": Invalid UTF-8 string\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
            input->errnum++;
        }
    }
    else if (match_code_block(input)) {
        const size_t q = input->bufcur;
        match_spaces(input);
        n_p = create_node(NODE_ACTION);
        n_p->data.action.code.text = strndup_e(input->buffer.p + p + 1, q - p - 2);
        n_p->data.action.code.len = find_trailing_blanks(n_p->data.action.code.text);
        file_pos__set(&(n_p->data.action.code.fpos), input->path, l, m);
        n_p->data.action.index = rule->data.rule.codes.n;
        node_const_array__add(&(rule->data.rule.codes), n_p);
    }
    else {
        goto EXCEPTION;
    }
    return n_p;

EXCEPTION:;
    destroy_node(n_p);
    input->bufcur = p;
    input->linenum = l;
    input->charnum = n;
    input->linepos = o;
    return NULL;
}

static node_t *parse_term(input_state_t *input, node_t *rule) {
    const size_t p = input->bufcur;
    const size_t l = input->linenum;
    const size_t n = input->charnum;
    const size_t o = input->linepos;
    node_t *n_p = NULL;
    node_t *n_q = NULL;
    node_t *n_r = NULL;
    node_t *n_t = NULL;
    const char t = match_character(input, '&') ? '&' : match_character(input, '!') ? '!' : '\0';
    if (t) {
        size_t p, l, m;
        match_spaces(input);
        p = input->bufcur;
        l = input->linenum;
        m = column_number(input);
        if (match_code_block(input)) {
            const size_t q = input->bufcur;
            match_spaces(input);
            n_p = create_node(NODE_PROGPRED);
            n_p->data.progpred.neg = (t == '!') ? TRUE : FALSE;
            n_p->data.progpred.code.text = strndup_e(input->buffer.p + p + 1, q - p - 2);
            n_p->data.progpred.code.len = find_trailing_blanks(n_p->data.progpred.code.text);
            file_pos__set(&n_p->data.progpred.code.fpos, input->path, l, m);
            n_p->data.progpred.index = rule->data.rule.preds.n;
            node_const_array__add(&(rule->data.rule.preds), n_p);
        }
    }
    if (n_p == NULL) {
        n_p = parse_primary(input, rule);
        if (n_p == NULL) goto EXCEPTION;
        if (match_character(input, '*')) {
            match_spaces(input);
            n_q = create_node(NODE_QUANTITY);
            n_q->data.quantity.min = 0;
            n_q->data.quantity.max = -1;
            n_q->data.quantity.expr = n_p;
        }
        else if (match_character(input, '+')) {
            match_spaces(input);
            n_q = create_node(NODE_QUANTITY);
            n_q->data.quantity.min = 1;
            n_q->data.quantity.max = -1;
            n_q->data.quantity.expr = n_p;
        }
        else if (match_character(input, '?')) {
            match_spaces(input);
            n_q = create_node(NODE_QUANTITY);
            n_q->data.quantity.min = 0;
            n_q->data.quantity.max = 1;
            n_q->data.quantity.expr = n_p;
        }
        else {
            n_q = n_p;
        }
        switch (t) {
        case '&':
            n_r = create_node(NODE_PREDICATE);
            n_r->data.predicate.neg = FALSE;
            n_r->data.predicate.expr = n_q;
            break;
        case '!':
            n_r = create_node(NODE_PREDICATE);
            n_r->data.predicate.neg = TRUE;
            n_r->data.predicate.expr = n_q;
            break;
        default:
            n_r = n_q;
        }
    }
    else {
        n_r = n_q = n_p;
    }
    if (match_character(input, '~')) {
        size_t p, l, m;
        match_spaces(input);
        p = input->bufcur;
        l = input->linenum;
        m = column_number(input);
        if (match_code_block(input)) {
            const size_t q = input->bufcur;
            match_spaces(input);
            n_t = create_node(NODE_ERROR);
            n_t->data.error.expr = n_r;
            n_t->data.error.code.text = strndup_e(input->buffer.p + p + 1, q - p - 2);
            n_t->data.error.code.len = find_trailing_blanks(n_t->data.error.code.text);
            file_pos__set(&(n_t->data.error.code.fpos), input->path, l, m);
            n_t->data.error.index = rule->data.rule.codes.n;
            node_const_array__add(&(rule->data.rule.codes), n_t);
        }
        else {
            goto EXCEPTION;
        }
    }
    else {
        n_t = n_r;
    }
    return n_t;

EXCEPTION:;
    destroy_node(n_r);
    input->bufcur = p;
    input->linenum = l;
    input->charnum = n;
    input->linepos = o;
    return NULL;
}

static node_t *parse_sequence(input_state_t *input, node_t *rule) {
    const size_t p = input->bufcur;
    const size_t l = input->linenum;
    const size_t n = input->charnum;
    const size_t o = input->linepos;
    node_array_t *a_t = NULL;
    node_t *n_t = NULL;
    node_t *n_u = NULL;
    node_t *n_s = NULL;
    n_t = parse_term(input, rule);
    if (n_t == NULL) goto EXCEPTION;
    n_u = parse_term(input, rule);
    if (n_u != NULL) {
        n_s = create_node(NODE_SEQUENCE);
        a_t = &(n_s->data.sequence.nodes);
        node_array__add(a_t, n_t);
        node_array__add(a_t, n_u);
        while ((n_t = parse_term(input, rule)) != NULL) {
            node_array__add(a_t, n_t);
        }
    }
    else {
        n_s = n_t;
    }
    return n_s;

EXCEPTION:;
    input->bufcur = p;
    input->linenum = l;
    input->charnum = n;
    input->linepos = o;
    return NULL;
}

static node_t *parse_expression(input_state_t *input, node_t *rule) {
    const size_t p = input->bufcur;
    const size_t l = input->linenum;
    const size_t n = input->charnum;
    const size_t o = input->linepos;
    size_t q;
    node_array_t *a_s = NULL;
    node_t *n_s = NULL;
    node_t *n_e = NULL;
    n_s = parse_sequence(input, rule);
    if (n_s == NULL) goto EXCEPTION;
    q = input->bufcur;
    if (match_character(input, '/')) {
        input->bufcur = q;
        n_e = create_node(NODE_ALTERNATE);
        a_s = &(n_e->data.alternate.nodes);
        node_array__add(a_s, n_s);
        while (match_character(input, '/')) {
            match_spaces(input);
            n_s = parse_sequence(input, rule);
            if (n_s == NULL) goto EXCEPTION;
            node_array__add(a_s, n_s);
        }
    }
    else {
        n_e = n_s;
    }
    return n_e;

EXCEPTION:;
    destroy_node(n_e);
    input->bufcur = p;
    input->linenum = l;
    input->charnum = n;
    input->linepos = o;
    return NULL;
}

static node_t *parse_rule(input_state_t *input) {
    const size_t p = input->bufcur;
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    const size_t n = input->charnum;
    const size_t o = input->linepos;
    size_t q;
    node_t *n_r = NULL;
    if (!match_identifier(input)) goto EXCEPTION;
    q = input->bufcur;
    match_spaces(input);
    if (!match_string(input, "<-")) goto EXCEPTION;
    match_spaces(input);
    n_r = create_node(NODE_RULE);
    n_r->data.rule.expr = parse_expression(input, n_r);
    if (n_r->data.rule.expr == NULL) goto EXCEPTION;
    assert(q >= p);
    n_r->data.rule.name = strndup_e(input->buffer.p + p, q - p);
    file_pos__set(&(n_r->data.rule.fpos), input->path, l, m);
    return n_r;

EXCEPTION:;
    destroy_node(n_r);
    input->bufcur = p;
    input->linenum = l;
    input->charnum = n;
    input->linepos = o;
    return NULL;
}

static const char *get_value_type(context_t *ctx) {
    return (ctx->vtype && ctx->vtype[0]) ? ctx->vtype : "int";
}

static const char *get_auxil_type(context_t *ctx) {
    return (ctx->atype && ctx->atype[0]) ? ctx->atype : "void *";
}

static const char *get_prefix(context_t *ctx) {
    return (ctx->prefix && ctx->prefix[0]) ? ctx->prefix : "pcc";
}

static void dump_options(context_t *ctx) {
    fprintf(stdout, "value_type: '%s'\n", get_value_type(ctx));
    fprintf(stdout, "auxil_type: '%s'\n", get_auxil_type(ctx));
    fprintf(stdout, "prefix: '%s'\n", get_prefix(ctx));
}

static bool_t parse_directive_block_(input_state_t *input, const char *name, code_block_array_t *output1, code_block_array_t *output2) {
    if (!match_string(input, name)) return FALSE;
    match_spaces(input);
    {
        const size_t p = input->bufcur;
        const size_t l = input->linenum;
        const size_t m = column_number(input);
        if (match_code_block(input)) {
            const size_t q = input->bufcur;
            match_spaces(input);
            if (output1 != NULL) {
                code_block_t *const c = code_block_array__create_entry(output1);
                c->text = strndup_e(input->buffer.p + p + 1, q - p - 2);
                c->len = q - p - 2;
                file_pos__set(&(c->fpos), input->path, l, m);
            }
            if (output2 != NULL) {
                code_block_t *const c = code_block_array__create_entry(output2);
                c->text = strndup_e(input->buffer.p + p + 1, q - p - 2);
                c->len = q - p - 2;
                file_pos__set(&(c->fpos), input->path, l, m);
            }
        }
        else {
            print_error("%s:" FMT_LU ":" FMT_LU ": Illegal %s syntax\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
            input->errnum++;
        }
    }
    return TRUE;
}

static bool_t parse_directive_string_(input_state_t *input, const char *name, char **output, string_flag_t mode) {
    const size_t l = input->linenum;
    const size_t m = column_number(input);
    if (!match_string(input, name)) return FALSE;
    match_spaces(input);
    {
        char *s = NULL;
        const size_t p = input->bufcur;
        const size_t lv = input->linenum;
        const size_t mv = column_number(input);
        size_t q;
        if (match_quotation_single(input) || match_quotation_double(input)) {
            q = input->bufcur;
            match_spaces(input);
            s = strndup_e(input->buffer.p + p + 1, q - p - 2);
            if (!unescape_string(s, FALSE)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Illegal escape sequence\n", input->path, (ulong_t)(lv + 1), (ulong_t)(mv + 1));
                input->errnum++;
            }
        }
        else {
            print_error("%s:" FMT_LU ":" FMT_LU ": Illegal %s syntax\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
            input->errnum++;
        }
        if (s != NULL) {
            string_flag_t f = STRING_FLAG__NONE;
            bool_t b = TRUE;
            remove_leading_blanks(s);
            remove_trailing_blanks(s);
            assert((mode & ~7) == 0);
            if ((mode & STRING_FLAG__NOTEMPTY) && !is_filled_string(s)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Empty string\n", input->path, (ulong_t)(lv + 1), (ulong_t)(mv + 1));
                input->errnum++;
                f |= STRING_FLAG__NOTEMPTY;
            }
            if ((mode & STRING_FLAG__NOTVOID) && strcmp(s, "void") == 0) {
                print_error("%s:" FMT_LU ":" FMT_LU ": 'void' not allowed\n", input->path, (ulong_t)(lv + 1), (ulong_t)(mv + 1));
                input->errnum++;
                f |= STRING_FLAG__NOTVOID;
            }
            if ((mode & STRING_FLAG__IDENTIFIER) && !is_identifier_string(s)) {
                if (!(f & STRING_FLAG__NOTEMPTY)) {
                    print_error("%s:" FMT_LU ":" FMT_LU ": Invalid identifier\n", input->path, (ulong_t)(lv + 1), (ulong_t)(mv + 1));
                    input->errnum++;
                }
                f |= STRING_FLAG__IDENTIFIER;
            }
            if (output == NULL) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Definition of %s not allowed\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
                input->errnum++;
                b = FALSE;
            }
            else if (*output != NULL) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Multiple definitions of %s\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), name);
                input->errnum++;
                b = FALSE;
            }
            if (f == STRING_FLAG__NONE && b) {
                assert(output != NULL);
                *output = s;
            }
            else {
                free(s); s = NULL;
            }
        }
    }
    return TRUE;
}

static bool_t parse_directive_marker_(input_state_t *input, const char *name, string_array_t *output) {
    if (!match_string(input, name)) return FALSE;
    for (;;) {
        match_spaces(input);
        if (!match_character(input, '@')) break;
        match_spaces(input);
        {
            const size_t p = input->bufcur;
            const size_t l = input->linenum;
            const size_t m = column_number(input);
            if (!match_identifier(input)) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Illegal marker variable identifier\n", input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                input->errnum++;
                break;
            }
            else {
                const size_t q = input->bufcur;
                size_t i;
                for (i = 0; i < output->n; i++) {
                    if (strncmp(output->p[i], input->buffer.p + p, q - p) == 0) break;
                }
                if (i < output->n) {
                    print_error(
                        "%s:" FMT_LU ":" FMT_LU ": Duplicate marker variable identifier '%s'\n",
                        input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), output->p[i]
                    );
                    input->errnum++;
                }
                else {
                    string_array__add(output, input->buffer.p + p, q - p);
                }
                if (output->p[i][0] == '_') {
                    print_error(
                        "%s:" FMT_LU ":" FMT_LU ": Leading underscore in marker variable identifier '%s'\n",
                        input->path, (ulong_t)(l + 1), (ulong_t)(m + 1), output->p[i]
                    );
                    input->errnum++;
                }
            }
        }
    }
    return TRUE;
}

static bool_t parse_footer_(input_state_t *input, code_block_array_t *output) {
    if (!match_footer_start(input)) return FALSE;
    {
        const size_t p = input->bufcur;
        const size_t l = input->linenum;
        const size_t m = column_number(input);
        while (!match_eof(input)) match_character_any(input);
        {
            const size_t q = input->bufcur;
            code_block_t *const c = code_block_array__create_entry(output);
            c->text = strndup_e(input->buffer.p + p, q - p);
            c->len = q - p;
            file_pos__set(&(c->fpos), input->path, l, m);
        }
    }
    return TRUE;
}

static void parse_file_(context_t *ctx) {
    if (ctx->input == NULL) return;
    {
        file_id_t id;
        file_id__get(ctx->input->file, ctx->input->path, &id);
        if (!file_id_array__add_if_not_yet(&(ctx->done), &id)) return; /* already imported */
    }
    {
        const bool_t imp = input_state__is_in_imported(ctx->input);
        bool_t b = TRUE;
        match_spaces(ctx->input);
        for (;;) {
            char *s = NULL;
            size_t l, m, n, o;
            if (match_eof(ctx->input) || parse_footer_(ctx->input, &(ctx->fsource))) break;
            l = ctx->input->linenum;
            m = column_number(ctx->input);
            n = ctx->input->charnum;
            o = ctx->input->linepos;
            if (parse_directive_string_(ctx->input, "%import", &s, STRING_FLAG__NOTEMPTY)) {
                if (s) {
                    if (is_absolute_path(s)) {
                        FILE *const file = fopen(s, "rb");
                        if (file) {
                            ctx->input = input_state__create(s, file, ctx->input, &(ctx->opts));
                            parse_file_(ctx);
                            ctx->input = input_state__destroy(ctx->input);
                        }
                        else {
                            if (errno != ENOENT) {
                                print_error(
                                    "%s:" FMT_LU ":" FMT_LU ": Cannot open file to read: %s\n",
                                    ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1),
                                    s
                                );
                            }
                            else {
                                print_error(
                                    "%s:" FMT_LU ":" FMT_LU ": File not found: %s\n",
                                    ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1),
                                    s
                                );
                            }
                            ctx->input->errnum++;
                        }
                    }
                    else {
                        size_t i = 0;
                        char *path = replace_filename(ctx->input->path, s);
                        FILE *file = fopen(path, "rb");
                        while (file == NULL) {
                            if (errno != ENOENT) {
                                print_error(
                                    "%s:" FMT_LU ":" FMT_LU ": Cannot open file to read: %s\n",
                                    ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1),
                                    path
                                );
                                ctx->input->errnum++;
                                break;
                            }
                            if (i >= ctx->dirs->n) {
                                print_error(
                                    "%s:" FMT_LU ":" FMT_LU ": File not found: %s\n",
                                    ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1),
                                    s
                                );
                                ctx->input->errnum++;
                                break;
                            }
                            free(path);
                            path = add_filename(ctx->dirs->p[i++], s);
                            file = fopen(path, "rb");
                        }
                        if (file) {
                            ctx->input = input_state__create(path, file, ctx->input, &(ctx->opts));
                            parse_file_(ctx);
                            ctx->input = input_state__destroy(ctx->input);
                        }
                        free(path);
                    }
                    free(s);
                }
                b = TRUE;
            }
            else if (
                parse_directive_block_(ctx->input, "%earlysource", &(ctx->esource), NULL) ||
                parse_directive_block_(ctx->input, "%earlyheader", &(ctx->eheader), NULL) ||
                parse_directive_block_(ctx->input, "%earlycommon", &(ctx->esource), &(ctx->eheader)) ||
                parse_directive_block_(ctx->input, "%source", &(ctx->source), NULL) ||
                parse_directive_block_(ctx->input, "%header", &(ctx->header), NULL) ||
                parse_directive_block_(ctx->input, "%common", &(ctx->source), &(ctx->header)) ||
                parse_directive_string_(ctx->input, "%value", imp ? NULL : &(ctx->vtype), STRING_FLAG__NOTEMPTY | STRING_FLAG__NOTVOID) ||
                parse_directive_string_(ctx->input, "%auxil", imp ? NULL : &(ctx->atype), STRING_FLAG__NOTEMPTY | STRING_FLAG__NOTVOID) ||
                parse_directive_string_(ctx->input, "%prefix", imp ? NULL : &(ctx->prefix), STRING_FLAG__NOTEMPTY | STRING_FLAG__IDENTIFIER) ||
                parse_directive_marker_(ctx->input, "%marker", &(ctx->mvars))
            ) {
                b = TRUE;
            }
            else if (match_character(ctx->input, '%')) {
                print_error("%s:" FMT_LU ":" FMT_LU ": Invalid directive\n", ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                ctx->input->errnum++;
                match_identifier(ctx->input);
                match_spaces(ctx->input);
                b = TRUE;
            }
            else {
                node_t *const n_r = parse_rule(ctx->input);
                if (n_r == NULL) {
                    if (b) {
                        print_error("%s:" FMT_LU ":" FMT_LU ": Illegal rule syntax\n", ctx->input->path, (ulong_t)(l + 1), (ulong_t)(m + 1));
                        ctx->input->errnum++;
                        b = FALSE;
                    }
                    ctx->input->linenum = l;
                    ctx->input->charnum = n;
                    ctx->input->linepos = o;
                    if (!match_identifier(ctx->input) && !match_spaces(ctx->input)) match_character_any(ctx->input);
                    continue;
                }
                node_array__add(&(ctx->rules), n_r);
                b = TRUE;
            }
            commit_buffer(ctx->input);
        }
        commit_buffer(ctx->input);
    }
    ctx->errnum += ctx->input->errnum;
    ctx->flags |= ctx->input->flags;
}

static bool_t parse(context_t *ctx) {
    parse_file_(ctx);
    make_rulehash(ctx);
    {
        size_t i;
        for (i = 0; i < ctx->rules.n; i++) {
            node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
            if (rule->ref < 0) {
                print_error(
                    "%s:" FMT_LU ":" FMT_LU ": Multiple definitions of rule: '%s'\n",
                    rule->fpos.path, (ulong_t)(rule->fpos.line + 1), (ulong_t)(rule->fpos.col + 1),
                    rule->name
                );
                ctx->errnum++;
                continue;
            }
            link_references(ctx, rule->expr);
        }
    }
    if (ctx->rules.n > 0)
        mark_rules_if_used(ctx, ctx->rules.p[0]);
    {
        size_t i;
        for (i = 0; i < ctx->rules.n; i++) {
            if (!ctx->rules.p[i]->data.rule.used)
                unreference_rules_from_unused_rule(ctx, ctx->rules.p[i]);
        }
    }
    {
        size_t i, j;
        for (i = 0, j = 0; i < ctx->rules.n; i++) {
            if (!ctx->rules.p[i]->data.rule.used)
                destroy_node(ctx->rules.p[i]);
            else
                ctx->rules.p[j++] = ctx->rules.p[i];
        }
        ctx->rules.n = j;
    }
    {
        size_t i;
        for (i = 0; i < ctx->rules.n; i++) {
            const node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
            verify_rule_variables(ctx, rule->expr, NULL);
            verify_captures(ctx, rule->expr, NULL);
        }
    }
    if (ctx->opts.debug) {
        size_t i;
        for (i = 0; i < ctx->rules.n; i++) {
            dump_node(ctx, ctx->rules.p[i], 0);
        }
        dump_options(ctx);
    }
    return (ctx->errnum == 0) ? TRUE : FALSE;
}

static code_reach_t generate_matching_string_code(generate_t *gen, const char *value, int onfail, size_t indent, bool_t bare) {
    const size_t n = (value != NULL) ? strlen(value) : 0;
    if (n > 0) {
        char s[5];
        if (n > 1) {
            size_t i;
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "if (\n");
            for (i = 0; i < n; i++) {
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__printf(
                    gen->stream,
                    "pcc_refill_buffer(ctx, " FMT_LU ") < " FMT_LU " || (ctx->buffer.p + ctx->cur)[" FMT_LU "] != '%s'%s\n",
                    (ulong_t)i + 1, (ulong_t)i + 1, (ulong_t)i, escape_character(value[i], &s), (i < n - 1) ? " ||" : ""
                );
            }
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, ") goto L%04d;\n", onfail);
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "ctx->cur += " FMT_LU ";\n", (ulong_t)n);
            return CODE_REACH__BOTH;
        }
        else {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "if (\n");
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__puts(gen->stream, "pcc_refill_buffer(ctx, 1) < 1 ||\n");
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__printf(gen->stream, "ctx->buffer.p[ctx->cur] != '%s'\n", escape_character(value[0], &s));
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, ") goto L%04d;\n", onfail);
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "ctx->cur++;\n");
            return CODE_REACH__BOTH;
        }
    }
    else {
        /* no code to generate */
        return CODE_REACH__ALWAYS_SUCCEED;
    }
}

static code_reach_t generate_matching_charclass_code(generate_t *gen, const char *value, int onfail, size_t indent, bool_t bare) {
    assert(gen->ascii);
    if (value != NULL) {
        const size_t n = strlen(value);
        if (n > 0) {
            char s[5], t[5];
            if (n > 1) {
                const bool_t a = (value[0] == '^') ? TRUE : FALSE;
                size_t i = a ? 1 : 0;
                if (i + 1 == n) { /* fulfilled only if a == TRUE */
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "if (\n");
                    stream__write_characters(gen->stream, ' ', indent + 4);
                    stream__puts(gen->stream, "pcc_refill_buffer(ctx, 1) < 1 ||\n");
                    stream__write_characters(gen->stream, ' ', indent + 4);
                    stream__printf(gen->stream, "ctx->buffer.p[ctx->cur] == '%s'\n", escape_character(value[i], &s));
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__printf(gen->stream, ") goto L%04d;\n", onfail);
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "ctx->cur++;\n");
                    return CODE_REACH__BOTH;
                }
                else {
                    if (!bare) {
                        stream__write_characters(gen->stream, ' ', indent);
                        stream__puts(gen->stream, "{\n");
                        indent += 4;
                    }
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "char c;\n");
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__printf(gen->stream, "if (pcc_refill_buffer(ctx, 1) < 1) goto L%04d;\n", onfail);
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "c = ctx->buffer.p[ctx->cur];\n");
                    if (i + 3 == n && value[i] != '\\' && value[i + 1] == '-') {
                        stream__write_characters(gen->stream, ' ', indent);
                        stream__printf(
                            gen->stream,
                            a ? "if (c >= '%s' && c <= '%s') goto L%04d;\n"
                              : "if (!(c >= '%s' && c <= '%s')) goto L%04d;\n",
                            escape_character(value[i], &s), escape_character(value[i + 2], &t), onfail
                        );
                    }
                    else {
                        stream__write_characters(gen->stream, ' ', indent);
                        stream__puts(gen->stream, a ? "if (\n" : "if (!(\n");
                        for (; i < n; i++) {
                            stream__write_characters(gen->stream, ' ', indent + 4);
                            if (value[i] == '\\' && i + 1 < n) i++;
                            if (i + 2 < n && value[i + 1] == '-') {
                                stream__printf(
                                    gen->stream, "(c >= '%s' && c <= '%s')%s\n",
                                    escape_character(value[i], &s), escape_character(value[i + 2], &t), (i + 3 == n) ? "" : " ||"
                                );
                                i += 2;
                            }
                            else {
                                stream__printf(
                                    gen->stream, "c == '%s'%s\n",
                                    escape_character(value[i], &s), (i + 1 == n) ? "" : " ||"
                                );
                            }
                        }
                        stream__write_characters(gen->stream, ' ', indent);
                        stream__printf(gen->stream, a ? ") goto L%04d;\n" : ")) goto L%04d;\n", onfail);
                    }
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "ctx->cur++;\n");
                    if (!bare) {
                        indent -= 4;
                        stream__write_characters(gen->stream, ' ', indent);
                        stream__puts(gen->stream, "}\n");
                    }
                    return CODE_REACH__BOTH;
                }
            }
            else {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "if (\n");
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__puts(gen->stream, "pcc_refill_buffer(ctx, 1) < 1 ||\n");
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__printf(gen->stream, "ctx->buffer.p[ctx->cur] != '%s'\n", escape_character(value[0], &s));
                stream__write_characters(gen->stream, ' ', indent);
                stream__printf(gen->stream, ") goto L%04d;\n", onfail);
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "ctx->cur++;\n");
                return CODE_REACH__BOTH;
            }
        }
        else {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", onfail);
            return CODE_REACH__ALWAYS_FAIL;
        }
    }
    else {
        stream__write_characters(gen->stream, ' ', indent);
        stream__printf(gen->stream, "if (pcc_refill_buffer(ctx, 1) < 1) goto L%04d;\n", onfail);
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "ctx->cur++;\n");
        return CODE_REACH__BOTH;
    }
}

static code_reach_t generate_matching_utf8_charclass_code(generate_t *gen, const char *value, int onfail, size_t indent, bool_t bare) {
    const size_t n = (value != NULL) ? strlen(value) : 0;
    if (value == NULL || n > 0) {
        const bool_t a = (n > 0 && value[0] == '^') ? TRUE : FALSE;
        size_t i = a ? 1 : 0;
        if (!bare) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "{\n");
            indent += 4;
        }
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "int u;\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "const size_t n = pcc_get_char_as_utf32(ctx, &u);\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__printf(gen->stream, "if (n == 0) goto L%04d;\n", onfail);
        if (value != NULL && !(a && n == 1)) { /* not '.' or '[^]' */
            int u0 = 0;
            bool_t r = FALSE;
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, a ? "if (\n" : "if (!(\n");
            while (i < n) {
                int u = 0;
                if (value[i] == '\\' && i + 1 < n) i++;
                i += utf8_to_utf32(value + i, &u);
                if (r) { /* character range */
                    stream__write_characters(gen->stream, ' ', indent + 4);
                    stream__printf(gen->stream, "(u >= 0x%06x && u <= 0x%06x)%s\n", u0, u, (i < n) ? " ||" : "");
                    u0 = 0;
                    r = FALSE;
                }
                else if (
                    value[i] != '-' ||
                    i == n - 1 /* the individual '-' character is valid when it is at the first or the last position */
                ) { /* single character */
                    stream__write_characters(gen->stream, ' ', indent + 4);
                    stream__printf(gen->stream, "u == 0x%06x%s\n", u, (i < n) ? " ||" : "");
                    u0 = 0;
                    r = FALSE;
                }
                else {
                    assert(value[i] == '-');
                    i++;
                    u0 = u;
                    r = TRUE;
                }
            }
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, a ? ") goto L%04d;\n" : ")) goto L%04d;\n", onfail);
        }
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "ctx->cur += n;\n");
        if (!bare) {
            indent -= 4;
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "}\n");
        }
        return CODE_REACH__BOTH;
    }
    else {
        stream__write_characters(gen->stream, ' ', indent);
        stream__printf(gen->stream, "goto L%04d;\n", onfail);
        return CODE_REACH__ALWAYS_FAIL;
    }
}

static code_reach_t generate_position_code(generate_t *gen, size_t value, int onfail, size_t indent, bool_t bare) {
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "if (ctx->pos + ctx->cur != " FMT_LU ") goto L%04d;\n", value, onfail);
    return CODE_REACH__BOTH;
}

static code_reach_t generate_code(generate_t *gen, const node_t *node, int onfail, size_t indent, bool_t bare);

static code_reach_t generate_quantifying_code(generate_t *gen, const node_t *expr, int min, int max, int onfail, size_t indent, bool_t bare) {
    if (max > 1 || max < 0) {
        code_reach_t r;
        if (!bare) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "{\n");
            indent += 4;
        }
        if (min > 0) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "const size_t p0 = ctx->cur;\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "const size_t n0 = chunk->thunks.n;\n");
        }
        if (max < 0) {
            if (min > 0) {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "int i;\n");
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "for (i = 0;; i++) {\n");
            }
            else {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "for (;;) {\n");
            }
        }
        else {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "int i;\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "for (i = 0; i < %d; i++) {\n", max);
        }
        stream__write_characters(gen->stream, ' ', indent + 4);
        stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
        stream__write_characters(gen->stream, ' ', indent + 4);
        stream__puts(gen->stream, "const size_t n = chunk->thunks.n;\n");
        {
            const int l = ++gen->label;
            r = generate_code(gen, expr, l, indent + 4, FALSE);
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__puts(gen->stream, "if (ctx->cur == p) break;\n");
            if (r != CODE_REACH__ALWAYS_SUCCEED) {
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__puts(gen->stream, "continue;\n");
                stream__write_characters(gen->stream, ' ', indent);
                stream__printf(gen->stream, "L%04d:;\n", l);
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__puts(gen->stream, "ctx->cur = p;\n");
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__puts(gen->stream, "pcc_thunk_array__revert(ctx, &(chunk->thunks), n);\n");
                stream__write_characters(gen->stream, ' ', indent + 4);
                stream__puts(gen->stream, "break;\n");
            }
        }
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
        if (min > 0) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "if (i < %d) {\n", min);
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__puts(gen->stream, "ctx->cur = p0;\n");
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__puts(gen->stream, "pcc_thunk_array__revert(ctx, &(chunk->thunks), n0);\n");
            stream__write_characters(gen->stream, ' ', indent + 4);
            stream__printf(gen->stream, "goto L%04d;\n", onfail);
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "}\n");
        }
        if (!bare) {
            indent -= 4;
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "}\n");
        }
        return (min > 0) ? ((r == CODE_REACH__ALWAYS_FAIL) ? CODE_REACH__ALWAYS_FAIL : CODE_REACH__BOTH) : CODE_REACH__ALWAYS_SUCCEED;
    }
    else if (max == 1) {
        if (min > 0) {
            return generate_code(gen, expr, onfail, indent, bare);
        }
        else {
            if (!bare) {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "{\n");
                indent += 4;
            }
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "MARK_VAR_AS_USED\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "MARK_VAR_AS_USED\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "const size_t n = chunk->thunks.n;\n");
            {
                const int l = ++gen->label;
                if (generate_code(gen, expr, l, indent, FALSE) != CODE_REACH__ALWAYS_SUCCEED) {
                    const int m = ++gen->label;
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__printf(gen->stream, "goto L%04d;\n", m);
                    if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
                    stream__printf(gen->stream, "L%04d:;\n", l);
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "ctx->cur = p;\n");
                    stream__write_characters(gen->stream, ' ', indent);
                    stream__puts(gen->stream, "pcc_thunk_array__revert(ctx, &(chunk->thunks), n);\n");
                    if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
                    stream__printf(gen->stream, "L%04d:;\n", m);
                }
            }
            if (!bare) {
                indent -= 4;
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "}\n");
            }
            return CODE_REACH__ALWAYS_SUCCEED;
        }
    }
    else {
        /* no code to generate */
        return CODE_REACH__ALWAYS_SUCCEED;
    }
}

static code_reach_t generate_predicating_code(generate_t *gen, const node_t *expr, bool_t neg, int onfail, size_t indent, bool_t bare) {
    code_reach_t r;
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
    if (neg) {
        const int l = ++gen->label;
        r = generate_code(gen, expr, l, indent, FALSE);
        if (r != CODE_REACH__ALWAYS_FAIL) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "ctx->cur = p;\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", onfail);
        }
        if (r != CODE_REACH__ALWAYS_SUCCEED) {
            if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
            stream__printf(gen->stream, "L%04d:;\n", l);
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "ctx->cur = p;\n");
        }
        switch (r) {
        case CODE_REACH__ALWAYS_SUCCEED: r = CODE_REACH__ALWAYS_FAIL; break;
        case CODE_REACH__ALWAYS_FAIL: r = CODE_REACH__ALWAYS_SUCCEED; break;
        case CODE_REACH__BOTH: break;
        }
    }
    else {
        const int l = ++gen->label;
        const int m = ++gen->label;
        r = generate_code(gen, expr, l, indent, FALSE);
        if (r != CODE_REACH__ALWAYS_FAIL) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "ctx->cur = p;\n");
        }
        if (r == CODE_REACH__BOTH) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", m);
        }
        if (r != CODE_REACH__ALWAYS_SUCCEED) {
            if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
            stream__printf(gen->stream, "L%04d:;\n", l);
            stream__write_characters(gen->stream, ' ', indent);
            stream__puts(gen->stream, "ctx->cur = p;\n");
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", onfail);
        }
        if (r == CODE_REACH__BOTH) {
            if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
            stream__printf(gen->stream, "L%04d:;\n", m);
        }
    }
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return r;
}

static code_reach_t generate_progpred_code(generate_t *gen, size_t index, bool_t neg, int onfail, size_t indent, bool_t bare) {
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "int r = %s;\n", neg ? "0" : "1");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "ctx->capt0.range.start = chunk->pos;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "ctx->capt0.range.end = ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "pcc_char_array__resize(ctx->auxil, &(ctx->capt0.string), 0);\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(
        gen->stream, "pcc_predicate_%s_" FMT_LU "(ctx, chunk, &r);\n",
        gen->rule->data.rule.name, (ulong_t)index
    );
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "ctx->cur = p;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "if (%sr) goto L%04d;\n", neg ? "" : "!", onfail);
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return CODE_REACH__BOTH;
}

static code_reach_t generate_sequential_code(generate_t *gen, const node_array_t *nodes, int onfail, size_t indent, bool_t bare) {
    bool_t b = FALSE;
    size_t i;
    for (i = 0; i < nodes->n; i++) {
        switch (generate_code(gen, nodes->p[i], onfail, indent, FALSE)) {
        case CODE_REACH__ALWAYS_FAIL:
            if (i + 1 < nodes->n) {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "/* unreachable codes omitted */\n");
            }
            return CODE_REACH__ALWAYS_FAIL;
        case CODE_REACH__ALWAYS_SUCCEED:
            break;
        default:
            b = TRUE;
        }
    }
    return b ? CODE_REACH__BOTH : CODE_REACH__ALWAYS_SUCCEED;
}

static code_reach_t generate_alternative_code(generate_t *gen, const node_array_t *nodes, int onfail, size_t indent, bool_t bare) {
    bool_t b = FALSE;
    int m = ++gen->label;
    size_t i;
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "MARK_VAR_AS_USED\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "MARK_VAR_AS_USED\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "const size_t n = chunk->thunks.n;\n");
    for (i = 0; i < nodes->n; i++) {
        const bool_t c = (i + 1 < nodes->n) ? TRUE : FALSE;
        const int l = ++gen->label;
        switch (generate_code(gen, nodes->p[i], l, indent, FALSE)) {
        case CODE_REACH__ALWAYS_SUCCEED:
            if (c) {
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "/* unreachable codes omitted */\n");
            }
            if (b) {
                if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
                stream__printf(gen->stream, "L%04d:;\n", m);
            }
            if (!bare) {
                indent -= 4;
                stream__write_characters(gen->stream, ' ', indent);
                stream__puts(gen->stream, "}\n");
            }
            return CODE_REACH__ALWAYS_SUCCEED;
        case CODE_REACH__ALWAYS_FAIL:
            break;
        default:
            b = TRUE;
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", m);
        }
        if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
        stream__printf(gen->stream, "L%04d:;\n", l);
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "ctx->cur = p;\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "pcc_thunk_array__revert(ctx, &(chunk->thunks), n);\n");
        if (!c) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(gen->stream, "goto L%04d;\n", onfail);
        }
    }
    if (b) {
        if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
        stream__printf(gen->stream, "L%04d:;\n", m);
    }
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return b ? CODE_REACH__BOTH : CODE_REACH__ALWAYS_FAIL;
}

static code_reach_t generate_capturing_code(generate_t *gen, const node_t *expr, size_t index, int onfail, size_t indent, bool_t bare) {
    code_reach_t r;
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "const size_t p = ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "size_t q;\n");
    r = generate_code(gen, expr, onfail, indent, FALSE);
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "q = ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "chunk->capts.p[" FMT_LU "].range.start = p;\n", (ulong_t)index);
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "chunk->capts.p[" FMT_LU "].range.end = q;\n", (ulong_t)index);
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "pcc_char_array__resize(ctx->auxil, &(chunk->capts.p[" FMT_LU "].string), 0);\n", (ulong_t)index);
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return r;
}

static code_reach_t generate_expanding_code(generate_t *gen, size_t index, int onfail, size_t indent, bool_t bare) {
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(
        gen->stream, "const size_t n = chunk->capts.p[" FMT_LU "].range.end - chunk->capts.p[" FMT_LU "].range.start;\n",
        (ulong_t)index, (ulong_t)index
    );
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "if (pcc_refill_buffer(ctx, n) < n) goto L%04d;\n", onfail);
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "if (n > 0) {\n");
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__puts(gen->stream, "const char *const p = ctx->buffer.p + ctx->cur;\n");
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__printf(gen->stream, "const char *const q = ctx->buffer.p + chunk->capts.p[" FMT_LU "].range.start;\n", (ulong_t)index);
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__puts(gen->stream, "size_t i;\n");
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__puts(gen->stream, "for (i = 0; i < n; i++) {\n");
    stream__write_characters(gen->stream, ' ', indent + 8);
    stream__printf(gen->stream, "if (p[i] != q[i]) goto L%04d;\n", onfail);
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__puts(gen->stream, "}\n");
    stream__write_characters(gen->stream, ' ', indent + 4);
    stream__puts(gen->stream, "ctx->cur += n;\n");
    stream__write_characters(gen->stream, ' ', indent);
    stream__puts(gen->stream, "}\n");
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return CODE_REACH__BOTH;
}

static code_reach_t generate_thunking_action_code(
    generate_t *gen, size_t index, const node_const_array_t *rvars, const node_const_array_t *capts, bool_t error, int onfail, size_t indent, bool_t bare
) {
    assert(gen->rule->type == NODE_RULE);
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    if (error) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "pcc_value_t null;\n");
    }
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(
        gen->stream, "pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx, pcc_action_%s_" FMT_LU ", " FMT_LU ", " FMT_LU ");\n",
        gen->rule->data.rule.name, (ulong_t)index, (ulong_t)gen->rule->data.rule.rvars.n, (ulong_t)gen->rule->data.rule.capts.n
    );
    {
        size_t i;
        for (i = 0; i < rvars->n; i++) {
            assert(rvars->p[i]->type == NODE_REFERENCE);
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(
                gen->stream, "thunk->data.leaf.values.p[" FMT_LU "] = &(chunk->values.p[" FMT_LU "]);\n",
                (ulong_t)rvars->p[i]->data.reference.index, (ulong_t)rvars->p[i]->data.reference.index
            );
        }
        for (i = 0; i < capts->n; i++) {
            assert(capts->p[i]->type == NODE_CAPTURE);
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(
                gen->stream, "thunk->data.leaf.capts.p[" FMT_LU "] = &(chunk->capts.p[" FMT_LU "]);\n",
                (ulong_t)capts->p[i]->data.capture.index, (ulong_t)capts->p[i]->data.capture.index
            );
        }
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "thunk->data.leaf.capt0.range.start = chunk->pos;\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "thunk->data.leaf.capt0.range.end = ctx->cur;\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "pcc_char_array__resize(ctx->auxil, &(thunk->data.leaf.capt0.string), 0);\n");
    }
    if (error) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "memset(&null, 0, sizeof(pcc_value_t)); /* in case */\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "thunk->data.leaf.action(ctx, thunk, &null);\n");
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "pcc_thunk__destroy(ctx, thunk);\n");
    }
    else {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "pcc_thunk_array__add(ctx, &(chunk->thunks), thunk);\n");
    }
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return CODE_REACH__ALWAYS_SUCCEED;
}

static code_reach_t generate_thunking_error_code(
    generate_t *gen, const node_t *expr, size_t index, const node_const_array_t *rvars, const node_const_array_t *capts, int onfail, size_t indent, bool_t bare
) {
    code_reach_t r;
    const int l = ++gen->label;
    const int m = ++gen->label;
    assert(gen->rule->type == NODE_RULE);
    if (!bare) {
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "{\n");
        indent += 4;
    }
    r = generate_code(gen, expr, l, indent, TRUE);
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "goto L%04d;\n", m);
    if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
    stream__printf(gen->stream, "L%04d:;\n", l);
    generate_thunking_action_code(gen, index, rvars, capts, TRUE, l, indent, FALSE);
    stream__write_characters(gen->stream, ' ', indent);
    stream__printf(gen->stream, "goto L%04d;\n", onfail);
    if (indent > 4) stream__write_characters(gen->stream, ' ', indent - 4);
    stream__printf(gen->stream, "L%04d:;\n", m);
    if (!bare) {
        indent -= 4;
        stream__write_characters(gen->stream, ' ', indent);
        stream__puts(gen->stream, "}\n");
    }
    return r;
}

static code_reach_t generate_code(generate_t *gen, const node_t *node, int onfail, size_t indent, bool_t bare) {
    if (node == NULL) {
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
    switch (node->type) {
    case NODE_RULE:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    case NODE_REFERENCE:
        if (node->data.reference.index != VOID_VALUE) {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(
                gen->stream, "if (!pcc_apply_rule(ctx, pcc_evaluate_rule_%s, &(chunk->thunks), &(chunk->values.p[" FMT_LU "]))) goto L%04d;\n",
                node->data.reference.name, (ulong_t)node->data.reference.index, onfail
            );
        }
        else {
            stream__write_characters(gen->stream, ' ', indent);
            stream__printf(
                gen->stream, "if (!pcc_apply_rule(ctx, pcc_evaluate_rule_%s, &(chunk->thunks), NULL)) goto L%04d;\n",
                node->data.reference.name, onfail
            );
        }
        return CODE_REACH__BOTH;
    case NODE_STRING:
        return generate_matching_string_code(gen, node->data.string.value, onfail, indent, bare);
    case NODE_CHARCLASS:
        return gen->ascii ?
               generate_matching_charclass_code(gen, node->data.charclass.value, onfail, indent, bare) :
               generate_matching_utf8_charclass_code(gen, node->data.charclass.value, onfail, indent, bare);
    case NODE_POSITION:
        return generate_position_code(gen, node->data.position.value, onfail, indent, bare);
    case NODE_QUANTITY:
        return generate_quantifying_code(gen, node->data.quantity.expr, node->data.quantity.min, node->data.quantity.max, onfail, indent, bare);
    case NODE_PREDICATE:
        return generate_predicating_code(gen, node->data.predicate.expr, node->data.predicate.neg, onfail, indent, bare);
    case NODE_PROGPRED:
        return generate_progpred_code(gen, node->data.progpred.index, node->data.progpred.neg, onfail, indent, bare);
    case NODE_SEQUENCE:
        return generate_sequential_code(gen, &(node->data.sequence.nodes), onfail, indent, bare);
    case NODE_ALTERNATE:
        return generate_alternative_code(gen, &(node->data.alternate.nodes), onfail, indent, bare);
    case NODE_CAPTURE:
        return generate_capturing_code(gen, node->data.capture.expr, node->data.capture.index, onfail, indent, bare);
    case NODE_EXPAND:
        return generate_expanding_code(gen, node->data.expand.index, onfail, indent, bare);
    case NODE_ACTION:
        return generate_thunking_action_code(
            gen, node->data.action.index, &(node->data.action.rvars), &(node->data.action.capts), FALSE, onfail, indent, bare
        );
    case NODE_ERROR:
        return generate_thunking_error_code(
            gen, node->data.error.expr, node->data.error.index, &(node->data.error.rvars), &(node->data.error.capts), onfail, indent, bare
        );
    default:
        print_error("Internal error [%d]\n", __LINE__);
        exit(-1);
    }
}

static bool_t generate(context_t *ctx) {
    const char *const vt = get_value_type(ctx);
    const char *const at = get_auxil_type(ctx);
    const bool_t vp = is_pointer_type(vt);
    const bool_t ap = is_pointer_type(at);
    stream_t sstream = stream__wrap(fopen_wt_e(ctx->spath), ctx->spath, ctx->opts.lines ? 0 : VOID_VALUE);
    stream_t hstream = stream__wrap(fopen_wt_e(ctx->hpath), ctx->hpath, ctx->opts.lines ? 0 : VOID_VALUE);
    stream__printf(&sstream, "/* A packrat parser generated by PackCC %s */\n\n", VERSION);
    stream__printf(&hstream, "/* A packrat parser generated by PackCC %s */\n\n", VERSION);
    {
        subst_map__clear(&(ctx->subst));
        subst_map__add(&(ctx->subst), "prefix", get_prefix(ctx));
        subst_map__add(&(ctx->subst), "PREFIX", get_prefix(ctx));
        to_uppercase(ctx->subst.p[ctx->subst.n - 1].ostr);
    }
    {
        {
            size_t i;
            for (i = 0; i < ctx->eheader.n; i++) {
                stream__write_code_block(
                    &hstream, ctx->eheader.p[i].text, ctx->eheader.p[i].len, 0,
                    ctx->eheader.p[i].fpos.path, ctx->eheader.p[i].fpos.line, FALSE, &(ctx->subst)
                );
                stream__puts(&hstream, "\n");
            }
        }
        stream__printf(
            &hstream,
            "#ifndef PCC_INCLUDED_%s\n"
            "#define PCC_INCLUDED_%s\n"
            "\n",
            ctx->hid, ctx->hid
        );
        {
            size_t i;
            for (i = 0; i < ctx->header.n; i++) {
                stream__write_code_block(
                    &hstream, ctx->header.p[i].text, ctx->header.p[i].len, 0,
                    ctx->header.p[i].fpos.path, ctx->header.p[i].fpos.line, FALSE, &(ctx->subst)
                );
                stream__puts(&hstream, "\n");
            }
        }
    }
    {
        {
            size_t i;
            for (i = 0; i < ctx->esource.n; i++) {
                stream__write_code_block(
                    &sstream, ctx->esource.p[i].text, ctx->esource.p[i].len, 0,
                    ctx->esource.p[i].fpos.path, ctx->esource.p[i].fpos.line, FALSE, &(ctx->subst)
                );
                stream__puts(&sstream, "\n");
            }
        }
        stream__puts(
            &sstream,
            "#ifdef _MSC_VER\n"
            "#undef _CRT_SECURE_NO_WARNINGS\n"
            "#define _CRT_SECURE_NO_WARNINGS\n"
            "#endif /* _MSC_VER */\n"
            "#include <stdio.h>\n"
            "#include <stdlib.h>\n"
            "#include <string.h>\n"
            "\n"
        );
        stream__printf(
            &sstream,
            "#include \"%s\"\n"
            "\n",
            extract_filename(ctx->hpath)
        );
        if (ctx->opts.ascii || ctx->opts.lines) {
            if (ctx->opts.ascii) {
                stream__puts(
                    &sstream,
                    "#define PCC_OPTION_ASCII_ENABLED\n"
                );
            }
            if (ctx->opts.lines) {
                stream__puts(
                    &sstream,
                    "#define PCC_OPTION_LINES_ENABLED\n"
                );
            }
            stream__puts(
                &sstream,
                "\n"
            );
        }
        {
            size_t i;
            for (i = 0; i < ctx->source.n; i++) {
                stream__write_code_block(
                    &sstream, ctx->source.p[i].text, ctx->source.p[i].len, 0,
                    ctx->source.p[i].fpos.path, ctx->source.p[i].fpos.line, FALSE, &(ctx->subst)
                );
                stream__puts(&sstream, "\n");
            }
        }
    }
    {
        stream__puts(
            &sstream,
            "#if !defined __has_attribute || defined _MSC_VER\n"
            "#define __attribute__(x)\n"
            "#endif\n"
            "\n"
            "#ifdef _MSC_VER\n"
            "#define MARK_FUNC_AS_USED __pragma(warning(suppress:4505))\n"
            "#else\n"
            "#define MARK_FUNC_AS_USED __attribute__((__unused__))\n"
            "#endif\n"
            "\n"
            "#ifdef _MSC_VER\n"
            "#define MARK_VAR_AS_USED __pragma(warning(suppress:4189))\n"
            "#else\n"
            "#define MARK_VAR_AS_USED __attribute__((__unused__))\n"
            "#endif\n"
            "\n"
            "#ifndef PCC_BUFFER_MIN_SIZE\n"
            "#define PCC_BUFFER_MIN_SIZE 256\n"
            "#endif /* !PCC_BUFFER_MIN_SIZE */\n"
            "\n"
            "#ifndef PCC_ARRAY_MIN_SIZE\n"
            "#define PCC_ARRAY_MIN_SIZE 2\n"
            "#endif /* !PCC_ARRAY_MIN_SIZE */\n"
            "\n"
            "#ifndef PCC_THUNK_ARRAY_MIN_SIZE\n"
            "#define PCC_THUNK_ARRAY_MIN_SIZE 2\n"
            "#endif /* !PCC_THUNK_ARRAY_MIN_SIZE */\n"
            "\n"
            "#ifndef PCC_LR_MEMO_MAP_MIN_SIZE\n"
            "#define PCC_LR_MEMO_MAP_MIN_SIZE 4\n"
            "#endif /* !PCC_LR_MEMO_MAP_MIN_SIZE */\n"
            "\n"
            "#ifndef PCC_POOL_MIN_SIZE\n"
            "#define PCC_POOL_MIN_SIZE 65536\n"
            "#endif /* !PCC_POOL_MIN_SIZE */\n"
            "\n"
            "#define PCC_DBG_EVALUATE 0\n"
            "#define PCC_DBG_MATCH    1\n"
            "#define PCC_DBG_NOMATCH  2\n"
            "\n"
            "#define PCC_VOID_VALUE (~(size_t)0)\n"
            "\n"
            "typedef enum pcc_bool_tag {\n"
            "    PCC_FALSE = 0,\n"
            "    PCC_TRUE\n"
            "} pcc_bool_t;\n"
            "\n"
            "typedef struct pcc_char_array_tag {\n"
            "    size_t m, n;\n"
            "    char *p;\n"
            "} pcc_char_array_t;\n"
            "\n"
            "typedef struct pcc_range_tag {\n"
            "    size_t start;\n"
            "    size_t end;\n"
            "} pcc_range_t;\n"
            "\n"
        );
        stream__printf(
            &sstream,
            "typedef %s%spcc_value_t;\n"
            "\n",
            vt, vp ? "" : " "
        );
        stream__printf(
            &sstream,
            "typedef %s%spcc_auxil_t;\n"
            "\n",
            at, ap ? "" : " "
        );
        if (strcmp(get_prefix(ctx), "pcc") != 0) {
            stream__printf(
                &sstream,
                "typedef %s_context_t pcc_context_t;\n"
                "\n",
                get_prefix(ctx)
            );
        }
        stream__puts(
            &sstream,
            "typedef struct pcc_value_table_tag {\n"
            "    size_t m, n;\n"
            "    pcc_value_t *p;\n"
            "} pcc_value_table_t;\n"
            "\n"
            "typedef struct pcc_value_refer_table_tag {\n"
            "    size_t m, n;\n"
            "    pcc_value_t **p;\n"
            "} pcc_value_refer_table_t;\n"
            "\n"
            "typedef struct pcc_capture_tag {\n"
            "    pcc_range_t range;\n"
            "    pcc_char_array_t string; /* mutable */\n"
            "} pcc_capture_t;\n"
            "\n"
            "typedef struct pcc_capture_table_tag {\n"
            "    size_t m, n;\n"
            "    pcc_capture_t *p;\n"
            "} pcc_capture_table_t;\n"
            "\n"
            "typedef struct pcc_capture_const_table_tag {\n"
            "    size_t m, n;\n"
            "    const pcc_capture_t **p;\n"
            "} pcc_capture_const_table_t;\n"
            "\n"
            "typedef struct pcc_thunk_tag pcc_thunk_t;\n"
            "typedef struct pcc_thunk_array_tag pcc_thunk_array_t;\n"
            "\n"
            "typedef void (*pcc_action_t)(pcc_context_t *, pcc_thunk_t *, pcc_value_t *);\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "typedef enum pcc_thunk_type_tag {\n"
            "    PCC_THUNK_LEAF,\n"
            "    PCC_THUNK_NODE\n"
            "} pcc_thunk_type_t;\n"
            "\n"
            "typedef struct pcc_thunk_leaf_tag {\n"
            "    pcc_value_refer_table_t values;\n"
            "    pcc_capture_const_table_t capts;\n"
            "    pcc_capture_t capt0;\n"
            "    pcc_action_t action;\n"
            "} pcc_thunk_leaf_t;\n"
            "\n"
            "typedef struct pcc_thunk_node_tag {\n"
            "    const pcc_thunk_array_t *thunks; /* just a reference */\n"
            "    pcc_value_t *value; /* just a reference */\n"
            "} pcc_thunk_node_t;\n"
            "\n"
            "typedef union pcc_thunk_data_tag {\n"
            "    pcc_thunk_leaf_t leaf;\n"
            "    pcc_thunk_node_t node;\n"
            "} pcc_thunk_data_t;\n"
            "\n"
            "struct pcc_thunk_tag {\n"
            "    pcc_thunk_type_t type;\n"
            "    pcc_thunk_data_t data;\n"
            "};\n"
            "\n"
            "struct pcc_thunk_array_tag {\n"
            "    size_t m, n;\n"
            "    pcc_thunk_t **p;\n"
            "    pcc_thunk_t *a[PCC_THUNK_ARRAY_MIN_SIZE]; /* for reducing memory allocation frequency */\n"
            "};\n"
            "\n"
            "typedef struct pcc_thunk_chunk_tag {\n"
            "    pcc_value_table_t values;\n"
            "    pcc_capture_table_t capts;\n"
            "    pcc_thunk_array_t thunks;\n"
            "    size_t pos; /* the starting position in the character buffer */\n"
            "} pcc_thunk_chunk_t;\n"
            "\n"
            "typedef struct pcc_lr_entry_tag pcc_lr_entry_t;\n"
            "\n"
            "typedef enum pcc_lr_answer_type_tag {\n"
            "    PCC_LR_ANSWER_LR,\n"
            "    PCC_LR_ANSWER_CHUNK\n"
            "} pcc_lr_answer_type_t;\n"
            "\n"
            "typedef union pcc_lr_answer_data_tag {\n"
            "    pcc_lr_entry_t *lr;\n"
            "    pcc_thunk_chunk_t *chunk;\n"
            "} pcc_lr_answer_data_t;\n"
            "\n"
            "typedef struct pcc_lr_answer_tag pcc_lr_answer_t;\n"
            "\n"
            "struct pcc_lr_answer_tag {\n"
            "    pcc_lr_answer_type_t type;\n"
            "    pcc_lr_answer_data_t data;\n"
            "    size_t pos; /* the absolute position in the input */\n"
            "    pcc_lr_answer_t *hold;\n"
            "};\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "typedef pcc_thunk_chunk_t *(*pcc_rule_t)(pcc_context_t *);\n"
            "\n"
            "typedef struct pcc_rule_set_tag {\n"
            "    size_t m, n;\n"
            "    pcc_rule_t *p;\n"
            "} pcc_rule_set_t;\n"
            "\n"
            "typedef struct pcc_lr_head_tag pcc_lr_head_t;\n"
            "\n"
            "struct pcc_lr_head_tag {\n"
            "    pcc_rule_t rule;\n"
            "    pcc_rule_set_t invol;\n"
            "    pcc_rule_set_t eval;\n"
            "    pcc_lr_head_t *hold;\n"
            "};\n"
            "\n"
            "typedef struct pcc_lr_memo_tag {\n"
            "    pcc_rule_t rule;\n"
            "    pcc_lr_answer_t *answer;\n"
            "} pcc_lr_memo_t;\n"
            "\n"
            "typedef struct pcc_lr_memo_map_tag {\n"
            "    size_t m, n;\n"
            "    pcc_lr_memo_t *p;\n"
            "    pcc_lr_memo_t a[PCC_LR_MEMO_MAP_MIN_SIZE]; /* for reducing memory allocation frequency */\n"
            "} pcc_lr_memo_map_t;\n"
            "\n"
            "typedef struct pcc_lr_table_entry_tag {\n"
            "    pcc_lr_head_t *head; /* just a reference */\n"
            "    pcc_lr_memo_map_t memos;\n"
            "    pcc_lr_answer_t *hold_a;\n"
            "    pcc_lr_head_t *hold_h;\n"
            "} pcc_lr_table_entry_t;\n"
            "\n"
            "typedef struct pcc_lr_table_tag {\n"
            "    size_t m, n;\n"
            "    size_t o;\n"
            "    pcc_lr_table_entry_t **p;\n"
            "} pcc_lr_table_t;\n"
            "\n"
            "struct pcc_lr_entry_tag {\n"
            "    pcc_rule_t rule;\n"
            "    pcc_thunk_chunk_t *seed; /* just a reference */\n"
            "    pcc_lr_head_t *head; /* just a reference */\n"
            "};\n"
            "\n"
            "typedef struct pcc_lr_stack_tag {\n"
            "    size_t m, n;\n"
            "    pcc_lr_entry_t **p;\n"
            "} pcc_lr_stack_t;\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "typedef struct pcc_memory_entry_tag pcc_memory_entry_t;\n"
            "typedef struct pcc_memory_pool_tag pcc_memory_pool_t;\n"
            "\n"
            "struct pcc_memory_entry_tag {\n"
            "    pcc_memory_entry_t *next;\n"
            "};\n"
            "\n"
            "struct pcc_memory_pool_tag {\n"
            "    pcc_memory_pool_t *next;\n"
            "    size_t allocated;\n"
            "    size_t unused;\n"
            "};\n"
            "\n"
            "typedef struct pcc_memory_recycler_tag {\n"
            "    pcc_memory_pool_t *pool_list;\n"
            "    pcc_memory_entry_t *entry_list;\n"
            "    size_t element_size;\n"
            "} pcc_memory_recycler_t;\n"
            "\n"
        );
        stream__printf(
            &sstream,
            "struct %s_context_tag {\n"
            "    size_t pos; /* the position in the input of the first character currently buffered */\n"
            "    size_t cur; /* the current parsing position in the character buffer */\n"
            "    size_t level;\n"
            "    pcc_char_array_t buffer;\n"
            "    pcc_lr_table_t lrtable;\n"
            "    pcc_lr_stack_t lrstack;\n"
            "    pcc_thunk_array_t thunks;\n"
            "    pcc_auxil_t auxil;\n"
            "    pcc_capture_t capt0; /* used for programmable predicates */\n"
            "    pcc_memory_recycler_t thunk_recycler;\n"
            "    pcc_memory_recycler_t thunk_chunk_recycler;\n"
            "    pcc_memory_recycler_t lr_head_recycler;\n"
            "    pcc_memory_recycler_t lr_answer_recycler;\n"
            "    pcc_memory_recycler_t lr_table_entry_recycler;\n"
            "    pcc_memory_recycler_t lr_entry_recycler;\n"
            "};\n"
            "\n",
            get_prefix(ctx)
        );
        stream__puts(
            &sstream,
            "#ifndef PCC_ERROR\n"
            "#define PCC_ERROR(auxil) pcc_error()\n"
            "MARK_FUNC_AS_USED\n"
            "static void pcc_error(void) {\n"
            "    fprintf(stderr, \"Syntax error\\n\");\n"
            "    exit(1);\n"
            "}\n"
            "#endif /* !PCC_ERROR */\n"
            "\n"
            "#ifndef PCC_GETCHAR\n"
            "#define PCC_GETCHAR(auxil) getchar()\n"
            "#endif /* !PCC_GETCHAR */\n"
            "\n"
            "#ifndef PCC_MALLOC\n"
            "#define PCC_MALLOC(auxil, size) pcc_malloc_e(size)\n"
            "static void *pcc_malloc_e(size_t size) {\n"
            "    void *const p = malloc(size);\n"
            "    if (p == NULL) {\n"
            "        fprintf(stderr, \"Out of memory\\n\");\n"
            "        exit(1);\n"
            "    }\n"
            "    return p;\n"
            "}\n"
            "#endif /* !PCC_MALLOC */\n"
            "\n"
            "#ifndef PCC_REALLOC\n"
            "#define PCC_REALLOC(auxil, ptr, size) pcc_realloc_e(ptr, size)\n"
            "static void *pcc_realloc_e(void *ptr, size_t size) {\n"
            "    void *const p = realloc(ptr, size);\n"
            "    if (p == NULL) {\n"
            "        fprintf(stderr, \"Out of memory\\n\");\n"
            "        exit(1);\n"
            "    }\n"
            "    return p;\n"
            "}\n"
            "#endif /* !PCC_REALLOC */\n"
            "\n"
            "#ifndef PCC_FREE\n"
            "#define PCC_FREE(auxil, ptr) free(ptr)\n"
            "#endif /* !PCC_FREE */\n"
            "\n"
            "#ifndef PCC_DEBUG\n"
            "#define PCC_DEBUG(auxil, event, rule, level, pos, buffer, length) ((void)0)\n"
            "#endif /* !PCC_DEBUG */\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_char_array__initialize(pcc_auxil_t auxil, pcc_char_array_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_char_array__finalize(pcc_auxil_t auxil, pcc_char_array_t *obj) {\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_char_array__resize(pcc_auxil_t auxil, pcc_char_array_t *obj, size_t len) {\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_BUFFER_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (char *)PCC_REALLOC(auxil, obj->p, m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
            "static void pcc_char_array__add(pcc_auxil_t auxil, pcc_char_array_t *obj, char ch) {\n"
            "    if (obj->m <= obj->n) {\n"
            "        const size_t n = obj->n + 1;\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_BUFFER_MIN_SIZE;\n"
            "        while (m < n && m != 0) m <<= 1;\n"
            "        if (m == 0) m = n;\n"
            "        obj->p = (char *)PCC_REALLOC(auxil, obj->p, m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->p[obj->n++] = ch;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_value_table__initialize(pcc_auxil_t auxil, pcc_value_table_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_value_table__finalize(pcc_auxil_t auxil, pcc_value_table_t *obj) {\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "MARK_FUNC_AS_USED\n"
            "static void pcc_value_table__clear(pcc_auxil_t auxil, pcc_value_table_t *obj) {\n"
            "    memset(obj->p, 0, sizeof(pcc_value_t) * obj->n);\n"
            "}\n"
            "\n"
            "MARK_FUNC_AS_USED\n"
            "static void pcc_value_table__resize(pcc_auxil_t auxil, pcc_value_table_t *obj, size_t len) {\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (pcc_value_t *)PCC_REALLOC(auxil, obj->p, sizeof(pcc_value_t) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_value_refer_table__initialize(pcc_auxil_t auxil, pcc_value_refer_table_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_value_refer_table__finalize(pcc_auxil_t auxil, pcc_value_refer_table_t *obj) {\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_value_refer_table__resize(pcc_auxil_t auxil, pcc_value_refer_table_t *obj, size_t len) {\n"
            "    size_t i;\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (pcc_value_t **)PCC_REALLOC(auxil, obj->p, sizeof(pcc_value_t *) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    for (i = obj->n; i < len; i++) obj->p[i] = NULL;\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_capture__initialize(pcc_auxil_t auxil, pcc_capture_t *obj) {\n"
            "    obj->range.start = 0;\n"
            "    obj->range.end = 0;\n"
            "    pcc_char_array__initialize(auxil, &(obj->string));\n"
            "}\n"
            "\n"
            "static void pcc_capture__finalize(pcc_auxil_t auxil, pcc_capture_t *obj) {\n"
            "    pcc_char_array__finalize(auxil, &(obj->string));\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_capture_table__initialize(pcc_auxil_t auxil, pcc_capture_table_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_capture_table__finalize(pcc_auxil_t auxil, pcc_capture_table_t *obj) {\n"
            "    while (obj->n > 0) {\n"
            "        obj->n--;\n"
            "        pcc_capture__finalize(auxil, &(obj->p[obj->n]));\n"
            "    }\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "MARK_FUNC_AS_USED\n"
            "static void pcc_capture_table__resize(pcc_auxil_t auxil, pcc_capture_table_t *obj, size_t len) {\n"
            "    size_t i;\n"
            "    for (i = len; i < obj->n; i++) pcc_capture__finalize(auxil, &(obj->p[i]));\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (pcc_capture_t *)PCC_REALLOC(auxil, obj->p, sizeof(pcc_capture_t) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    for (i = obj->n; i < len; i++) pcc_capture__initialize(auxil, &(obj->p[i]));\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_capture_const_table__initialize(pcc_auxil_t auxil, pcc_capture_const_table_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_capture_const_table__finalize(pcc_auxil_t auxil, pcc_capture_const_table_t *obj) {\n"
            "    PCC_FREE(auxil, (void *)obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_capture_const_table__resize(pcc_auxil_t auxil, pcc_capture_const_table_t *obj, size_t len) {\n"
            "    size_t i;\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (const pcc_capture_t **)PCC_REALLOC(auxil, (pcc_capture_t **)obj->p, sizeof(const pcc_capture_t *) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    for (i = obj->n; i < len; i++) obj->p[i] = NULL;\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_memory_recycler__initialize(pcc_auxil_t auxil, pcc_memory_recycler_t *obj, size_t element_size) {\n"
            "    obj->pool_list = NULL;\n"
            "    obj->entry_list = NULL;\n"
            "    obj->element_size = element_size;\n"
            "}\n"
            "\n"
            "static void pcc_memory_recycler__finalize(pcc_auxil_t auxil, pcc_memory_recycler_t *obj) {\n"
            "    while (obj->pool_list) {\n"
            "        pcc_memory_pool_t *const pool = obj->pool_list;\n"
            "        obj->pool_list = pool->next;\n"
            "        PCC_FREE(auxil, pool);\n"
            "    }\n"
            "}\n"
            "\n"
            "static void *pcc_memory_recycler__supply(pcc_auxil_t auxil, pcc_memory_recycler_t *obj) {\n"
            "    if (obj->entry_list) {\n"
            "        pcc_memory_entry_t *const tmp = obj->entry_list;\n"
            "        obj->entry_list = tmp->next;\n"
            "        return tmp;\n"
            "    }\n"
            "    if (!obj->pool_list || obj->pool_list->unused == 0) {\n"
            "        size_t size = PCC_POOL_MIN_SIZE;\n"
            "        if (obj->pool_list) {\n"
            "            size = obj->pool_list->allocated << 1;\n"
            "            if (size == 0) size = obj->pool_list->allocated;\n"
            "        }\n"
            "        {\n"
            "            pcc_memory_pool_t *const pool = (pcc_memory_pool_t *)PCC_MALLOC(\n"
            "                auxil, sizeof(pcc_memory_pool_t) + obj->element_size * size\n"
            "            );\n"
            "            pool->allocated = size;\n"
            "            pool->unused = size;\n"
            "            pool->next = obj->pool_list;\n"
            "            obj->pool_list = pool;\n"
            "        }\n"
            "    }\n"
            "    obj->pool_list->unused--;\n"
            "    return (char *)(obj->pool_list + 1) + obj->element_size * obj->pool_list->unused;\n"
            "}\n"
            "\n"
            "static void pcc_memory_recycler__recycle(pcc_auxil_t auxil, pcc_memory_recycler_t *obj, void *ptr) {\n"
            "    pcc_memory_entry_t *const tmp = (pcc_memory_entry_t *)ptr;\n"
            "    if (tmp == NULL) return;\n"
            "    tmp->next = obj->entry_list;\n"
            "    obj->entry_list = tmp;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static pcc_thunk_t *pcc_thunk__create_leaf(pcc_context_t *ctx, pcc_action_t action, size_t valuec, size_t captc) {\n"
            "    pcc_thunk_t *const obj = (pcc_thunk_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->thunk_recycler));\n"
            "    obj->type = PCC_THUNK_LEAF;\n"
            "    pcc_value_refer_table__initialize(ctx->auxil, &(obj->data.leaf.values));\n"
            "    pcc_value_refer_table__resize(ctx->auxil, &(obj->data.leaf.values), valuec);\n"
            "    pcc_capture_const_table__initialize(ctx->auxil, &(obj->data.leaf.capts));\n"
            "    pcc_capture_const_table__resize(ctx->auxil, &(obj->data.leaf.capts), captc);\n"
            "    pcc_capture__initialize(ctx->auxil, &(obj->data.leaf.capt0));\n"
            "    obj->data.leaf.action = action;\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static pcc_thunk_t *pcc_thunk__create_node(pcc_context_t *ctx, const pcc_thunk_array_t *thunks, pcc_value_t *value) {\n"
            "    pcc_thunk_t *const obj = (pcc_thunk_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->thunk_recycler));\n"
            "    obj->type = PCC_THUNK_NODE;\n"
            "    obj->data.node.thunks = thunks;\n"
            "    obj->data.node.value = value;\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static void pcc_thunk__destroy(pcc_context_t *ctx, pcc_thunk_t *obj) {\n"
            "    if (obj == NULL) return;\n"
            "    switch (obj->type) {\n"
            "    case PCC_THUNK_LEAF:\n"
            "        pcc_value_refer_table__finalize(ctx->auxil, &(obj->data.leaf.values));\n"
            "        pcc_capture_const_table__finalize(ctx->auxil, &(obj->data.leaf.capts));\n"
            "        pcc_capture__finalize(ctx->auxil, &(obj->data.leaf.capt0));\n"
            "        break;\n"
            "    case PCC_THUNK_NODE:\n"
            "        break;\n"
            "    default: /* unknown */\n"
            "        break;\n"
            "    }\n"
            "    pcc_memory_recycler__recycle(ctx->auxil, &(ctx->thunk_recycler), obj);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_thunk_array__initialize(pcc_context_t *ctx, pcc_thunk_array_t *obj) {\n"
            "    obj->m = PCC_THUNK_ARRAY_MIN_SIZE;\n"
            "    obj->n = 0;\n"
            "    obj->p = obj->a;\n"
            "}\n"
            "\n"
            "static void pcc_thunk_array__finalize(pcc_context_t *ctx, pcc_thunk_array_t *obj) {\n"
            "    while (obj->n > 0) {\n"
            "        obj->n--;\n"
            "        pcc_thunk__destroy(ctx, obj->p[obj->n]);\n"
            "    }\n"
            "    if (obj->p != obj->a) PCC_FREE(ctx->auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_thunk_array__add(pcc_context_t *ctx, pcc_thunk_array_t *obj, pcc_thunk_t *thunk) {\n"
            "    if (obj->m <= obj->n) {\n"
            "        const size_t n = obj->n + 1;\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_THUNK_ARRAY_MIN_SIZE << 1;\n"
            "        while (m < n && m != 0) m <<= 1;\n"
            "        if (m == 0) m = n;\n"
            "        if (obj->p == obj->a) {\n"
            "            size_t i;\n"
            "            obj->p = (pcc_thunk_t **)PCC_MALLOC(ctx->auxil, sizeof(pcc_thunk_t *) * m);\n"
            "            for (i = 0; i < PCC_THUNK_ARRAY_MIN_SIZE; i++) obj->p[i] = obj->a[i];\n"
            "        }\n"
            "        else {\n"
            "            obj->p = (pcc_thunk_t **)PCC_REALLOC(ctx->auxil, obj->p, sizeof(pcc_thunk_t *) * m);\n"
            "        }\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->p[obj->n++] = thunk;\n"
            "}\n"
            "\n"
            "static void pcc_thunk_array__revert(pcc_context_t *ctx, pcc_thunk_array_t *obj, size_t len) {\n"
            "    while (obj->n > len) {\n"
            "        obj->n--;\n"
            "        pcc_thunk__destroy(ctx, obj->p[obj->n]);\n"
            "    }\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static pcc_thunk_chunk_t *pcc_thunk_chunk__create(pcc_context_t *ctx) {\n"
            "    pcc_thunk_chunk_t *const obj = (pcc_thunk_chunk_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->thunk_chunk_recycler));\n"
            "    pcc_value_table__initialize(ctx->auxil, &(obj->values));\n"
            "    pcc_capture_table__initialize(ctx->auxil, &(obj->capts));\n"
            "    pcc_thunk_array__initialize(ctx, &(obj->thunks));\n"
            "    obj->pos = 0;\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static void pcc_thunk_chunk__destroy(pcc_context_t *ctx, pcc_thunk_chunk_t *obj) {\n"
            "    if (obj == NULL) return;\n"
            "    pcc_value_table__finalize(ctx->auxil, &(obj->values));\n"
            "    pcc_capture_table__finalize(ctx->auxil, &(obj->capts));\n"
            "    pcc_thunk_array__finalize(ctx, &(obj->thunks));\n"
            "    pcc_memory_recycler__recycle(ctx->auxil, &(ctx->thunk_chunk_recycler), obj);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_rule_set__initialize(pcc_auxil_t auxil, pcc_rule_set_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_rule_set__finalize(pcc_auxil_t auxil, pcc_rule_set_t *obj) {\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_rule_set__clear(pcc_auxil_t auxil, pcc_rule_set_t *obj) {\n"
            "    obj->n = 0;\n"
            "}\n"
            "\n"
            "static size_t pcc_rule_set__index(pcc_auxil_t auxil, const pcc_rule_set_t *obj, pcc_rule_t rule) {\n"
            "    size_t i;\n"
            "    for (i = 0; i < obj->n; i++) {\n"
            "        if (obj->p[i] == rule) return i;\n"
            "    }\n"
            "    return PCC_VOID_VALUE;\n"
            "}\n"
            "\n"
            "static pcc_bool_t pcc_rule_set__add(pcc_auxil_t auxil, pcc_rule_set_t *obj, pcc_rule_t rule) {\n"
            "    const size_t i = pcc_rule_set__index(auxil, obj, rule);\n"
            "    if (i != PCC_VOID_VALUE) return PCC_FALSE;\n"
            "    if (obj->m <= obj->n) {\n"
            "        const size_t n = obj->n + 1;\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < n && m != 0) m <<= 1;\n"
            "        if (m == 0) m = n;\n"
            "        obj->p = (pcc_rule_t *)PCC_REALLOC(auxil, obj->p, sizeof(pcc_rule_t) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->p[obj->n++] = rule;\n"
            "    return PCC_TRUE;\n"
            "}\n"
            "\n"
            "static pcc_bool_t pcc_rule_set__remove(pcc_auxil_t auxil, pcc_rule_set_t *obj, pcc_rule_t rule) {\n"
            "    const size_t i = pcc_rule_set__index(auxil, obj, rule);\n"
            "    if (i == PCC_VOID_VALUE) return PCC_FALSE;\n"
            "    memmove(obj->p + i, obj->p + (i + 1), sizeof(pcc_rule_t) * (obj->n - (i + 1)));\n"
            "    return PCC_TRUE;\n"
            "}\n"
            "\n"
            "static void pcc_rule_set__copy(pcc_auxil_t auxil, pcc_rule_set_t *obj, const pcc_rule_set_t *src) {\n"
            "    size_t i;\n"
            "    if (obj == src) return;\n"
            "    pcc_rule_set__clear(auxil, obj);\n"
            "    for (i = 0; i < src->n; i++) {\n"
            "        pcc_rule_set__add(auxil, obj, src->p[i]);\n"
            "    }\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static pcc_lr_head_t *pcc_lr_head__create(pcc_context_t *ctx, pcc_rule_t rule) {\n"
            "    pcc_lr_head_t *const obj = (pcc_lr_head_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->lr_head_recycler));\n"
            "    obj->rule = rule;\n"
            "    pcc_rule_set__initialize(ctx->auxil, &(obj->invol));\n"
            "    pcc_rule_set__initialize(ctx->auxil, &(obj->eval));\n"
            "    obj->hold = NULL;\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static void pcc_lr_head__destroy(pcc_context_t *ctx, pcc_lr_head_t *obj) {\n"
            "    if (obj == NULL) return;\n"
            "    pcc_rule_set__finalize(ctx->auxil, &(obj->invol));\n"
            "    pcc_rule_set__finalize(ctx->auxil, &(obj->eval));\n"
            "    pcc_lr_head__destroy(ctx, obj->hold);\n"
            "    pcc_memory_recycler__recycle(ctx->auxil, &(ctx->lr_head_recycler), obj);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_lr_entry__destroy(pcc_context_t *ctx, pcc_lr_entry_t *lr);\n"
            "\n"
            "static pcc_lr_answer_t *pcc_lr_answer__create(pcc_context_t *ctx, pcc_lr_answer_type_t type, size_t pos) {\n"
            "    pcc_lr_answer_t *obj = (pcc_lr_answer_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->lr_answer_recycler));\n"
            "    obj->type = type;\n"
            "    obj->pos = pos;\n"
            "    obj->hold = NULL;\n"
            "    switch (obj->type) {\n"
            "    case PCC_LR_ANSWER_LR:\n"
            "        obj->data.lr = NULL;\n"
            "        break;\n"
            "    case PCC_LR_ANSWER_CHUNK:\n"
            "        obj->data.chunk = NULL;\n"
            "        break;\n"
            "    default: /* unknown */\n"
            "        pcc_memory_recycler__recycle(ctx->auxil, &(ctx->lr_answer_recycler), obj);\n"
            "        obj = NULL;\n"
            "    }\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static void pcc_lr_answer__destroy(pcc_context_t *ctx, pcc_lr_answer_t *obj) {\n"
            "    while (obj != NULL) {\n"
            "        pcc_lr_answer_t *const a = obj->hold;\n"
            "        switch (obj->type) {\n"
            "        case PCC_LR_ANSWER_LR:\n"
            "            pcc_lr_entry__destroy(ctx, obj->data.lr);\n"
            "            break;\n"
            "        case PCC_LR_ANSWER_CHUNK:\n"
            "            pcc_thunk_chunk__destroy(ctx, obj->data.chunk);\n"
            "            break;\n"
            "        default: /* unknown */\n"
            "            break;\n"
            "        }\n"
            "        pcc_memory_recycler__recycle(ctx->auxil, &(ctx->lr_answer_recycler), obj);\n"
            "        obj = a;\n"
            "    }\n"
            "}\n"
            "\n"
            "static void pcc_lr_answer__set_chunk(pcc_context_t *ctx, pcc_lr_answer_t *obj, pcc_thunk_chunk_t *chunk) {\n"
            "    pcc_lr_answer_t *const a = pcc_lr_answer__create(ctx, obj->type, obj->pos);\n"
            "    switch (obj->type) {\n"
            "    case PCC_LR_ANSWER_LR:\n"
            "        a->data.lr = obj->data.lr;\n"
            "        break;\n"
            "    case PCC_LR_ANSWER_CHUNK:\n"
            "        a->data.chunk = obj->data.chunk;\n"
            "        break;\n"
            "    default: /* unknown */\n"
            "        break;\n"
            "    }\n"
            "    a->hold = obj->hold;\n"
            "    obj->hold = a;\n"
            "    obj->type = PCC_LR_ANSWER_CHUNK;\n"
            "    obj->data.chunk = chunk;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_lr_memo_map__initialize(pcc_context_t *ctx, pcc_lr_memo_map_t *obj) {\n"
            "    obj->m = PCC_LR_MEMO_MAP_MIN_SIZE;\n"
            "    obj->n = 0;\n"
            "    obj->p = obj->a;\n"
            "}\n"
            "\n"
            "static void pcc_lr_memo_map__finalize(pcc_context_t *ctx, pcc_lr_memo_map_t *obj) {\n"
            "    while (obj->n > 0) {\n"
            "        obj->n--;\n"
            "        pcc_lr_answer__destroy(ctx, obj->p[obj->n].answer);\n"
            "    }\n"
            "    if (obj->p != obj->a) PCC_FREE(ctx->auxil, obj->p);\n"
            "}\n"
            "\n"
            "static size_t pcc_lr_memo_map__index(pcc_context_t *ctx, pcc_lr_memo_map_t *obj, pcc_rule_t rule) {\n"
            "    size_t i;\n"
            "    for (i = 0; i < obj->n; i++) {\n"
            "        if (obj->p[i].rule == rule) return i;\n"
            "    }\n"
            "    return PCC_VOID_VALUE;\n"
            "}\n"
            "\n"
            "static void pcc_lr_memo_map__put(pcc_context_t *ctx, pcc_lr_memo_map_t *obj, pcc_rule_t rule, pcc_lr_answer_t *answer) {\n"
            "    const size_t i = pcc_lr_memo_map__index(ctx, obj, rule);\n"
            "    if (i != PCC_VOID_VALUE) {\n"
            "        pcc_lr_answer__destroy(ctx, obj->p[i].answer);\n"
            "        obj->p[i].answer = answer;\n"
            "    }\n"
            "    else {\n"
            "        if (obj->m <= obj->n) {\n"
            "            const size_t n = obj->n + 1;\n"
            "            size_t m = obj->m;\n"
            "            if (m == 0) m = PCC_LR_MEMO_MAP_MIN_SIZE << 1;\n"
            "            while (m < n && m != 0) m <<= 1;\n"
            "            if (m == 0) m = n;\n"
            "            if (obj->p == obj->a) {\n"
            "                size_t i;\n"
            "                obj->p = (pcc_lr_memo_t *)PCC_MALLOC(ctx->auxil, sizeof(pcc_lr_memo_t) * m);\n"
            "                for (i = 0; i < PCC_LR_MEMO_MAP_MIN_SIZE; i++) obj->p[i] = obj->a[i];\n"
            "            }\n"
            "            else {\n"
            "                obj->p = (pcc_lr_memo_t *)PCC_REALLOC(ctx->auxil, obj->p, sizeof(pcc_lr_memo_t) * m);\n"
            "            }\n"
            "            obj->m = m;\n"
            "        }\n"
            "        obj->p[obj->n].rule = rule;\n"
            "        obj->p[obj->n].answer = answer;\n"
            "        obj->n++;\n"
            "    }\n"
            "}\n"
            "\n"
            "static pcc_lr_answer_t *pcc_lr_memo_map__get(pcc_context_t *ctx, pcc_lr_memo_map_t *obj, pcc_rule_t rule) {\n"
            "    const size_t i = pcc_lr_memo_map__index(ctx, obj, rule);\n"
            "    return (i != PCC_VOID_VALUE) ? obj->p[i].answer : NULL;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static pcc_lr_table_entry_t *pcc_lr_table_entry__create(pcc_context_t *ctx) {\n"
            "    pcc_lr_table_entry_t *const obj = (pcc_lr_table_entry_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->lr_table_entry_recycler));\n"
            "    obj->head = NULL;\n"
            "    pcc_lr_memo_map__initialize(ctx, &(obj->memos));\n"
            "    obj->hold_a = NULL;\n"
            "    obj->hold_h = NULL;\n"
            "    return obj;\n"
            "}\n"
            "\n"
            "static void pcc_lr_table_entry__destroy(pcc_context_t *ctx, pcc_lr_table_entry_t *obj) {\n"
            "    if (obj == NULL) return;\n"
            "    pcc_lr_memo_map__finalize(ctx, &(obj->memos));\n"
            "    pcc_lr_answer__destroy(ctx, obj->hold_a);\n"
            "    pcc_lr_head__destroy(ctx, obj->hold_h);\n"
            "    pcc_memory_recycler__recycle(ctx->auxil, &(ctx->lr_table_entry_recycler), obj);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_lr_table__initialize(pcc_context_t *ctx, pcc_lr_table_t *obj) {\n"
            "    obj->o = 0;\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__finalize(pcc_context_t *ctx, pcc_lr_table_t *obj) {\n"
            "    while (obj->n > obj->o) {\n"
            "        obj->n--;\n"
            "        pcc_lr_table_entry__destroy(ctx, obj->p[obj->n]);\n"
            "    }\n"
            "    PCC_FREE(ctx->auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__resize(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t len) {\n"
            "    size_t i;\n"
            "    for (i = len; i < obj->n; i++) pcc_lr_table_entry__destroy(ctx, obj->p[i]);\n"
            "    if (obj->m < len) {\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < len && m != 0) m <<= 1;\n"
            "        if (m == 0) m = len;\n"
            "        obj->p = (pcc_lr_table_entry_t **)PCC_REALLOC(ctx->auxil, obj->p, sizeof(pcc_lr_table_entry_t *) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    for (i = obj->n; i < len; i++) obj->p[i] = NULL;\n"
            "    obj->n = len;\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__set_head(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index, pcc_lr_head_t *head) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n) pcc_lr_table__resize(ctx, obj, index + 1);\n"
            "    if (obj->p[index] == NULL) obj->p[index] = pcc_lr_table_entry__create(ctx);\n"
            "    obj->p[index]->head = head;\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__hold_head(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index, pcc_lr_head_t *head) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n) pcc_lr_table__resize(ctx, obj, index + 1);\n"
            "    if (obj->p[index] == NULL) obj->p[index] = pcc_lr_table_entry__create(ctx);\n"
            "    head->hold = obj->p[index]->hold_h;\n"
            "    obj->p[index]->hold_h = head;\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__set_answer(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index, pcc_rule_t rule, pcc_lr_answer_t *answer) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n) pcc_lr_table__resize(ctx, obj, index + 1);\n"
            "    if (obj->p[index] == NULL) obj->p[index] = pcc_lr_table_entry__create(ctx);\n"
            "    pcc_lr_memo_map__put(ctx, &(obj->p[index]->memos), rule, answer);\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__hold_answer(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index, pcc_lr_answer_t *answer) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n) pcc_lr_table__resize(ctx, obj, index + 1);\n"
            "    if (obj->p[index] == NULL) obj->p[index] = pcc_lr_table_entry__create(ctx);\n"
            "    answer->hold = obj->p[index]->hold_a;\n"
            "    obj->p[index]->hold_a = answer;\n"
            "}\n"
            "\n"
            "static pcc_lr_head_t *pcc_lr_table__get_head(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n || obj->p[index] == NULL) return NULL;\n"
            "    return obj->p[index]->head;\n"
            "}\n"
            "\n"
            "static pcc_lr_answer_t *pcc_lr_table__get_answer(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t index, pcc_rule_t rule) {\n"
            "    index += obj->o;\n"
            "    if (index >= obj->n || obj->p[index] == NULL) return NULL;\n"
            "    return pcc_lr_memo_map__get(ctx, &(obj->p[index]->memos), rule);\n"
            "}\n"
            "\n"
            "static void pcc_lr_table__shift(pcc_context_t *ctx, pcc_lr_table_t *obj, size_t count) {\n"
            "    size_t i;\n"
            "    if (count > obj->n - obj->o) count = obj->n - obj->o;\n"
            "    for (i = 0; i < count; i++) pcc_lr_table_entry__destroy(ctx, obj->p[obj->o++]);\n"
            "    if (obj->o > (obj->m >> 1)) {\n"
            "        memmove(obj->p, obj->p + obj->o, sizeof(pcc_lr_table_entry_t *) * (obj->n - obj->o));\n"
            "        obj->n -= obj->o;\n"
            "        obj->o = 0;\n"
            "    }\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static pcc_lr_entry_t *pcc_lr_entry__create(pcc_context_t *ctx, pcc_rule_t rule) {\n"
            "    pcc_lr_entry_t *const lr = (pcc_lr_entry_t *)pcc_memory_recycler__supply(ctx->auxil, &(ctx->lr_entry_recycler));\n"
            "    lr->rule = rule;\n"
            "    lr->seed = NULL;\n"
            "    lr->head = NULL;\n"
            "    return lr;\n"
            "}\n"
            "\n"
            "static void pcc_lr_entry__destroy(pcc_context_t *ctx, pcc_lr_entry_t *lr) {\n"
            "    pcc_memory_recycler__recycle(ctx->auxil, &(ctx->lr_entry_recycler), lr);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_lr_stack__initialize(pcc_auxil_t auxil, pcc_lr_stack_t *obj) {\n"
            "    obj->m = 0;\n"
            "    obj->n = 0;\n"
            "    obj->p = NULL;\n"
            "}\n"
            "\n"
            "static void pcc_lr_stack__finalize(pcc_auxil_t auxil, pcc_lr_stack_t *obj) {\n"
            "    PCC_FREE(auxil, obj->p);\n"
            "}\n"
            "\n"
            "static void pcc_lr_stack__push(pcc_auxil_t auxil, pcc_lr_stack_t *obj, pcc_lr_entry_t *lr) {\n"
            "    if (obj->m <= obj->n) {\n"
            "        const size_t n = obj->n + 1;\n"
            "        size_t m = obj->m;\n"
            "        if (m == 0) m = PCC_ARRAY_MIN_SIZE;\n"
            "        while (m < n && m != 0) m <<= 1;\n"
            "        if (m == 0) m = n;\n"
            "        obj->p = (pcc_lr_entry_t **)PCC_REALLOC(auxil, obj->p, sizeof(pcc_lr_entry_t *) * m);\n"
            "        obj->m = m;\n"
            "    }\n"
            "    obj->p[obj->n++] = lr;\n"
            "}\n"
            "\n"
            "static pcc_lr_entry_t *pcc_lr_stack__pop(pcc_auxil_t auxil, pcc_lr_stack_t *obj) {\n"
            "    return obj->p[--obj->n];\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static pcc_context_t *pcc_context__create(pcc_auxil_t auxil) {\n"
            "    pcc_context_t *const ctx = (pcc_context_t *)PCC_MALLOC(auxil, sizeof(pcc_context_t));\n"
            "    ctx->pos = 0;\n"
            "    ctx->cur = 0;\n"
            "    ctx->level = 0;\n"
            "    pcc_char_array__initialize(auxil, &(ctx->buffer));\n"
            "    pcc_lr_table__initialize(ctx, &(ctx->lrtable));\n"
            "    pcc_lr_stack__initialize(auxil, &(ctx->lrstack));\n"
            "    pcc_thunk_array__initialize(ctx, &(ctx->thunks));\n"
            "    pcc_capture__initialize(ctx->auxil, &(ctx->capt0));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->thunk_recycler), sizeof(pcc_thunk_t));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->thunk_chunk_recycler), sizeof(pcc_thunk_chunk_t));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->lr_head_recycler), sizeof(pcc_lr_head_t));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->lr_answer_recycler), sizeof(pcc_lr_answer_t));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->lr_table_entry_recycler), sizeof(pcc_lr_table_entry_t));\n"
            "    pcc_memory_recycler__initialize(auxil, &(ctx->lr_entry_recycler), sizeof(pcc_lr_entry_t));\n"
            "    ctx->auxil = auxil;\n"
            "    return ctx;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static void pcc_context__destroy(pcc_context_t *ctx) {\n"
            "    if (ctx == NULL) return;\n"
            "    pcc_char_array__finalize(ctx->auxil, &(ctx->buffer));\n"
            "    pcc_lr_table__finalize(ctx, &(ctx->lrtable));\n"
            "    pcc_lr_stack__finalize(ctx->auxil, &(ctx->lrstack));\n"
            "    pcc_thunk_array__finalize(ctx, &(ctx->thunks));\n"
            "    pcc_capture__finalize(ctx->auxil, &(ctx->capt0));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->thunk_recycler));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->thunk_chunk_recycler));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->lr_head_recycler));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->lr_answer_recycler));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->lr_table_entry_recycler));\n"
            "    pcc_memory_recycler__finalize(ctx->auxil, &(ctx->lr_entry_recycler));\n"
            "    PCC_FREE(ctx->auxil, ctx);\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "static size_t pcc_refill_buffer(pcc_context_t *ctx, size_t num) {\n"
            "    if (ctx->buffer.n >= ctx->cur + num) return ctx->buffer.n - ctx->cur;\n"
            "    while (ctx->buffer.n < ctx->cur + num) {\n"
            "        const int c = PCC_GETCHAR(ctx->auxil);\n"
            "        if (c < 0) break;\n"
            "        pcc_char_array__add(ctx->auxil, &(ctx->buffer), (char)c);\n"
            "    }\n"
            "    return ctx->buffer.n - ctx->cur;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static void pcc_commit_buffer(pcc_context_t *ctx) {\n"
            "    memmove(ctx->buffer.p, ctx->buffer.p + ctx->cur, ctx->buffer.n - ctx->cur);\n"
            "    ctx->buffer.n -= ctx->cur;\n"
            "    ctx->pos += ctx->cur;\n"
            "    pcc_lr_table__shift(ctx, &(ctx->lrtable), ctx->cur);\n"
            "    ctx->cur = 0;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static const char *pcc_get_capture_string(pcc_context_t *ctx, const pcc_capture_t *capt) {\n"
            "    if (capt->string.n == 0) {\n"
            "        const size_t n = capt->range.end - capt->range.start;\n"
            "        pcc_capture_t *const p = (pcc_capture_t *)capt;\n"
            "        pcc_char_array__resize(ctx->auxil, &(p->string), n + 1);\n"
            "        if (n > 0) memcpy(p->string.p, ctx->buffer.p + capt->range.start, n);\n"
            "        p->string.p[n] = '\\0';\n"
            "    }\n"
            "    return capt->string.p;\n"
            "}\n"
            "\n"
        );
        if (ctx->flags & CODE_FLAG__UTF8_CHARCLASS_USED) {
            stream__puts(
                &sstream,
                "static size_t pcc_get_char_as_utf32(pcc_context_t *ctx, int *out) { /* with checking UTF-8 validity */\n"
                "    int c, u;\n"
                "    size_t n;\n"
                "    if (pcc_refill_buffer(ctx, 1) < 1) return 0;\n"
                "    c = (int)(unsigned char)ctx->buffer.p[ctx->cur];\n"
                "    n = (c < 0x80) ? 1 :\n"
                "        ((c & 0xe0) == 0xc0) ? 2 :\n"
                "        ((c & 0xf0) == 0xe0) ? 3 :\n"
                "        ((c & 0xf8) == 0xf0) ? 4 : 0;\n"
                "    if (n < 1) return 0;\n"
                "    if (pcc_refill_buffer(ctx, n) < n) return 0;\n"
                "    switch (n) {\n"
                "    case 1:\n"
                "        u = c;\n"
                "        break;\n"
                "    case 2:\n"
                "        u = c & 0x1f;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 1];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        if (u < 0x80) return 0;\n"
                "        break;\n"
                "    case 3:\n"
                "        u = c & 0x0f;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 1];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 2];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        if (u < 0x800) return 0;\n"
                "        break;\n"
                "    case 4:\n"
                "        u = c & 0x07;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 1];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 2];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        c = (int)(unsigned char)ctx->buffer.p[ctx->cur + 3];\n"
                "        if ((c & 0xc0) != 0x80) return 0;\n"
                "        u <<= 6; u |= c & 0x3f;\n"
                "        if (u < 0x10000 || u > 0x10ffff) return 0;\n"
                "        break;\n"
                "    default:\n"
                "        return 0;\n"
                "    }\n"
                "    if (out) *out = u;\n"
                "    return n;\n"
                "}\n"
                "\n"
            );
        }
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static pcc_bool_t pcc_apply_rule(pcc_context_t *ctx, pcc_rule_t rule, pcc_thunk_array_t *thunks, pcc_value_t *value) {\n"
            "    static pcc_value_t null;\n"
            "    pcc_thunk_chunk_t *c = NULL;\n"
            "    const size_t p = ctx->pos + ctx->cur;\n"
            "    pcc_bool_t b = PCC_TRUE;\n"
            "    pcc_lr_answer_t *a = pcc_lr_table__get_answer(ctx, &(ctx->lrtable), p, rule);\n"
            "    pcc_lr_head_t *h = pcc_lr_table__get_head(ctx, &(ctx->lrtable), p);\n"
            "    if (h != NULL) {\n"
            "        if (a == NULL && rule != h->rule && pcc_rule_set__index(ctx->auxil, &(h->invol), rule) == PCC_VOID_VALUE) {\n"
            "            b = PCC_FALSE;\n"
            "            c = NULL;\n"
            "        }\n"
            "        else if (pcc_rule_set__remove(ctx->auxil, &(h->eval), rule)) {\n"
            "            b = PCC_FALSE;\n"
            "            c = rule(ctx);\n"
            "            a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_CHUNK, ctx->pos + ctx->cur);\n"
            "            a->data.chunk = c;\n"
            "            pcc_lr_table__hold_answer(ctx, &(ctx->lrtable), p, a);\n"
            "        }\n"
            "    }\n"
            "    if (b) {\n"
            "        if (a != NULL) {\n"
            "            ctx->cur = a->pos - ctx->pos;\n"
            "            switch (a->type) {\n"
            "            case PCC_LR_ANSWER_LR:\n"
            "                if (a->data.lr->head == NULL) {\n"
            "                    a->data.lr->head = pcc_lr_head__create(ctx, rule);\n"
            "                    pcc_lr_table__hold_head(ctx, &(ctx->lrtable), p, a->data.lr->head);\n"
            "                }\n"
            "                {\n"
            "                    size_t i = ctx->lrstack.n;\n"
            "                    while (i > 0) {\n"
            "                        i--;\n"
            "                        if (ctx->lrstack.p[i]->head == a->data.lr->head) break;\n"
            "                        ctx->lrstack.p[i]->head = a->data.lr->head;\n"
            "                        pcc_rule_set__add(ctx->auxil, &(a->data.lr->head->invol), ctx->lrstack.p[i]->rule);\n"
            "                    }\n"
            "                }\n"
            "                c = a->data.lr->seed;\n"
            "                break;\n"
            "            case PCC_LR_ANSWER_CHUNK:\n"
            "                c = a->data.chunk;\n"
            "                break;\n"
            "            default: /* unknown */\n"
            "                break;\n"
            "            }\n"
            "        }\n"
            "        else {\n"
            "            pcc_lr_entry_t *const e = pcc_lr_entry__create(ctx, rule);\n"
            "            pcc_lr_stack__push(ctx->auxil, &(ctx->lrstack), e);\n"
            "            a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_LR, p);\n"
            "            a->data.lr = e;\n"
            "            pcc_lr_table__set_answer(ctx, &(ctx->lrtable), p, rule, a);\n"
            "            c = rule(ctx);\n"
            "            pcc_lr_stack__pop(ctx->auxil, &(ctx->lrstack));\n"
            "            a->pos = ctx->pos + ctx->cur;\n"
            "            if (e->head == NULL) {\n"
            "                pcc_lr_answer__set_chunk(ctx, a, c);\n"
            "            }\n"
            "            else {\n"
            "                e->seed = c;\n"
            "                h = a->data.lr->head;\n"
            "                if (h->rule != rule) {\n"
            "                    c = a->data.lr->seed;\n"
            "                    a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_CHUNK, ctx->pos + ctx->cur);\n"
            "                    a->data.chunk = c;\n"
            "                    pcc_lr_table__hold_answer(ctx, &(ctx->lrtable), p, a);\n"
            "                }\n"
            "                else {\n"
            "                    pcc_lr_answer__set_chunk(ctx, a, a->data.lr->seed);\n"
            "                    if (a->data.chunk == NULL) {\n"
            "                        c = NULL;\n"
            "                    }\n"
            "                    else {\n"
            "                        pcc_lr_table__set_head(ctx, &(ctx->lrtable), p, h);\n"
            "                        for (;;) {\n"
            "                            ctx->cur = p - ctx->pos;\n"
            "                            pcc_rule_set__copy(ctx->auxil, &(h->eval), &(h->invol));\n"
            "                            c = rule(ctx);\n"
            "                            if (c == NULL || ctx->pos + ctx->cur <= a->pos) break;\n"
            "                            pcc_lr_answer__set_chunk(ctx, a, c);\n"
            "                            a->pos = ctx->pos + ctx->cur;\n"
            "                        }\n"
            "                        pcc_thunk_chunk__destroy(ctx, c);\n"
            "                        pcc_lr_table__set_head(ctx, &(ctx->lrtable), p, NULL);\n"
            "                        ctx->cur = a->pos - ctx->pos;\n"
            "                        c = a->data.chunk;\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    if (c == NULL) return PCC_FALSE;\n"
            "    if (value == NULL) value = &null;\n"
            "    memset(value, 0, sizeof(pcc_value_t)); /* in case */\n"
            "    pcc_thunk_array__add(ctx, thunks, pcc_thunk__create_node(ctx, &(c->thunks), value));\n"
            "    return PCC_TRUE;\n"
            "}\n"
            "\n"
        );
        stream__puts(
            &sstream,
            "MARK_FUNC_AS_USED\n"
            "static void pcc_do_action(pcc_context_t *ctx, const pcc_thunk_array_t *thunks, pcc_value_t *value) {\n"
            "    size_t i;\n"
            "    for (i = 0; i < thunks->n; i++) {\n"
            "        pcc_thunk_t *const thunk = thunks->p[i];\n"
            "        switch (thunk->type) {\n"
            "        case PCC_THUNK_LEAF:\n"
            "            thunk->data.leaf.action(ctx, thunk, value);\n"
            "            break;\n"
            "        case PCC_THUNK_NODE:\n"
            "            pcc_do_action(ctx, thunk->data.node.thunks, thunk->data.node.value);\n"
            "            break;\n"
            "        default: /* unknown */\n"
            "            break;\n"
            "        }\n"
            "    }\n"
            "}\n"
            "\n"
        );
        {
            size_t i, j, k;
            for (i = 0; i < ctx->rules.n; i++) {
                const node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
                for (j = 0; j < rule->preds.n; j++) {
                    const code_block_t *const b = &(rule->preds.p[j]->data.progpred.code);
                    const size_t d = rule->preds.p[j]->data.progpred.index;
                    const node_const_array_t *const c = &(rule->preds.p[j]->data.progpred.capts);
                    stream__printf(
                        &sstream,
                        "static void pcc_predicate_%s_" FMT_LU "(%s_context_t *__pcc_ctx, pcc_thunk_chunk_t *__pcc_in, int *__pcc_out) {\n",
                        rule->name, (ulong_t)d, get_prefix(ctx)
                    );
                    stream__puts(
                        &sstream,
                        "#define auxil (__pcc_ctx->auxil)\n"
                        "#define __ (*__pcc_out)\n"
                    );
                    stream__puts(
                        &sstream,
                        "#define _0 pcc_get_capture_string(__pcc_ctx, &(__pcc_ctx->capt0))\n"
                        "#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_ctx->capt0.range.start))\n"
                        "#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_ctx->capt0.range.end))\n"
                    );
                    k = 0;
                    while (k < c->n) {
                        assert(c->p[k]->type == NODE_CAPTURE);
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU " pcc_get_capture_string(__pcc_ctx, &(__pcc_in->capts.p[" FMT_LU "]))\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU "s ((const size_t)(__pcc_ctx->pos + __pcc_in->capts.p[" FMT_LU "].range.start))\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU "e ((const size_t)(__pcc_ctx->pos + __pcc_in->capts.p[" FMT_LU "].range.end))\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        k++;
                    }
                    stream__write_code_block(&sstream, b->text, b->len, 4, b->fpos.path, b->fpos.line, TRUE, &(ctx->subst));
                    k = c->n;
                    while (k > 0) {
                        k--;
                        assert(c->p[k]->type == NODE_CAPTURE);
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "e\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "s\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                    }
                    stream__puts(
                        &sstream,
                        "#undef _0e\n"
                        "#undef _0s\n"
                        "#undef _0\n"
                    );
                    stream__puts(
                        &sstream,
                        "#undef __\n"
                        "#undef auxil\n"
                    );
                    stream__puts(
                        &sstream,
                        "}\n"
                        "\n"
                    );
                }
            }
        }
        {
            size_t i, j, k;
            for (i = 0; i < ctx->rules.n; i++) {
                const node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
                for (j = 0; j < rule->codes.n; j++) {
                    const code_block_t *b;
                    size_t d;
                    const node_const_array_t *v, *c;
                    switch (rule->codes.p[j]->type) {
                    case NODE_ACTION:
                        b = &(rule->codes.p[j]->data.action.code);
                        d = rule->codes.p[j]->data.action.index;
                        v = &(rule->codes.p[j]->data.action.rvars);
                        c = &(rule->codes.p[j]->data.action.capts);
                        break;
                    case NODE_ERROR:
                        b = &(rule->codes.p[j]->data.error.code);
                        d = rule->codes.p[j]->data.error.index;
                        v = &(rule->codes.p[j]->data.error.rvars);
                        c = &(rule->codes.p[j]->data.error.capts);
                        break;
                    default:
                        print_error("Internal error [%d]\n", __LINE__);
                        exit(-1);
                    }
                    stream__printf(
                        &sstream,
                        "static void pcc_action_%s_" FMT_LU "(%s_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {\n",
                        rule->name, (ulong_t)d, get_prefix(ctx)
                    );
                    stream__puts(
                        &sstream,
                        "#define auxil (__pcc_ctx->auxil)\n"
                        "#define __ (*__pcc_out)\n"
                    );
                    k = 0;
                    while (k < v->n) {
                        assert(v->p[k]->type == NODE_REFERENCE);
                        stream__printf(
                            &sstream,
                            "#define %s (*(__pcc_in->data.leaf.values.p[" FMT_LU "]))\n",
                            v->p[k]->data.reference.rvar, (ulong_t)v->p[k]->data.reference.index
                        );
                        k++;
                    }
                    stream__puts(
                        &sstream,
                        "#define _0 pcc_get_capture_string(__pcc_ctx, &(__pcc_in->data.leaf.capt0))\n"
                        "#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))\n"
                        "#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))\n"
                    );
                    k = 0;
                    while (k < c->n) {
                        assert(c->p[k]->type == NODE_CAPTURE);
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU " pcc_get_capture_string(__pcc_ctx, __pcc_in->data.leaf.capts.p[" FMT_LU "])\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU "s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capts.p[" FMT_LU "]->range.start))\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        stream__printf(
                            &sstream,
                            "#define _" FMT_LU "e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capts.p[" FMT_LU "]->range.end))\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1), (ulong_t)c->p[k]->data.capture.index
                        );
                        k++;
                    }
                    stream__write_code_block(&sstream, b->text, b->len, 4, b->fpos.path, b->fpos.line, TRUE, &(ctx->subst));
                    k = c->n;
                    while (k > 0) {
                        k--;
                        assert(c->p[k]->type == NODE_CAPTURE);
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "e\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "s\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                        stream__printf(
                            &sstream,
                            "#undef _" FMT_LU "\n",
                            (ulong_t)(c->p[k]->data.capture.index + 1)
                        );
                    }
                    stream__puts(
                        &sstream,
                        "#undef _0e\n"
                        "#undef _0s\n"
                        "#undef _0\n"
                    );
                    k = v->n;
                    while (k > 0) {
                        k--;
                        assert(v->p[k]->type == NODE_REFERENCE);
                        stream__printf(
                            &sstream,
                            "#undef %s\n",
                            v->p[k]->data.reference.rvar
                        );
                    }
                    stream__puts(
                        &sstream,
                        "#undef __\n"
                        "#undef auxil\n"
                    );
                    stream__puts(
                        &sstream,
                        "}\n"
                        "\n"
                    );
                }
            }
        }
        {
            size_t i;
            for (i = 0; i < ctx->rules.n; i++) {
                const node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
                stream__printf(
                    &sstream,
                    "static pcc_thunk_chunk_t *pcc_evaluate_rule_%s(pcc_context_t *ctx);\n",
                    rule->name
                );
            }
            stream__puts(
                &sstream,
                "\n"
            );
            for (i = 0; i < ctx->rules.n; i++) {
                code_reach_t r;
                generate_t g;
                const node_rule_t *const rule = &(ctx->rules.p[i]->data.rule);
                g.stream = &sstream;
                g.rule = ctx->rules.p[i];
                g.label = 0;
                g.ascii = ctx->opts.ascii;
                stream__printf(
                    &sstream,
                    "static pcc_thunk_chunk_t *pcc_evaluate_rule_%s(pcc_context_t *ctx) {\n",
                    rule->name
                );
                stream__printf(
                    &sstream,
                    "    pcc_thunk_chunk_t *const chunk = pcc_thunk_chunk__create(ctx);\n"
                    "    chunk->pos = ctx->cur;\n"
                    "    PCC_DEBUG(ctx->auxil, PCC_DBG_EVALUATE, \"%s\", ctx->level, chunk->pos, (ctx->buffer.p + chunk->pos), (ctx->buffer.n - chunk->pos));\n"
                    "    ctx->level++;\n",
                    rule->name
                );
                if (rule->capts.n > 0) {
                    stream__printf(
                        &sstream,
                        "    pcc_capture_table__resize(ctx->auxil, &(chunk->capts), " FMT_LU ");\n",
                        (ulong_t)rule->capts.n
                    );
                }
                if (rule->rvars.n > 0) {
                    stream__printf(
                        &sstream,
                        "    pcc_value_table__resize(ctx->auxil, &(chunk->values), " FMT_LU ");\n",
                        (ulong_t)rule->rvars.n
                    );
                    stream__puts(
                        &sstream,
                        "    pcc_value_table__clear(ctx->auxil, &(chunk->values));\n"
                    );
                }
                r = generate_code(&g, rule->expr, 0, 4, FALSE);
                stream__printf(
                    &sstream,
                    "    ctx->level--;\n"
                    "    PCC_DEBUG(ctx->auxil, PCC_DBG_MATCH, \"%s\", ctx->level, chunk->pos, (ctx->buffer.p + chunk->pos), (ctx->cur - chunk->pos));\n"
                    "    return chunk;\n",
                    rule->name
                );
                if (r != CODE_REACH__ALWAYS_SUCCEED) {
                    stream__printf(
                        &sstream,
                        "L0000:;\n"
                        "    ctx->level--;\n"
                        "    PCC_DEBUG(ctx->auxil, PCC_DBG_NOMATCH, \"%s\", ctx->level, chunk->pos, (ctx->buffer.p + chunk->pos), (ctx->cur - chunk->pos));\n"
                        "    pcc_thunk_chunk__destroy(ctx, chunk);\n"
                        "    return NULL;\n",
                        rule->name
                    );
                }
                stream__puts(
                    &sstream,
                    "}\n"
                    "\n"
                );
            }
        }
        stream__printf(
            &sstream,
            "%s_context_t *%s_create(%s%sauxil) {\n",
            get_prefix(ctx), get_prefix(ctx),
            at, ap ? "" : " "
        );
        stream__puts(
            &sstream,
            "    return pcc_context__create(auxil);\n"
            "}\n"
            "\n"
        );
        stream__printf(
            &sstream,
            "int %s_parse(%s_context_t *ctx, %s%s*ret) {\n",
            get_prefix(ctx), get_prefix(ctx),
            vt, vp ? "" : " "
        );
        stream__printf(
            &sstream,
            "    if (pcc_refill_buffer(ctx, 1) < 1) return 0;\n"
        );
        if (ctx->rules.n > 0) {
            stream__printf(
                &sstream,
                "    if (pcc_apply_rule(ctx, pcc_evaluate_rule_%s, &(ctx->thunks), ret))\n",
                ctx->rules.p[0]->data.rule.name
            );
            stream__puts(
                &sstream,
                "        pcc_do_action(ctx, &(ctx->thunks), ret);\n"
                "    else\n"
                "        PCC_ERROR(ctx->auxil);\n"
                "    pcc_commit_buffer(ctx);\n"
            );
        }
        stream__puts(
            &sstream,
            "    pcc_thunk_array__revert(ctx, &(ctx->thunks), 0);\n"
            "    return 1;\n"
            "}\n"
            "\n"
        );
        stream__printf(
            &sstream,
            "void %s_destroy(%s_context_t *ctx) {\n",
            get_prefix(ctx), get_prefix(ctx)
        );
        stream__puts(
            &sstream,
            "    pcc_context__destroy(ctx);\n"
            "}\n"
        );
    }
    {
        stream__puts(
            &hstream,
            "#ifdef __cplusplus\n"
            "extern \"C\" {\n"
            "#endif\n"
            "\n"
        );
        stream__printf(
            &hstream,
            "typedef struct %s_context_tag %s_context_t;\n"
            "\n",
            get_prefix(ctx), get_prefix(ctx)
        );
        stream__printf(
            &hstream,
            "%s_context_t *%s_create(%s%sauxil);\n",
            get_prefix(ctx), get_prefix(ctx),
            at, ap ? "" : " "
        );
        stream__printf(
            &hstream,
            "int %s_parse(%s_context_t *ctx, %s%s*ret);\n",
            get_prefix(ctx), get_prefix(ctx),
            vt, vp ? "" : " "
        );
        stream__printf(
            &hstream,
            "void %s_destroy(%s_context_t *ctx);\n",
            get_prefix(ctx), get_prefix(ctx)
        );
        stream__puts(
            &hstream,
            "\n"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif\n"
        );
        stream__printf(
            &hstream,
            "\n"
            "#endif /* !PCC_INCLUDED_%s */\n",
            ctx->hid
        );
    }
    {
        size_t i;
        for (i = 0; i < ctx->fsource.n; i++) {
            stream__putc(&sstream, '\n');
            stream__write_footer(
                &sstream, ctx->fsource.p[i].text, ctx->fsource.p[i].len,
                ctx->fsource.p[i].fpos.path, ctx->fsource.p[i].fpos.line, &(ctx->subst)
            );
        }
    }
    fclose_e(hstream.file, hstream.path);
    fclose_e(sstream.file, sstream.path);
    if (ctx->errnum) {
        unlink(ctx->hpath);
        unlink(ctx->spath);
        return FALSE;
    }
    return TRUE;
}

static void print_version(FILE *output) {
    fprintf(output, "%s version %s\n", g_cmdname, VERSION);
    fprintf(output, "Copyright (c) 2014, 2019-2024 Arihiro Yoshida. All rights reserved.\n");
}

static void print_usage(FILE *output) {
    fprintf(output, "Usage: %s [OPTIONS] [FILE]\n", g_cmdname);
    fprintf(output, "Generates a packrat parser for C.\n");
    fprintf(output, "\n");
    fprintf(output, "Options:\n");
    fprintf(output, "  -o BASENAME    specify a base name of output source and header files;\n");
    fprintf(output, "                   can be used only once\n");
    fprintf(output, "  -I DIRNAME     specify a directory name to search for import files;\n");
    fprintf(output, "                   can be used as many times as needed to add directories\n");
    fprintf(output, "  -a, --ascii    disable UTF-8 support\n");
    fprintf(output, "  -l, --lines    add #line directives\n");
    fprintf(output, "  -d, --debug    with debug information\n");
    fprintf(output, "  -h, --help     print this help message and exit\n");
    fprintf(output, "  -v, --version  print the version and exit\n");
    fprintf(output, "\n");
    fprintf(output, "Environment Variable:\n");
    fprintf(output, "  %s\n", ENVVAR_IMPORT_PATH);
    fprintf(output, "      specify directory names to search for import files, delimited by '%c'\n", PATH_SEP);
    fprintf(output, "\n");
    fprintf(output, "Full documentation at: <%s>\n", WEBSITE);
}

int main(int argc, char **argv) {
#ifdef _MSC_VER
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif
    g_cmdname = extract_filename(argv[0]);
    {
        const char *ipath = NULL;
        const char *opath = NULL;
        options_t opts = { 0 };
        string_array_t dirs;
        string_array__initialize(&dirs);
        opts.ascii = FALSE;
        opts.lines = FALSE;
        opts.debug = FALSE;
        {
            const char *path = NULL;
            const char *opt_o = NULL;
            bool_t opt_a = FALSE;
            bool_t opt_l = FALSE;
            bool_t opt_d = FALSE;
            bool_t opt_h = FALSE;
            bool_t opt_v = FALSE;
            int i;
            for (i = 1; i < argc; i++) {
                if (argv[i][0] != '-') {
                    break;
                }
                else if (strcmp(argv[i], "--") == 0) {
                    i++; break;
                }
                else if (argv[i][1] == 'I') {
                    const char *const v = (argv[i][2] != '\0') ? argv[i] + 2 : (++i < argc) ?  argv[i] : NULL;
                    if (v == NULL || v[0] == '\0') {
                        print_error("Import directory name missing\n");
                        fprintf(stderr, "\n");
                        print_usage(stderr);
                        exit(1);
                    }
                    string_array__add(&dirs, v, VOID_VALUE);
                }
                else if (argv[i][1] == 'o') {
                    const char *const v = (argv[i][2] != '\0') ? argv[i] + 2 : (++i < argc) ?  argv[i] : NULL;
                    if (v == NULL || v[0] == '\0') {
                        print_error("Output base name missing\n");
                        fprintf(stderr, "\n");
                        print_usage(stderr);
                        exit(1);
                    }
                    if (opt_o != NULL) {
                        print_error("Extra output base name: '%s'\n", v);
                        fprintf(stderr, "\n");
                        print_usage(stderr);
                        exit(1);
                    }
                    opt_o = v;
                }
                else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--ascii") == 0) {
                    opt_a = TRUE;
                }
                else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lines") == 0) {
                    opt_l = TRUE;
                }
                else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
                    opt_d = TRUE;
                }
                else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                    opt_h = TRUE;
                }
                else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                    opt_v = TRUE;
                }
                else {
                    print_error("Invalid option: '%s'\n", argv[i]);
                    fprintf(stderr, "\n");
                    print_usage(stderr);
                    exit(1);
                }
            }
            switch (argc - i) {
            case 0:
                break;
            case 1:
                path = argv[i];
                break;
            default:
                print_error("Extra input file: '%s'\n", argv[i + 1]);
                fprintf(stderr, "\n");
                print_usage(stderr);
                exit(1);
            }
            if (opt_h || opt_v) {
                if (opt_v) print_version(stdout);
                if (opt_v && opt_h) fprintf(stdout, "\n");
                if (opt_h) print_usage(stdout);
                exit(0);
            }
            ipath = (path && path[0]) ? path : NULL;
            opath = (opt_o && opt_o[0]) ? opt_o : NULL;
            opts.ascii = opt_a;
            opts.lines = opt_l;
            opts.debug = opt_d;
        }
        {
            const char *const v = getenv(ENVVAR_IMPORT_PATH);
            if (v) {
                size_t i = 0, h = 0;
                for (;;) {
                    if (v[i] == '\0') {
                        if (i > h) string_array__add(&dirs, v + h, i - h);
                        break;
                    }
                    else if (v[i] == PATH_SEP) {
                        if (i > h) string_array__add(&dirs, v + h, i - h);
                        h = i + 1;
                    }
                    i++;
                }
            }
        }
        {
            char *const s = get_home_directory();
            if (s) {
                char *const t = add_filename(s, IMPORT_DIR_USER);
                string_array__add(&dirs, t, VOID_VALUE);
                free(t);
                free(s);
            }
        }
        {
#ifdef _MSC_VER
            char *const s = get_appdata_directory();
            if (s) {
                char *const t = add_filename(s, IMPORT_DIR_SYSTEM);
                string_array__add(&dirs, t, VOID_VALUE);
                free(t);
                free(s);
            }
#else
            string_array__add(&dirs, IMPORT_DIR_SYSTEM, VOID_VALUE);
#endif
        }
        {
            context_t *const ctx = create_context(ipath, opath, &dirs, &opts);
            const int b = parse(ctx) && generate(ctx);
            destroy_context(ctx);
            if (!b) exit(10);
        }
        string_array__finalize(&dirs);
    }
    return 0;
}
