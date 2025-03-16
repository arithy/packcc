### `code/pcc_ast.peg` (version 2.3.0)

#### Synopsis

An import file that provides codes to make it easier to build an AST (abstract syntax tree).

#### Usage

The usage procedure is shown below.

1. Import `code/pcc_ast.peg` **after the last `%header` section** in the PEG file if any.
   ```c
   %import "code/pcc_ast.peg"
   ```
2. Set the designated data types as follows:
   ```c
   %value "pcc_ast_node_t *"

   %auxil "pcc_ast_manager_t *"
   ```

   If the prefix is set with `%prefix`, all symbols starting with <code><b><i>pcc</i></b>\_</code> are changed to those with the specified prefix as below.
   ```c
   %prefix "my"

   %value "my_ast_node_t *"

   %auxil "my_ast_manager_t *"
   ```
3. Create an AST node using either of the following functions in every rule action.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_0(void);</code>
     + Returns a newly created nullary node.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_0_int(ptrdiff_t val);</code>
     + Returns a newly created nullary node retaining an integer value.
     + The integer value can be accessed using <code>ptrdiff_t <b><i>pcc</i></b>\_ast_node__get_integer(<b><i>pcc</i></b>\_ast_node_t *node)</code>.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_0_str(const char *str);</code>
     + Returns a newly created nullary node retaining a copy of the specified string.
     + The string can be accessed using <code>const char *<b><i>pcc</i></b>\_ast_node__get_string(<b><i>pcc</i></b>\_ast_node_t *node)</code>.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_1(<b><i>pcc</i></b>\_ast_node_t *node);</code>
     + Returns a newly created unary node with one child node specified by the argument `node`.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_2(<b><i>pcc</i></b>\_ast_node_t *node0, <b><i>pcc</i></b>\_ast_node_t *node1);</code>
     + Returns a newly created binary node with two child nodes specified by the argument `node0` and `node1`.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_3(<b><i>pcc</i></b>\_ast_node_t *node0, <b><i>pcc</i></b>\_ast_node_t *node1, <b><i>pcc</i></b>\_ast_node_t *node2);</code>
     + Returns a newly created ternary node with three child nodes specified by the argument `node0`, `node1`, and `node2`.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_v(void);</code>
     + Returns a newly created variadic node initially with no child node.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__add_child(<b><i>pcc</i></b>\_ast_node_t *obj, <b><i>pcc</i></b>\_ast_node_t *node);</code>
     + Adds a child node specified by the argument `node` to the variadic node `obj`.
     + Can be used for `obj` as a variadic node only.

   As written above, if the prefix is set with `%prefix`, all symbols starting with <code><b><i>pcc</i></b>\_</code> are changed to those with the specified prefix.

   There are the variants of the node creation functions that enable setting a label as an `int` value.
   The label can be used for specifying node kinds in order to make it easier to analyze the AST in the later parsing steps.
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_0_ext(int label);</code>
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_0_ext_str(int label, const char *str);</code>
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_1_ext(int label, <b><i>pcc</i></b>\_ast_node_t *node);</code>
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_2_ext(int label, <b><i>pcc</i></b>\_ast_node_t *node0, <b><i>pcc</i></b>\_ast_node_t *node1);</code>
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_3_ext(int label, <b><i>pcc</i></b>\_ast_node_t *node0, <b><i>pcc</i></b>\_ast_node_t *node1, <b><i>pcc</i></b>\_ast_node_t *node2);</code>
   - <code><b><i>pcc</i></b>\_ast_node_t *<b><i>pcc</i></b>\_ast_node__create_v_ext(int label);</code>

   Every AST node retains the rule pattern matching range in the member variable `range`.
   Namely, `obj->range.start` and `obj->range.end` memorize `$0s` and `$0e` respectively at the time when the node `obj` was created in a rule action.

   A usage example is shown below.
   ```c
   rule0 <- l:rule1 '+' r:rule1 { $$ = my_ast_node__create_2(l, r); }
   rule1 <- [0-9]+ { $$ = my_ast_node__create_0(); }
   ```
4. Call the generated parser API functions as follows:
   ```c
   my_ast_manager_t mgr;
   my_ast_manager__initialize(&mgr);
   {
       my_context_t *ctx = my_create(&mgr);
       my_ast_node_t *ast = NULL; /* ast: the root node of the AST */
       while (my_parse(ctx, &ast)) {
           /* ... do something needed here */
           my_ast_node__destroy(&mgr, ast);
       }
       my_destroy(ctx);
   }
   my_ast_manager__finalize(&mgr);
   ```
   This code can be executed safely with no memory leak (if "_do something needed here_" does not bring memory leaks).

#### Customization

