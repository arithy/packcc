#include "parser.h"

int main(int argc, char **argv) {
    double ret;
    my_context_t *ctx = my_create(0);
    while (my_parse(ctx, &ret));
    my_destroy(ctx);
    return 0;
}
