
$$
$0
$0s
$0e
$123
$123s
$123e
foo1
FOO1
${bar}

$$
$0
$0s
$0e
$123
$123s
$123e
foo1
FOO1
${bar}

#ifndef PCC_INCLUDED_PARSER_H
#define PCC_INCLUDED_PARSER_H

$$
$0
$0s
$0e
$123
$123s
$123e
foo1
FOO1
${bar}

$$
$0
$0s
$0e
$123
$123s
$123e
foo1
FOO1
${bar}

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foo1_context_tag foo1_context_t;

foo1_context_t *foo1_create(long auxil);
int foo1_parse(foo1_context_t *ctx, double *ret);
void foo1_destroy(foo1_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !PCC_INCLUDED_PARSER_H */