To build a meaningful AST, customization of the node is needed.
By defining the macro <code><b><i>PCC</i></b>\_AST_NODE_CUSTOM_DATA_DEFINED</code> in a `%header` section before `%import "code/pcc_ast.peg"`,
the node member variable `custom` whose data type is <code><b><i>pcc</i></b>\_ast_node_custom_data_t</code> is enabled for storing node custom data.
If the prefix is set with `%prefix`, the macro name <code><b><i>PCC</i></b>\_AST_NODE_CUSTOM_DATA_DEFINED</code> is changed to those with the uppercased prefix as below.
```c
%prefix "my"

%header {
#define MY_AST_NODE_CUSTOM_DATA_DEFINED
...
}
```

The concrete usage procedure is shown below.

1. Define the data type of the node custom data in a PEG file.
   ```c
   %header {
   #define MY_AST_NODE_CUSTOM_DATA_DEFINED /* <-- enables node custom data */

   typedef struct node_custom_data_tag { /* <-- node custom data type */
       /* ... define member variables as needed */
   } my_ast_node_custom_data_t;
   }
   ```
   An example is as follows.
   ```c
   %header {
   #define MY_AST_NODE_CUSTOM_DATA_DEFINED

   typedef struct text_data_tag {
       char *text;
   } my_ast_node_custom_data_t;
   }
   ```
   Make sure that this `%header` section is located before `%import "code/pcc_ast.peg"`.
2. Set a node custom data value in every rule action as needed.
   An example is as follows.
   ```c
   rule0 <- l:rule1 '+' r:rule1 { $$ = my_ast_node__create_2(l, r); $$->custom.text = strdup("+"); }
   rule1 <- < [0-9]+ > { $$ = my_ast_node__create_0(); $$->custom.text = strdup($1); }
   ```
3. Implement the initialization and finalization functions for the node custom data.
   - <code>void <b><i>pcc</i></b>\_ast_node_custom_data__initialize(<b><i>pcc</i></b>\_ast_manager_t *mgr, <b><i>pcc</i></b>\_ast_node_custom_data_t *obj);</code>
     + Initializes the node custom data `obj`.
   - <code>void <b><i>pcc</i></b>\_ast_node_custom_data__finalize(<b><i>pcc</i></b>\_ast_manager_t *mgr, <b><i>pcc</i></b>\_ast_node_custom_data_t *obj);</code>
     + Finalizes the node custom data `obj`.

   An example is as follows.
   ```c
   void my_ast_node_custom_data__initialize(my_ast_manager_t *mgr, my_ast_node_custom_data_t *obj) {
       obj->text = NULL;
   }

   void my_ast_node_custom_data__finalize(my_ast_manager_t *mgr, my_ast_node_custom_data_t *obj) {
       free(obj->text);
   }
   ```

#### Macros

Some macros are prepared to customize the behavior of memory allocation for AST nodes.
The macro definition should be **in `%source` section** in the PEG source.

The following macros are available.
Note that, unlike other symbols, the prefix of these macro names is never changed even when a different prefix is set with `%prefix`.

**`PCC_AST_MALLOC(`**_mgr_**`,`**_size_**`)`**

The function macro to allocate a memory block.
The pointer to the instance of <code><b><i>pcc</i></b>\_ast_manager_t</code> that was passed to the API function <code><b><i>pcc</i></b>\_create()</code> can be retrieved from the argument _auxil_.
It can be ignored if the instance does not concern memory allocation.
The argument _size_ is the number of bytes to allocate.
This macro must return a pointer to the allocated memory block, or `NULL` if no sufficient memory is available.

The default is defined as `PCC_MALLOC(mgr, size)`, which is used in the generated parser.

**`PCC_AST_REALLOC(`**_mgr_**`,`**_ptr_**`,`**_size_**`)`**

The function macro to reallocate the existing memory block.
The pointer to the instance of <code><b><i>pcc</i></b>\_ast_manager_t</code> that was passed to the API function <code><b><i>pcc</i></b>\_create()</code> can be retrieved from the argument _auxil_.
It can be ignored if the instance does not concern memory allocation.
The argument _ptr_ is the pointer to the previously allocated memory block.
The argument _size_ is the new number of bytes to reallocate.
This macro must return a pointer to the reallocated memory block, or `NULL` if no sufficient memory is available.
The contents of the memory block should be left unchanged in any case even if the reallocation fails.

The default is defined as `PCC_REALLOC(mgr, ptr, size)`, which is used in the generated parser.

**`PCC_AST_FREE(`**_mgr_**`,`**_ptr_**`)`**

The function macro to free the existing memory block.
The pointer to the instance of <code><b><i>pcc</i></b>\_ast_manager_t</code> that was passed to the API function <code><b><i>pcc</i></b>\_create()</code> can be retrieved from the argument _auxil_.
It can be ignored if the instance does not concern memory allocation.
The argument _ptr_ is the pointer to the previously allocated memory block.
This macro need not return a value.

The default is defined as `PCC_FREE(mgr, ptr)`, which is used in the generated parser.

**`PCC_AST_NODE_ARRAY_MIN_SIZE`**

The initial size (the number of nods) of the node arrays used in AST nodes.
The arrays are expanded as needed.
The default is `4`.

#### Example

