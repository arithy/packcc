### `util/tab.peg` (version 1.0.0)

#### Synopsis

An import file that provides the following rule:
- The rule to match a space character or a tab character and to update the marker variable retaining a column number.

#### Usage

The usage procedure is shown below.

1. Import `util/tab.peg` **after the last rule** in the PEG file.
   ```c
   %import "util/tab.peg"
   ```
2. Set the tab size if necessary as follows:
   ```c
   %source {
   #define PCC_TAB_SIZE 4
   }
   ```
   In the example above, the tab size is set to 4.
   Without the definition, the default tab size 8 is applied.

   If the prefix is set to `%prefix`, the macro stating with <code><b><i>PCC</i></b>\_</code> is changed to that with the specified prefix as below.
   ```c
   %prefix "my"

   %source {
   #define MY_TAB_SIZE 4
   }
   ```
3. Use the rule `TAB` provided by this import file to match a space character `' '` or a tab character `'\t'`.
   ```c
   rule <- TAB* '\n'
   ```
   In the example above, the rule matches any length including 0 of a sequence containing space and tab characters followed by `'\n'`.
4. Use the marker variable `@tab_col` to measure the column number.
   ```c
   rule <- &{ @tab_col = 0; } TAB* { printf("column: %d\n", (int)@tab_col); } '\n'
   ```
   In the example above, `@tab_col` is set to 0 in advance, and `@tab_col` will have the column number at the position after the part matched by `TAB*`.

**Notice:** Whenever parsing starts, the values of all marker variables are set to 0.

#### Example

An example which shows column numbers of the respective input sequences of space and tab characters followed by `'\n'` is shown here.

```c
%prefix "my"

%source {
#define MY_TAB_SIZE 4
}

rule <- &{ @tab_col = 0; } TAB* { printf("column: %d\n", (int)@tab_col); } '\n'

%import "util/tab.peg"

%%
int main(int argc, char **argv) {
    my_context_t *ctx = my_create(NULL);
    while (my_parse(ctx, NULL));
    my_destroy(ctx);
    return 0;
}
```

Some output results of the generated parser are shown below.
- input: `" "` &rarr; output: `"column: 1\n"`
- input: `"\t"` &rarr; output: `"column: 4\n"`
- input: `" \t"` &rarr; output: `"column: 4\n"`
- input: `"  \t"` &rarr; output: `"column: 4\n"`
- input: `"   \t"` &rarr; output: `"column: 4\n"`
- input: `"    \t"` &rarr; output: `"column: 8\n"`
- input: `"\t  \t "` &rarr; output: `"column: 9\n"`
