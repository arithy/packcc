# Import File of Utility Codes

## Overview

In this directory, currently one import file that provides utility codes is stored.

## Import File

### `code/pcc_ast.peg`

#### Synopsis

An import file that provides codes to make it easier to build an AST (abstract syntax tree).

#### Usage

The usage procedure is shown below.

1. Import `code/pcc_ast.peg` **after the last `%header` section** in the PEG file if any.
   ```
   %import "code/pcc_ast.peg"
   ```
2. Set the designated data types as follows:
   ```
   %value "pcc_ast_node_t *"

   %auxil "pcc_ast_manager_t *"
   ```
3. Create an AST node using either of the following functions in every rule action.
   - `pcc_ast_node_t *pcc_ast_node__create_0(void);`
     + Returns a newly created nullary node.
   - `pcc_ast_node_t *pcc_ast_node__create_1(pcc_ast_node_t *node);`
     + Returns a newly created unary node with one child node specified by the argument `node`.
   - `pcc_ast_node_t *pcc_ast_node__create_2(pcc_ast_node_t *node0, pcc_ast_node_t *node1);`
     + Returns a newly created binary node with two child nodes specified by the argument `node0` and `node1`.
   - `pcc_ast_node_t *pcc_ast_node__create_3(pcc_ast_node_t *node0, pcc_ast_node_t *node1, pcc_ast_node_t *node2);`
     + Returns a newly created ternary node with three child nodes specified by the argument `node0`, `node1`, and `node2`.
   - `pcc_ast_node_t *pcc_ast_node__create_v(void);`
     + Returns a newly created variadic node initially with no child node.
   - `pcc_ast_node_t *pcc_ast_node__add_child(pcc_ast_node_t *obj, pcc_ast_node_t *node);`
     + Adds a child node specified by the argument `node` to the variadic node `obj`.
     + Can be used for `obj` as a variadic node only.

   An example is shown below.
   ```
   rule0 <- l:rule1 '+' r:rule1 { $$ = pcc_ast_node__create_2(l, r); }
   rule1 <- [0-9]+ { $$ = pcc_ast_node__create_0(); }
   ```
4. Call the generated parser API functions as follows:
   ```c
   pcc_ast_manager_t mgr;
   pcc_ast_manager__initialize(&mgr);
   {
       pcc_context_t *ctx = pcc_create(&mgr);
       pcc_ast_node_t *ast = NULL; /* ast: the root node of the AST */
       while (pcc_parse(ctx, &ast)) {
           /* ... do something needed here */
           pcc_ast_node__destroy(ast);
       }
       pcc_destroy(ctx);
   }
   pcc_ast_manager__finalize(&mgr);
   ```
   This code can be executed safely with no memory leak (if "_do something needed here_" does not bring memory leaks).

#### Customization

To build a meaningful AST, customization of the node is needed.
By defining the macro `PCC_AST_NODE_CUSTOM_DATA_DEFINED` in a `%header` section before `%import "code/pcc_ast.peg"`,
the node member variable `custom` whose data type is `pcc_ast_node_custom_data_t` is enabled for storing node custom data.
The concrete usage procedure is shown below.

1. Define the data type of the node custom data in a PEG file.
   ```c
   %header {
   #define PCC_AST_NODE_CUSTOM_DATA_DEFINED /* <-- enables node custom data */

   typedef struct node_custom_data_tag { /* <-- node custom data type */
       /* ... define member variables as needed */
   } pcc_ast_node_custom_data_t;
   }
   ```
   An example is as follows.
   ```c
   %header {
   #define PCC_AST_NODE_CUSTOM_DATA_DEFINED

   typedef struct text_data_tag {
       char *text;
   } pcc_ast_node_custom_data_t;
   }
   ```
   Make sure that this `%header` section is located before `%import "code/pcc_ast.peg"`.
2. Set a node custom data value in every rule action as needed.
   An example is as follows.
   ```c
   rule0 <- l:rule1 '+' r:rule1 { $$ = pcc_ast_node__create_2(l, r); $$->custom.text = strdup("+"); }
   rule1 <- < [0-9]+ > { $$ = pcc_ast_node__create_0(); $$->custom.text = strdup($1); }
   ```
3. Implement the initialization and finalization functions for the node custom data.
   - `void pcc_ast_node_custom_data__initialize(pcc_ast_node_custom_data_t *obj);`
     + Initializes the node custom data `obj`.
   - `void pcc_ast_node_custom_data__finalize(pcc_ast_node_custom_data_t *obj);`
     + Finalizes the node custom data `obj`.

   An example is as follows.
   ```c
   void pcc_ast_node_custom_data__initialize(pcc_ast_node_custom_data_t *obj) {
       obj->text = NULL;
   }

   void pcc_ast_node_custom_data__finalize(pcc_ast_node_custom_data_t *obj) {
       free(obj->text);
   }
   ```

#### Macros

Some macros are prepared to customize the behavior of memory allocation for AST nodes.
The macro definition should be **in `%source` section** in the PEG source.

The following macros are available.

**`PCC_AST_MALLOC(`**_mgr_**`,`**_size_**`)`**