An example which builds an AST and dumps it is shown here.
This example accepts the same inputs as [*Desktop Calculator*](../../examples/calc.peg).

```c
%prefix "calc"

%value "calc_ast_node_t *"    # <-- must be set

%auxil "calc_ast_manager_t *" # <-- must be set

%header {
#define CALC_AST_NODE_CUSTOM_DATA_DEFINED /* <-- enables node custom data */

typedef struct text_data_tag { /* <-- node custom data type */
    char *text;
} calc_ast_node_custom_data_t;
}

%source {
#include <stdio.h>
#include <string.h>
}

statement <- _ e:expression _ EOL { $$ = e; }
           / ( !EOL . )* EOL      { $$ = NULL; }

expression <- e:term { $$ = e; }

term <- l:term _ '+' _ r:factor { $$ = calc_ast_node__create_2(l, r); $$->custom.text = strdup("+"); }
      / l:term _ '-' _ r:factor { $$ = calc_ast_node__create_2(l, r); $$->custom.text = strdup("-"); }
      / e:factor                { $$ = e; }

factor <- l:factor _ '*' _ r:unary { $$ = calc_ast_node__create_2(l, r); $$->custom.text = strdup("*"); }
        / l:factor _ '/' _ r:unary { $$ = calc_ast_node__create_2(l, r); $$->custom.text = strdup("/"); }
        / e:unary                  { $$ = e; }

unary <- '+' _ e:unary { $$ = calc_ast_node__create_1(e); $$->custom.text = strdup("+"); }
       / '-' _ e:unary { $$ = calc_ast_node__create_1(e); $$->custom.text = strdup("-"); }
       / e:primary     { $$ = e; }

primary <- < [0-9]+ >               { $$ = calc_ast_node__create_0(); $$->custom.text = strdup($1); }
         / '(' _ e:expression _ ')' { $$ = e; }

_      <- [ \t]*
EOL    <- '\n' / '\r\n' / '\r' / ';'

%import "code/pcc_ast.peg"   # <-- provides AST build functions

%%
void calc_ast_node_custom_data__initialize(calc_ast_manager_t *mgr, calc_ast_node_custom_data_t *obj) {
    obj->text = NULL;
} /* <-- must be implemented when enabling node custom data */

void calc_ast_node_custom_data__finalize(calc_ast_manager_t *mgr, calc_ast_node_custom_data_t *obj) {
    free(obj->text);
} /* <-- must be implemented when enabling node custom data */

static void dump_ast(const calc_ast_node_t *obj, int depth) {
    if (obj) {
        switch (obj->type) {
        case CALC_AST_NODE_TYPE_NULLARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "nullary", obj->custom.text);
            break;
        case CALC_AST_NODE_TYPE_UNARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "unary", obj->custom.text);
            dump_ast(obj->data.unary.node, depth + 1);
            break;
        case CALC_AST_NODE_TYPE_BINARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "binary", obj->custom.text);
            dump_ast(obj->data.binary.node[0], depth + 1);
            dump_ast(obj->data.binary.node[1], depth + 1);
            break;
        case CALC_AST_NODE_TYPE_TERNARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "ternary", obj->custom.text);
            dump_ast(obj->data.ternary.node[0], depth + 1);
            dump_ast(obj->data.ternary.node[1], depth + 1);
            dump_ast(obj->data.ternary.node[2], depth + 1);
            break;
        case CALC_AST_NODE_TYPE_VARIADIC:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "variadic", obj->custom.text);
            {
                size_t i;
                for (i = 0; i < obj->data.variadic.len; i++) {
                    dump_ast(obj->data.variadic.node[i], depth + 1);
                }
            }
            break;
        default:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "(unknown)", obj->custom.text);
            break;
        }
    }
    else {
        printf("%*s(null)\n", 2 * depth, "");
    }
}

int main(int argc, char **argv) {
    calc_ast_manager_t mgr;
    calc_ast_manager__initialize(&mgr);
    {
        calc_context_t *ctx = calc_create(&mgr);
        calc_ast_node_t *ast = NULL;
        while (calc_parse(ctx, &ast)) {
            dump_ast(ast, 0);
            calc_ast_node__destroy(&mgr, ast);
        }
        calc_destroy(ctx);
    }
    calc_ast_manager__finalize(&mgr);
    return 0;
}
```

An execution example is as follows.

```
$ ./ast-calc↵
1+2*(3+4*(5+6))↵
binary: "+"
  nullary: "1"
  binary: "*"
    nullary: "2"
    binary: "+"
      nullary: "3"
      binary: "*"
        nullary: "4"
        binary: "+"
          nullary: "5"
          nullary: "6"
5*6*7*8/(1*2*3*4)↵
binary: "/"
  binary: "*"
    binary: "*"
      binary: "*"
        nullary: "5"
        nullary: "6"
      nullary: "7"
    nullary: "8"
  binary: "*"
    binary: "*"
      binary: "*"
        nullary: "1"
        nullary: "2"
      nullary: "3"
    nullary: "4"
```
