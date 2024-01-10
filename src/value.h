#define TREE_HEADER                                                            \
  "#include <stdio.h>\n"                                                       \
  "#include <stdlib.h>\n"                                                      \
  "#include <string.h>\n"                                                      \
  "\n"                                                                         \
  "#define number(x) ({ TREE_VALUE *ret = malloc(sizeof(TREE_VALUE)); "        \
  "ret->type = NUMBER; ret->value = malloc(sizeof(_TREE_VALUE)); "             \
  "ret->value->number = x; ret; })\n"                                          \
  "#define text(x) ({ TREE_VALUE *ret = malloc(sizeof(TREE_VALUE)); "          \
  "ret->type = STRING; ret->value = malloc(sizeof(_TREE_VALUE)); "             \
  "ret->value->_text = x; ret; })\n"                                           \
  "#define make(a, b, c) ({ TREE_VALUE *ret = malloc(sizeof(TREE_VALUE)); "    \
  "ret->type = TREE; ret->value = malloc(sizeof(_TREE_VALUE)); "               \
  "ret->value->tree = tree(a, b, c); ret; })\n"                                \
  "\n"                                                                         \
  "#define CREATE_NULL() text(\"EMPTY\")\n"                                    \
  "#define CREATE_NODE(t, x) make(t, x, CREATE_NULL())\n"                      \
  "\n"                                                                         \
  "typedef char *string;\n"                                                    \
  "union _value;\n"                                                            \
  "enum VALUE_TYPE { NUMBER, STRING, TREE };\n"                                \
  "\n"                                                                         \
  "typedef struct value {\n"                                                   \
  "   enum VALUE_TYPE    type;\n"                                              \
  "   union _value *value;\n"                                                  \
  "} TREE_VALUE;\n"                                                            \
  "\n"                                                                         \
  "typedef struct tree {\n"                                                    \
  "   string type;\n"                                                          \
  "   TREE_VALUE *left;\n"                                                     \
  "   TREE_VALUE *right;\n"                                                    \
  "} TREE_PART;\n"                                                             \
  "\n"                                                                         \
  "typedef union _value {\n"                                                   \
  "   int   number;\n"                                                         \
  "   string _text;\n"                                                         \
  "   TREE_PART *tree;\n"                                                      \
  "} _TREE_VALUE;\n"                                                           \
  "\n"                                                                         \
  "TREE_PART *tree(string type, TREE_VALUE *left, TREE_VALUE *right) {\n"      \
  "   TREE_PART *t = malloc(sizeof(TREE_PART));\n"                             \
  "   t->type = type;\n"                                                       \
  "   t->left = left;\n"                                                       \
  "   t->right = right;\n"                                                     \
  "\n"                                                                         \
  "   return t;\n"                                                             \
  "}\n"                                                                        \
  "\n"                                                                         \
  "void PRINT_INDENT(int i) {\n"                                               \
  "   for(int j = 0; j < i; j++) printf(\"   \");\n"                           \
  "}\n"                                                                        \
  "\n"                                                                         \
  "string VALUE_TYPES[] = { \"number\", \"string\", \"tree\" };\n"             \
  "\n"                                                                         \
  "typedef char byte;\n"                                                       \
  "typedef byte bool;\n"                                                       \
  "\n"                                                                         \
  "bool IS_VALUE_NULL(TREE_VALUE *v) {\n"                                      \
  "   return v->type == STRING && !strcmp(v->value->_text, \"EMPTY\");\n"      \
  "}\n"                                                                        \
  "\n"                                                                         \
  "#define run() ({ PRINT_INDENT(depth); printf(\"%s:\\n\", t->type); })\n"    \
  "void PRINT_TREE_(TREE_PART *t, int depth) {\n"                              \
  "   TREE_VALUE *v = t->left;\n"                                              \
  "\n"                                                                         \
  "   if(!IS_VALUE_NULL(v)) {\n"                                               \
  "       switch(v->type) {\n"                                                 \
  "           case NUMBER:\n"                                                  \
  "               run();\n"                                                    \
  "               PRINT_INDENT(depth + 1);\n"                                  \
  "               printf(\"%d\\n\", v->value->number);\n"                      \
  "               break;\n"                                                    \
  "           case STRING:\n"                                                  \
  "               if(strcmp(t->type, v->value->_text) != 0) { run(); "         \
  "PRINT_INDENT(depth + 1); printf(\"\\\"%s\\\"\\n\", v->value->_text); }\n"   \
  "               else { PRINT_INDENT(depth); printf(\"%s\\n\", "              \
  "v->value->_text); }\n"                                                      \
  "               break;\n"                                                    \
  "           case TREE:\n"                                                    \
  "               run();\n"                                                    \
  "               PRINT_TREE_(v->value->tree, depth + 1);\n"                   \
  "               break;\n"                                                    \
  "       }\n"                                                                 \
  "   }\n"                                                                     \
  "\n"                                                                         \
  "   v = t->right;\n"                                                         \
  "   if(!IS_VALUE_NULL(v)) {\n"                                               \
  "       switch(v->type) {\n"                                                 \
  "           case NUMBER:\n"                                                  \
  "               PRINT_INDENT(depth + 1);\n"                                  \
  "               printf(\"%d\\n\", v->value->number);\n"                      \
  "               break;\n"                                                    \
  "           case STRING:\n"                                                  \
  "               if(strcmp(t->type, v->value->_text) == 0) { "                \
  "PRINT_INDENT(depth); printf(\"\\\"%s\\\"\\n\", v->value->_text); }\n"       \
  "               else printf(\"%s\\n\", v->value->_text);\n"                  \
  "               break;\n"                                                    \
  "           case TREE:\n"                                                    \
  "               PRINT_TREE_(v->value->tree, depth +1);\n"                    \
  "               break;\n"                                                    \
  "       }\n"                                                                 \
  "   }\n"                                                                     \
  "}\n"                                                                        \
  "\n"                                                                         \
  "#define PRINT_TREE(x) PRINT_TREE_(x, 0)\n"                                  \
  "#define PRINT_VALUE(x) PRINT_TREE(tree(\"value\", x, CREATE_NULL()))\n"     \
  "\n"                                                                         \
  "enum { false = 0, true = 1 };\n"                                            \
  "const bool __DEBUG_LOG = false;\n"                                          \
  "#define DEBUG(x) if(__DEBUG_LOG) PRINT_VALUE(x)\n"                          \
  "\n\n"