The function macro to allocate a memory block.
The pointer to the instance of `pcc_ast_manager_t` that was passed to the API function `pcc_create()` can be retrieved from the argument _auxil_.
It can be ignored if the instance does not concern memory allocation.
The argument _size_ is the number of bytes to allocate.
This macro must return a pointer to the allocated memory block, or `NULL` if no sufficient memory is available.

The default is defined as `PCC_MALLOC(mgr, size)`, which is used in the generated parser.

**`PCC_AST_REALLOC(`**_mgr_**`,`**_ptr_**`,`**_size_**`)`**

The function macro to reallocate the existing memory block.
The pointer to the instance of `pcc_ast_manager_t` that was passed to the API function `pcc_create()` can be retrieved from the argument _auxil_.
It can be ignored if the instance does not concern memory allocation.
The argument _ptr_ is the pointer to the previously allocated memory block.
The argument _size_ is the new number of bytes to reallocate.
This macro must return a pointer to the reallocated memory block, or `NULL` if no sufficient memory is available.
The contents of the memory block should be left unchanged in any case even if the reallocation fails.

The default is defined as `PCC_REALLOC(mgr, ptr, size)`, which is used in the generated parser.

**`PCC_AST_FREE(`**_mgr_**`,`**_ptr_**`)`**

The function macro to free the existing memory block.
The pointer to the instance of `pcc_ast_manager_t` that was passed to the API function `pcc_create()` can be retrieved from the argument _auxil_.
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

%value "pcc_ast_node_t *"    # <-- must be set

%auxil "pcc_ast_manager_t *" # <-- must be set

%header {
#define PCC_AST_NODE_CUSTOM_DATA_DEFINED /* <-- enables node custom data */

typedef struct text_data_tag { /* <-- node custom data type */
    char *text;
} pcc_ast_node_custom_data_t;
}

%source {
#include <stdio.h>
#include <string.h>
}

statement <- _ e:expression _ EOL { $$ = e; }
           / ( !EOL . )* EOL      { $$ = NULL; }

expression <- e:term { $$ = e; }

term <- l:term _ '+' _ r:factor { $$ = pcc_ast_node__create_2(l, r); $$->custom.text = strdup("+"); }
      / l:term _ '-' _ r:factor { $$ = pcc_ast_node__create_2(l, r); $$->custom.text = strdup("-"); }
      / e:factor                { $$ = e; }

factor <- l:factor _ '*' _ r:unary { $$ = pcc_ast_node__create_2(l, r); $$->custom.text = strdup("*"); }
        / l:factor _ '/' _ r:unary { $$ = pcc_ast_node__create_2(l, r); $$->custom.text = strdup("/"); }
        / e:unary                  { $$ = e; }

unary <- '+' _ e:unary { $$ = pcc_ast_node__create_1(e); $$->custom.text = strdup("+"); }
       / '-' _ e:unary { $$ = pcc_ast_node__create_1(e); $$->custom.text = strdup("-"); }
       / e:primary     { $$ = e; }

primary <- < [0-9]+ >               { $$ = pcc_ast_node__create_0(); $$->custom.text = strdup($1); }
         / '(' _ e:expression _ ')' { $$ = e; }

_      <- [ \t]*
EOL    <- '\n' / '\r\n' / '\r' / ';'

%import "code/pcc_ast.peg"   # <-- provides AST build functions

%%
void pcc_ast_node_custom_data__initialize(pcc_ast_node_custom_data_t *obj) { /* <-- must be implemented when enabling node custom data */
    obj->text = NULL;
}

void pcc_ast_node_custom_data__finalize(pcc_ast_node_custom_data_t *obj) {   /* <-- must be implemented when enabling node custom data */
    free(obj->text);
}

static void dump_ast(const pcc_ast_node_t *obj, int depth) {
    if (obj) {
        switch (obj->type) {
        case PCC_AST_NODE_TYPE_NULLARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "nullary", obj->custom.text);
            break;
        case PCC_AST_NODE_TYPE_UNARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "unary", obj->custom.text);
            dump_ast(obj->data.unary.node, depth + 1);
            break;
        case PCC_AST_NODE_TYPE_BINARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "binary", obj->custom.text);
            dump_ast(obj->data.binary.node[0], depth + 1);
            dump_ast(obj->data.binary.node[1], depth + 1);
            break;
        case PCC_AST_NODE_TYPE_TERNARY:
            printf("%*s%s: \"%s\"\n", 2 * depth, "", "ternary", obj->custom.text);
            dump_ast(obj->data.ternary.node[0], depth + 1);
            dump_ast(obj->data.ternary.node[1], depth + 1);
            dump_ast(obj->data.ternary.node[2], depth + 1);
            break;
        case PCC_AST_NODE_TYPE_VARIADIC:
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
    pcc_ast_manager_t mgr;
    pcc_ast_manager__initialize(&mgr);
    {
        calc_context_t *ctx = calc_create(&mgr);
        pcc_ast_node_t *ast = NULL;
        while (calc_parse(ctx, &ast)) {
            dump_ast(ast, 0);
            pcc_ast_node__destroy(ast);
        }
        calc_destroy(ctx);
    }
    pcc_ast_manager__finalize(&mgr);
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
