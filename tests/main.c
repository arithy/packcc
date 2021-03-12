#include <stddef.h>

#include "parser.h"

#define PRINT(X) printf("%s\n", X);

#include "parser.c"

int main(int argc, char **argv) {
    int ret;
    pcc_context_t *ctx = pcc_create(NULL);
    while (pcc_parse(ctx, &ret));
    pcc_destroy(ctx);
    return 0;
}
