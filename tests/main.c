#include <stddef.h>

#include "parser.h"

#define PRINT(X) printf("%s\n", X);

#include "parser.c"

void main(void) {
    int ret;
    pcc_context_t *ctx = pcc_create(NULL);
    while (pcc_parse(ctx, &ret));
    pcc_destroy(ctx);
}
