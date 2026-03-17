### `util/eol.peg` (version 1.0.0)

#### Synopsis

An import file that provides the following rules:
- The rule to match an end-of-line and to update the marker variables retaining a line number and a start position of the current line respectively.
- The rule to match the end-of-file.

#### Usage

The usage procedure is shown below.

1. Import `util/eol.peg` **after the last rule** in the PEG file.
   ```c
   %import "util/eol.peg"
   ```
2. Use the rule `EOL` provided by this import file to match an end-of-line: `'\r\n'`, `'\n'`, or `'\r'`.
   ```c
   line <- ( word / !EOL . )* EOL
   word <- [a-zA-Z0-9_]+
   ```
   In the example above, the rule `line` matches one text line including a blank line.
3. Use the rule `EOF` provided by this import file to match an end-of-file.
   ```c
   file <- line* EOF
   ```
   In the example above, the rule `file` matches any number including 0 of text lines in the file.
4. Use the marker variable `@eol_lineno` and `@eol_col_base` to obtain the current line number and the start byte position of the current line.
   Both are 0-based indexing, i.e. the first line number is 0, and the byte position starts from 0.
   ```c
   %marker @lineno @col_offset

   word
      <- &{
           @lineno = @eol_lineno;
           @col_offset = $0s - @eol_col_base;
       } < [a-zA-Z0-9_]+ >
       {
           printf("line: %d, column: %d, word: '%s'\n", (int)@lineno + 1, (int)@col_offset + 1, $1);
       }
   ```
   In the example above, the line number and the column byte offset where each `word` starts are output.

#### Example

An example which shows the line number and the column byte offset where each `word` starts is shown here.

```c
%prefix "my"

%marker @lineno @col_offset

file <- line* EOF
line <- ( word / !EOL . )* EOL
word
   <- &{
        @lineno = @eol_lineno;
        @col_offset = $0s - @eol_col_base;
    } < [a-zA-Z0-9_]+ >
    {
        printf("line: %d, column: %d, word: '%s'\n", (int)@lineno + 1, (int)@col_offset + 1, $1);
    }

%import "util/eol.peg"

%%
int main(int argc, char **argv) {
    my_context_t *ctx = my_create(NULL);
    my_parse(ctx, NULL);
    my_destroy(ctx);
    return 0;
}
```

If the following text is input to the parser,

```
The meaning of life is
to find your gift.
The purpose of life is
to give it away.
```

the parser will output the following text.

```
line: 1, column: 1, word: 'The'
line: 1, column: 5, word: 'meaning'
line: 1, column: 13, word: 'of'
line: 1, column: 16, word: 'life'
line: 1, column: 21, word: 'is'
line: 2, column: 1, word: 'to'
line: 2, column: 4, word: 'find'
line: 2, column: 9, word: 'your'
line: 2, column: 14, word: 'gift'
line: 3, column: 1, word: 'The'
line: 3, column: 5, word: 'purpose'
line: 3, column: 13, word: 'of'
line: 3, column: 16, word: 'life'
line: 3, column: 21, word: 'is'
line: 4, column: 1, word: 'to'
line: 4, column: 4, word: 'give'
line: 4, column: 9, word: 'it'
line: 4, column: 12, word: 'away'
```
