
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

#ifdef _MSC_VER
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _MSC_VER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#if defined __GNUC__ && defined _WIN32 /* MinGW */
#ifndef PCC_USE_SYSTEM_STRNLEN
#define strnlen(str, maxlen) pcc_strnlen(str, maxlen)
static size_t pcc_strnlen(const char *str, size_t maxlen) {
    size_t i;
    for (i = 0; i < maxlen && str[i]; i++);
    return i;
}
#endif /* !PCC_USE_SYSTEM_STRNLEN */
#endif /* defined __GNUC__ && defined _WIN32 */
#endif /* !_MSC_VER */

#include "parser.h"

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

#if !defined __has_attribute || defined _MSC_VER
#define __attribute__(x)
#endif

#ifdef _MSC_VER
#define MARK_FUNC_AS_USED __pragma(warning(suppress:4505))
#else
#define MARK_FUNC_AS_USED __attribute__((__unused__))
#endif

#ifdef _MSC_VER
#define MARK_VAR_AS_USED __pragma(warning(suppress:4189))
#else
#define MARK_VAR_AS_USED __attribute__((__unused__))
#endif

#ifndef PCC_BUFFER_MIN_SIZE
#define PCC_BUFFER_MIN_SIZE 256
#endif /* !PCC_BUFFER_MIN_SIZE */

#ifndef PCC_ARRAY_MIN_SIZE
#define PCC_ARRAY_MIN_SIZE 2
#endif /* !PCC_ARRAY_MIN_SIZE */

#ifndef PCC_POOL_MIN_SIZE
#define PCC_POOL_MIN_SIZE 65536
#endif /* !PCC_POOL_MIN_SIZE */

#define PCC_DBG_EVALUATE 0
#define PCC_DBG_MATCH    1
#define PCC_DBG_NOMATCH  2

#define PCC_VOID_VALUE (~(size_t)0)

typedef enum pcc_bool_tag {
    PCC_FALSE = 0,
    PCC_TRUE
} pcc_bool_t;

typedef struct pcc_char_array_tag {
    char *buf;
    size_t max;
    size_t len;
} pcc_char_array_t;

typedef struct pcc_range_tag {
    size_t start;
    size_t end;
} pcc_range_t;

typedef double pcc_value_t;

typedef long pcc_auxil_t;

typedef foo1_context_t pcc_context_t;

typedef struct pcc_value_table_tag {
    pcc_value_t *buf;
    size_t max;
    size_t len;
} pcc_value_table_t;

typedef struct pcc_value_refer_table_tag {
    pcc_value_t **buf;
    size_t max;
    size_t len;
} pcc_value_refer_table_t;

typedef struct pcc_capture_tag {
    pcc_range_t range;
    char *string; /* mutable */
} pcc_capture_t;

typedef struct pcc_capture_table_tag {
    pcc_capture_t *buf;
    size_t max;
    size_t len;
} pcc_capture_table_t;

typedef struct pcc_capture_const_table_tag {
    const pcc_capture_t **buf;
    size_t max;
    size_t len;
} pcc_capture_const_table_t;

typedef struct pcc_thunk_tag pcc_thunk_t;
typedef struct pcc_thunk_array_tag pcc_thunk_array_t;

typedef void (*pcc_action_t)(pcc_context_t *, pcc_thunk_t *, pcc_value_t *);

typedef enum pcc_thunk_type_tag {
    PCC_THUNK_LEAF,
    PCC_THUNK_NODE
} pcc_thunk_type_t;

typedef struct pcc_thunk_leaf_tag {
    pcc_value_refer_table_t values;
    pcc_capture_const_table_t capts;
    pcc_capture_t capt0;
    pcc_action_t action;
} pcc_thunk_leaf_t;

typedef struct pcc_thunk_node_tag {
    const pcc_thunk_array_t *thunks; /* just a reference */
    pcc_value_t *value; /* just a reference */
} pcc_thunk_node_t;

typedef union pcc_thunk_data_tag {
    pcc_thunk_leaf_t leaf;
    pcc_thunk_node_t node;
} pcc_thunk_data_t;

struct pcc_thunk_tag {
    pcc_thunk_type_t type;
    pcc_thunk_data_t data;
};

struct pcc_thunk_array_tag {
    pcc_thunk_t **buf;
    size_t max;
    size_t len;
};

typedef struct pcc_thunk_chunk_tag {
    pcc_value_table_t values;
    pcc_capture_table_t capts;
    pcc_thunk_array_t thunks;
    size_t pos; /* the starting position in the character buffer */
} pcc_thunk_chunk_t;

typedef struct pcc_lr_entry_tag pcc_lr_entry_t;

typedef enum pcc_lr_answer_type_tag {
    PCC_LR_ANSWER_LR,
    PCC_LR_ANSWER_CHUNK
} pcc_lr_answer_type_t;

typedef union pcc_lr_answer_data_tag {
    pcc_lr_entry_t *lr;
    pcc_thunk_chunk_t *chunk;
} pcc_lr_answer_data_t;

typedef struct pcc_lr_answer_tag pcc_lr_answer_t;

struct pcc_lr_answer_tag {
    pcc_lr_answer_type_t type;
    pcc_lr_answer_data_t data;
    size_t pos; /* the absolute position in the input */
    pcc_lr_answer_t *hold;
};

typedef pcc_thunk_chunk_t *(*pcc_rule_t)(pcc_context_t *);

typedef struct pcc_rule_set_tag {
    pcc_rule_t *buf;
    size_t max;
    size_t len;
} pcc_rule_set_t;

typedef struct pcc_lr_head_tag pcc_lr_head_t;

struct pcc_lr_head_tag {
    pcc_rule_t rule;
    pcc_rule_set_t invol;
    pcc_rule_set_t eval;
    pcc_lr_head_t *hold;
};

typedef struct pcc_lr_memo_tag {
    pcc_rule_t rule;
    pcc_lr_answer_t *answer;
} pcc_lr_memo_t;

typedef struct pcc_lr_memo_map_tag {
    pcc_lr_memo_t *buf;
    size_t max;
    size_t len;
} pcc_lr_memo_map_t;

typedef struct pcc_lr_table_entry_tag {
    pcc_lr_head_t *head; /* just a reference */
    pcc_lr_memo_map_t memos;
    pcc_lr_answer_t *hold_a;
    pcc_lr_head_t *hold_h;
} pcc_lr_table_entry_t;

typedef struct pcc_lr_table_tag {
    pcc_lr_table_entry_t **buf;
    size_t max;
    size_t len;
    size_t ofs;
} pcc_lr_table_t;

struct pcc_lr_entry_tag {
    pcc_rule_t rule;
    pcc_thunk_chunk_t *seed; /* just a reference */
    pcc_lr_head_t *head; /* just a reference */
};

typedef struct pcc_lr_stack_tag {
    pcc_lr_entry_t **buf;
    size_t max;
    size_t len;
} pcc_lr_stack_t;

typedef struct pcc_memory_entry_tag pcc_memory_entry_t;
typedef struct pcc_memory_pool_tag pcc_memory_pool_t;

struct pcc_memory_entry_tag {
    pcc_memory_entry_t *next;
};

struct pcc_memory_pool_tag {
    pcc_memory_pool_t *next;
    size_t allocated;
    size_t unused;
};

typedef struct pcc_memory_recycler_tag {
    pcc_memory_pool_t *pool_list;
    pcc_memory_entry_t *entry_list;
    size_t element_size;
} pcc_memory_recycler_t;

struct foo1_context_tag {
    size_t pos; /* the position in the input of the first character currently buffered */
    size_t cur; /* the current parsing position in the character buffer */
    size_t level;
    pcc_char_array_t buffer;
    pcc_lr_table_t lrtable;
    pcc_lr_stack_t lrstack;
    pcc_thunk_array_t thunks;
    pcc_auxil_t auxil;
    pcc_memory_recycler_t thunk_chunk_recycler;
    pcc_memory_recycler_t lr_head_recycler;
    pcc_memory_recycler_t lr_answer_recycler;
};

#ifndef PCC_ERROR
#define PCC_ERROR(auxil) pcc_error()
MARK_FUNC_AS_USED
static void pcc_error(void) {
    fprintf(stderr, "Syntax error\n");
    exit(1);
}
#endif /* !PCC_ERROR */

#ifndef PCC_GETCHAR
#define PCC_GETCHAR(auxil) getchar()
#endif /* !PCC_GETCHAR */

#ifndef PCC_MALLOC
#define PCC_MALLOC(auxil, size) pcc_malloc_e(size)
static void *pcc_malloc_e(size_t size) {
    void *const p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}
#endif /* !PCC_MALLOC */

#ifndef PCC_REALLOC
#define PCC_REALLOC(auxil, ptr, size) pcc_realloc_e(ptr, size)
static void *pcc_realloc_e(void *ptr, size_t size) {
    void *const p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}
#endif /* !PCC_REALLOC */

#ifndef PCC_FREE
#define PCC_FREE(auxil, ptr) free(ptr)
#endif /* !PCC_FREE */

#ifndef PCC_DEBUG
#define PCC_DEBUG(auxil, event, rule, level, pos, buffer, length) ((void)0)
#endif /* !PCC_DEBUG */

static char *pcc_strndup_e(pcc_auxil_t auxil, const char *str, size_t len) {
    const size_t m = strnlen(str, len);
    char *const s = (char *)PCC_MALLOC(auxil, m + 1);
    memcpy(s, str, m);
    s[m] = '\0';
    return s;
}

static void pcc_char_array__init(pcc_auxil_t auxil, pcc_char_array_t *array) {
    array->len = 0;
    array->max = 0;
    array->buf = NULL;
}

static void pcc_char_array__add(pcc_auxil_t auxil, pcc_char_array_t *array, char ch) {
    if (array->max <= array->len) {
        const size_t n = array->len + 1;
        size_t m = array->max;
        if (m == 0) m = PCC_BUFFER_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n;
        array->buf = (char *)PCC_REALLOC(auxil, array->buf, m);
        array->max = m;
    }
    array->buf[array->len++] = ch;
}

static void pcc_char_array__term(pcc_auxil_t auxil, pcc_char_array_t *array) {
    PCC_FREE(auxil, array->buf);
}

static void pcc_value_table__init(pcc_auxil_t auxil, pcc_value_table_t *table) {
    table->len = 0;
    table->max = 0;
    table->buf = NULL;
}

MARK_FUNC_AS_USED
static void pcc_value_table__resize(pcc_auxil_t auxil, pcc_value_table_t *table, size_t len) {
    if (table->max < len) {
        size_t m = table->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < len && m != 0) m <<= 1;
        if (m == 0) m = len;
        table->buf = (pcc_value_t *)PCC_REALLOC(auxil, table->buf, sizeof(pcc_value_t) * m);
        table->max = m;
    }
    table->len = len;
}

MARK_FUNC_AS_USED
static void pcc_value_table__clear(pcc_auxil_t auxil, pcc_value_table_t *table) {
    memset(table->buf, 0, sizeof(pcc_value_t) * table->len);
}

static void pcc_value_table__term(pcc_auxil_t auxil, pcc_value_table_t *table) {
    PCC_FREE(auxil, table->buf);
}

static void pcc_value_refer_table__init(pcc_auxil_t auxil, pcc_value_refer_table_t *table) {
    table->len = 0;
    table->max = 0;
    table->buf = NULL;
}

static void pcc_value_refer_table__resize(pcc_auxil_t auxil, pcc_value_refer_table_t *table, size_t len) {
    size_t i;
    if (table->max < len) {
        size_t m = table->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < len && m != 0) m <<= 1;
        if (m == 0) m = len;
        table->buf = (pcc_value_t **)PCC_REALLOC(auxil, table->buf, sizeof(pcc_value_t *) * m);
        table->max = m;
    }
    for (i = table->len; i < len; i++) table->buf[i] = NULL;
    table->len = len;
}

static void pcc_value_refer_table__term(pcc_auxil_t auxil, pcc_value_refer_table_t *table) {
    PCC_FREE(auxil, table->buf);
}

static void pcc_capture_table__init(pcc_auxil_t auxil, pcc_capture_table_t *table) {
    table->len = 0;
    table->max = 0;
    table->buf = NULL;
}

MARK_FUNC_AS_USED
static void pcc_capture_table__resize(pcc_auxil_t auxil, pcc_capture_table_t *table, size_t len) {
    size_t i;
    for (i = len; i < table->len; i++) PCC_FREE(auxil, table->buf[i].string);
    if (table->max < len) {
        size_t m = table->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < len && m != 0) m <<= 1;
        if (m == 0) m = len;
        table->buf = (pcc_capture_t *)PCC_REALLOC(auxil, table->buf, sizeof(pcc_capture_t) * m);
        table->max = m;
    }
    for (i = table->len; i < len; i++) {
        table->buf[i].range.start = 0;
        table->buf[i].range.end = 0;
        table->buf[i].string = NULL;
    }
    table->len = len;
}

static void pcc_capture_table__term(pcc_auxil_t auxil, pcc_capture_table_t *table) {
    while (table->len > 0) {
        table->len--;
        PCC_FREE(auxil, table->buf[table->len].string);
    }
    PCC_FREE(auxil, table->buf);
}

static void pcc_capture_const_table__init(pcc_auxil_t auxil, pcc_capture_const_table_t *table) {
    table->len = 0;
    table->max = 0;
    table->buf = NULL;
}

static void pcc_capture_const_table__resize(pcc_auxil_t auxil, pcc_capture_const_table_t *table, size_t len) {
    size_t i;
    if (table->max < len) {
        size_t m = table->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < len && m != 0) m <<= 1;
        if (m == 0) m = len;
        table->buf = (const pcc_capture_t **)PCC_REALLOC(auxil, (pcc_capture_t **)table->buf, sizeof(const pcc_capture_t *) * m);
        table->max = m;
    }
    for (i = table->len; i < len; i++) table->buf[i] = NULL;
    table->len = len;
}

static void pcc_capture_const_table__term(pcc_auxil_t auxil, pcc_capture_const_table_t *table) {
    PCC_FREE(auxil, (void *)table->buf);
}

MARK_FUNC_AS_USED
static pcc_thunk_t *pcc_thunk__create_leaf(pcc_auxil_t auxil, pcc_action_t action, size_t valuec, size_t captc) {
    pcc_thunk_t *const thunk = (pcc_thunk_t *)PCC_MALLOC(auxil, sizeof(pcc_thunk_t));
    thunk->type = PCC_THUNK_LEAF;
    pcc_value_refer_table__init(auxil, &thunk->data.leaf.values);
    pcc_value_refer_table__resize(auxil, &thunk->data.leaf.values, valuec);
    pcc_capture_const_table__init(auxil, &thunk->data.leaf.capts);
    pcc_capture_const_table__resize(auxil, &thunk->data.leaf.capts, captc);
    thunk->data.leaf.capt0.range.start = 0;
    thunk->data.leaf.capt0.range.end = 0;
    thunk->data.leaf.capt0.string = NULL;
    thunk->data.leaf.action = action;
    return thunk;
}

static pcc_thunk_t *pcc_thunk__create_node(pcc_auxil_t auxil, const pcc_thunk_array_t *thunks, pcc_value_t *value) {
    pcc_thunk_t *const thunk = (pcc_thunk_t *)PCC_MALLOC(auxil, sizeof(pcc_thunk_t));
    thunk->type = PCC_THUNK_NODE;
    thunk->data.node.thunks = thunks;
    thunk->data.node.value = value;
    return thunk;
}

static void pcc_thunk__destroy(pcc_auxil_t auxil, pcc_thunk_t *thunk) {
    if (thunk == NULL) return;
    switch (thunk->type) {
    case PCC_THUNK_LEAF:
        PCC_FREE(auxil, thunk->data.leaf.capt0.string);
        pcc_capture_const_table__term(auxil, &thunk->data.leaf.capts);
        pcc_value_refer_table__term(auxil, &thunk->data.leaf.values);
        break;
    case PCC_THUNK_NODE:
        break;
    default: /* unknown */
        break;
    }
    PCC_FREE(auxil, thunk);
}

static void pcc_thunk_array__init(pcc_auxil_t auxil, pcc_thunk_array_t *array) {
    array->len = 0;
    array->max = 0;
    array->buf = NULL;
}

static void pcc_thunk_array__add(pcc_auxil_t auxil, pcc_thunk_array_t *array, pcc_thunk_t *thunk) {
    if (array->max <= array->len) {
        const size_t n = array->len + 1;
        size_t m = array->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n;
        array->buf = (pcc_thunk_t **)PCC_REALLOC(auxil, array->buf, sizeof(pcc_thunk_t *) * m);
        array->max = m;
    }
    array->buf[array->len++] = thunk;
}

static void pcc_thunk_array__revert(pcc_auxil_t auxil, pcc_thunk_array_t *array, size_t len) {
    while (array->len > len) {
        array->len--;
        pcc_thunk__destroy(auxil, array->buf[array->len]);
    }
}

static void pcc_thunk_array__term(pcc_auxil_t auxil, pcc_thunk_array_t *array) {
    while (array->len > 0) {
        array->len--;
        pcc_thunk__destroy(auxil, array->buf[array->len]);
    }
    PCC_FREE(auxil, array->buf);
}

static void pcc_memory_recycler__init(pcc_auxil_t auxil, pcc_memory_recycler_t *recycler, size_t element_size) {
    recycler->pool_list = NULL;
    recycler->entry_list = NULL;
    recycler->element_size = element_size;
}

static void *pcc_memory_recycler__supply(pcc_auxil_t auxil, pcc_memory_recycler_t *recycler) {
    if (recycler->entry_list) {
        pcc_memory_entry_t *const tmp = recycler->entry_list;
        recycler->entry_list = tmp->next;
        return tmp;
    }
    if (!recycler->pool_list || recycler->pool_list->unused == 0) {
        size_t size = PCC_POOL_MIN_SIZE;
        if (recycler->pool_list) {
            size = recycler->pool_list->allocated << 1;
            if (size == 0) size = recycler->pool_list->allocated;
        }
        {
            pcc_memory_pool_t *const pool = (pcc_memory_pool_t *)PCC_MALLOC(
                auxil, sizeof(pcc_memory_pool_t) + recycler->element_size * size
            );
            pool->allocated = size;
            pool->unused = size;
            pool->next = recycler->pool_list;
            recycler->pool_list = pool;
        }
    }
    recycler->pool_list->unused--;
    return (char *)recycler->pool_list + sizeof(pcc_memory_pool_t) + recycler->element_size * recycler->pool_list->unused;
}

static void pcc_memory_recycler__recycle(pcc_auxil_t auxil, pcc_memory_recycler_t *recycler, void *ptr) {
    pcc_memory_entry_t *const tmp = (pcc_memory_entry_t *)ptr;
    tmp->next = recycler->entry_list;
    recycler->entry_list = tmp;
}

static void pcc_memory_recycler__term(pcc_auxil_t auxil, pcc_memory_recycler_t *recycler) {
    while (recycler->pool_list) {
        pcc_memory_pool_t *const tmp = recycler->pool_list;
        recycler->pool_list = tmp->next;
        PCC_FREE(auxil, tmp);
    }
}

MARK_FUNC_AS_USED
static pcc_thunk_chunk_t *pcc_thunk_chunk__create(pcc_context_t *ctx) {
    pcc_thunk_chunk_t *const chunk = (pcc_thunk_chunk_t *)pcc_memory_recycler__supply(ctx->auxil, &ctx->thunk_chunk_recycler);
    pcc_value_table__init(ctx->auxil, &chunk->values);
    pcc_capture_table__init(ctx->auxil, &chunk->capts);
    pcc_thunk_array__init(ctx->auxil, &chunk->thunks);
    chunk->pos = 0;
    return chunk;
}

static void pcc_thunk_chunk__destroy(pcc_context_t *ctx, pcc_thunk_chunk_t *chunk) {
    if (chunk == NULL) return;
    pcc_thunk_array__term(ctx->auxil, &chunk->thunks);
    pcc_capture_table__term(ctx->auxil, &chunk->capts);
    pcc_value_table__term(ctx->auxil, &chunk->values);
    pcc_memory_recycler__recycle(ctx->auxil, &ctx->thunk_chunk_recycler, chunk);
}

static void pcc_rule_set__init(pcc_auxil_t auxil, pcc_rule_set_t *set) {
    set->len = 0;
    set->max = 0;
    set->buf = NULL;
}

static size_t pcc_rule_set__index(pcc_auxil_t auxil, const pcc_rule_set_t *set, pcc_rule_t rule) {
    size_t i;
    for (i = 0; i < set->len; i++) {
        if (set->buf[i] == rule) return i;
    }
    return PCC_VOID_VALUE;
}

static pcc_bool_t pcc_rule_set__add(pcc_auxil_t auxil, pcc_rule_set_t *set, pcc_rule_t rule) {
    const size_t i = pcc_rule_set__index(auxil, set, rule);
    if (i != PCC_VOID_VALUE) return PCC_FALSE;
    if (set->max <= set->len) {
        const size_t n = set->len + 1;
        size_t m = set->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n;
        set->buf = (pcc_rule_t *)PCC_REALLOC(auxil, set->buf, sizeof(pcc_rule_t) * m);
        set->max = m;
    }
    set->buf[set->len++] = rule;
    return PCC_TRUE;
}

static pcc_bool_t pcc_rule_set__remove(pcc_auxil_t auxil, pcc_rule_set_t *set, pcc_rule_t rule) {
    const size_t i = pcc_rule_set__index(auxil, set, rule);
    if (i == PCC_VOID_VALUE) return PCC_FALSE;
    memmove(set->buf + i, set->buf + (i + 1), sizeof(pcc_rule_t) * (set->len - (i + 1)));
    return PCC_TRUE;
}

static void pcc_rule_set__clear(pcc_auxil_t auxil, pcc_rule_set_t *set) {
    set->len = 0;
}

static void pcc_rule_set__copy(pcc_auxil_t auxil, pcc_rule_set_t *set, const pcc_rule_set_t *src) {
    size_t i;
    pcc_rule_set__clear(auxil, set);
    for (i = 0; i < src->len; i++) {
        pcc_rule_set__add(auxil, set, src->buf[i]);
    }
}

static void pcc_rule_set__term(pcc_auxil_t auxil, pcc_rule_set_t *set) {
    PCC_FREE(auxil, set->buf);
}

static pcc_lr_head_t *pcc_lr_head__create(pcc_context_t *ctx, pcc_rule_t rule) {
    pcc_lr_head_t *const head = (pcc_lr_head_t *)pcc_memory_recycler__supply(ctx->auxil, &ctx->lr_head_recycler);
    head->rule = rule;
    pcc_rule_set__init(ctx->auxil, &head->invol);
    pcc_rule_set__init(ctx->auxil, &head->eval);
    head->hold = NULL;
    return head;
}

static void pcc_lr_head__destroy(pcc_context_t *ctx, pcc_lr_head_t *head) {
    if (head == NULL) return;
    pcc_lr_head__destroy(ctx, head->hold);
    pcc_rule_set__term(ctx->auxil, &head->eval);
    pcc_rule_set__term(ctx->auxil, &head->invol);
    pcc_memory_recycler__recycle(ctx->auxil, &ctx->lr_head_recycler, head);
}

static void pcc_lr_entry__destroy(pcc_auxil_t auxil, pcc_lr_entry_t *lr);

static pcc_lr_answer_t *pcc_lr_answer__create(pcc_context_t *ctx, pcc_lr_answer_type_t type, size_t pos) {
    pcc_lr_answer_t *answer = (pcc_lr_answer_t *)pcc_memory_recycler__supply(ctx->auxil, &ctx->lr_answer_recycler);
    answer->type = type;
    answer->pos = pos;
    answer->hold = NULL;
    switch (answer->type) {
    case PCC_LR_ANSWER_LR:
        answer->data.lr = NULL;
        break;
    case PCC_LR_ANSWER_CHUNK:
        answer->data.chunk = NULL;
        break;
    default: /* unknown */
        PCC_FREE(ctx->auxil, answer);
        answer = NULL;
    }
    return answer;
}

static void pcc_lr_answer__set_chunk(pcc_context_t *ctx, pcc_lr_answer_t *answer, pcc_thunk_chunk_t *chunk) {
    pcc_lr_answer_t *const a = pcc_lr_answer__create(ctx, answer->type, answer->pos);
    switch (answer->type) {
    case PCC_LR_ANSWER_LR:
        a->data.lr = answer->data.lr;
        break;
    case PCC_LR_ANSWER_CHUNK:
        a->data.chunk = answer->data.chunk;
        break;
    default: /* unknown */
        break;
    }
    a->hold = answer->hold;
    answer->hold = a;
    answer->type = PCC_LR_ANSWER_CHUNK;
    answer->data.chunk = chunk;
}

static void pcc_lr_answer__destroy(pcc_context_t *ctx, pcc_lr_answer_t *answer) {
    while (answer != NULL) {
        pcc_lr_answer_t *const a = answer->hold;
        switch (answer->type) {
        case PCC_LR_ANSWER_LR:
            pcc_lr_entry__destroy(ctx->auxil, answer->data.lr);
            break;
        case PCC_LR_ANSWER_CHUNK:
            pcc_thunk_chunk__destroy(ctx, answer->data.chunk);
            break;
        default: /* unknown */
            break;
        }
        pcc_memory_recycler__recycle(ctx->auxil, &ctx->lr_answer_recycler, answer);
        answer = a;
    }
}

static void pcc_lr_memo_map__init(pcc_auxil_t auxil, pcc_lr_memo_map_t *map) {
    map->len = 0;
    map->max = 0;
    map->buf = NULL;
}

static size_t pcc_lr_memo_map__index(pcc_context_t *ctx, pcc_lr_memo_map_t *map, pcc_rule_t rule) {
    size_t i;
    for (i = 0; i < map->len; i++) {
        if (map->buf[i].rule == rule) return i;
    }
    return PCC_VOID_VALUE;
}

static void pcc_lr_memo_map__put(pcc_context_t *ctx, pcc_lr_memo_map_t *map, pcc_rule_t rule, pcc_lr_answer_t *answer) {
    const size_t i = pcc_lr_memo_map__index(ctx, map, rule);
    if (i != PCC_VOID_VALUE) {
        pcc_lr_answer__destroy(ctx, map->buf[i].answer);
        map->buf[i].answer = answer;
    }
    else {
        if (map->max <= map->len) {
            const size_t n = map->len + 1;
            size_t m = map->max;
            if (m == 0) m = PCC_ARRAY_MIN_SIZE;
            while (m < n && m != 0) m <<= 1;
            if (m == 0) m = n;
            map->buf = (pcc_lr_memo_t *)PCC_REALLOC(ctx->auxil, map->buf, sizeof(pcc_lr_memo_t) * m);
            map->max = m;
        }
        map->buf[map->len].rule = rule;
        map->buf[map->len].answer = answer;
        map->len++;
    }
}

static pcc_lr_answer_t *pcc_lr_memo_map__get(pcc_context_t *ctx, pcc_lr_memo_map_t *map, pcc_rule_t rule) {
    const size_t i = pcc_lr_memo_map__index(ctx, map, rule);
    return (i != PCC_VOID_VALUE) ? map->buf[i].answer : NULL;
}

static void pcc_lr_memo_map__term(pcc_context_t *ctx, pcc_lr_memo_map_t *map) {
    while (map->len > 0) {
        map->len--;
        pcc_lr_answer__destroy(ctx, map->buf[map->len].answer);
    }
    PCC_FREE(ctx->auxil, map->buf);
}

static pcc_lr_table_entry_t *pcc_lr_table_entry__create(pcc_context_t *ctx) {
    pcc_lr_table_entry_t *const entry = (pcc_lr_table_entry_t *)PCC_MALLOC(ctx->auxil, sizeof(pcc_lr_table_entry_t));
    entry->head = NULL;
    pcc_lr_memo_map__init(ctx->auxil, &entry->memos);
    entry->hold_a = NULL;
    entry->hold_h = NULL;
    return entry;
}

static void pcc_lr_table_entry__destroy(pcc_context_t *ctx, pcc_lr_table_entry_t *entry) {
    if (entry == NULL) return;
    pcc_lr_head__destroy(ctx, entry->hold_h);
    pcc_lr_answer__destroy(ctx, entry->hold_a);
    pcc_lr_memo_map__term(ctx, &entry->memos);
    PCC_FREE(ctx->auxil, entry);
}

static void pcc_lr_table__init(pcc_auxil_t auxil, pcc_lr_table_t *table) {
    table->ofs = 0;
    table->len = 0;
    table->max = 0;
    table->buf = NULL;
}

static void pcc_lr_table__resize(pcc_context_t *ctx, pcc_lr_table_t *table, size_t len) {
    size_t i;
    for (i = len; i < table->len; i++) pcc_lr_table_entry__destroy(ctx, table->buf[i]);
    if (table->max < len) {
        size_t m = table->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < len && m != 0) m <<= 1;
        if (m == 0) m = len;
        table->buf = (pcc_lr_table_entry_t **)PCC_REALLOC(ctx->auxil, table->buf, sizeof(pcc_lr_table_entry_t *) * m);
        table->max = m;
    }
    for (i = table->len; i < len; i++) table->buf[i] = NULL;
    table->len = len;
}

static void pcc_lr_table__set_head(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index, pcc_lr_head_t *head) {
    index += table->ofs;
    if (index >= table->len) pcc_lr_table__resize(ctx, table, index + 1);
    if (table->buf[index] == NULL) table->buf[index] = pcc_lr_table_entry__create(ctx);
    table->buf[index]->head = head;
}

static void pcc_lr_table__hold_head(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index, pcc_lr_head_t *head) {
    index += table->ofs;
    if (index >= table->len) pcc_lr_table__resize(ctx, table, index + 1);
    if (table->buf[index] == NULL) table->buf[index] = pcc_lr_table_entry__create(ctx);
    head->hold = table->buf[index]->hold_h;
    table->buf[index]->hold_h = head;
}

static void pcc_lr_table__set_answer(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index, pcc_rule_t rule, pcc_lr_answer_t *answer) {
    index += table->ofs;
    if (index >= table->len) pcc_lr_table__resize(ctx, table, index + 1);
    if (table->buf[index] == NULL) table->buf[index] = pcc_lr_table_entry__create(ctx);
    pcc_lr_memo_map__put(ctx, &table->buf[index]->memos, rule, answer);
}

static void pcc_lr_table__hold_answer(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index, pcc_lr_answer_t *answer) {
    index += table->ofs;
    if (index >= table->len) pcc_lr_table__resize(ctx, table, index + 1);
    if (table->buf[index] == NULL) table->buf[index] = pcc_lr_table_entry__create(ctx);
    answer->hold = table->buf[index]->hold_a;
    table->buf[index]->hold_a = answer;
}

static pcc_lr_head_t *pcc_lr_table__get_head(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index) {
    index += table->ofs;
    if (index >= table->len || table->buf[index] == NULL) return NULL;
    return table->buf[index]->head;
}

static pcc_lr_answer_t *pcc_lr_table__get_answer(pcc_context_t *ctx, pcc_lr_table_t *table, size_t index, pcc_rule_t rule) {
    index += table->ofs;
    if (index >= table->len || table->buf[index] == NULL) return NULL;
    return pcc_lr_memo_map__get(ctx, &table->buf[index]->memos, rule);
}

static void pcc_lr_table__shift(pcc_context_t *ctx, pcc_lr_table_t *table, size_t count) {
    size_t i;
    if (count > table->len - table->ofs) count = table->len - table->ofs;
    for (i = 0; i < count; i++) pcc_lr_table_entry__destroy(ctx, table->buf[table->ofs++]);
    if (table->ofs > (table->max >> 1)) {
        memmove(table->buf, table->buf + table->ofs, sizeof(pcc_lr_table_entry_t *) * (table->len - table->ofs));
        table->len -= table->ofs;
        table->ofs = 0;
    }
}

static void pcc_lr_table__term(pcc_context_t *ctx, pcc_lr_table_t *table) {
    while (table->len > table->ofs) {
        table->len--;
        pcc_lr_table_entry__destroy(ctx, table->buf[table->len]);
    }
    PCC_FREE(ctx->auxil, table->buf);
}

static pcc_lr_entry_t *pcc_lr_entry__create(pcc_auxil_t auxil, pcc_rule_t rule) {
    pcc_lr_entry_t *const lr = (pcc_lr_entry_t *)PCC_MALLOC(auxil, sizeof(pcc_lr_entry_t));
    lr->rule = rule;
    lr->seed = NULL;
    lr->head = NULL;
    return lr;
}

static void pcc_lr_entry__destroy(pcc_auxil_t auxil, pcc_lr_entry_t *lr) {
    PCC_FREE(auxil, lr);
}

static void pcc_lr_stack__init(pcc_auxil_t auxil, pcc_lr_stack_t *stack) {
    stack->len = 0;
    stack->max = 0;
    stack->buf = NULL;
}

static void pcc_lr_stack__push(pcc_auxil_t auxil, pcc_lr_stack_t *stack, pcc_lr_entry_t *lr) {
    if (stack->max <= stack->len) {
        const size_t n = stack->len + 1;
        size_t m = stack->max;
        if (m == 0) m = PCC_ARRAY_MIN_SIZE;
        while (m < n && m != 0) m <<= 1;
        if (m == 0) m = n;
        stack->buf = (pcc_lr_entry_t **)PCC_REALLOC(auxil, stack->buf, sizeof(pcc_lr_entry_t *) * m);
        stack->max = m;
    }
    stack->buf[stack->len++] = lr;
}

static pcc_lr_entry_t *pcc_lr_stack__pop(pcc_auxil_t auxil, pcc_lr_stack_t *stack) {
    return stack->buf[--stack->len];
}

static void pcc_lr_stack__term(pcc_auxil_t auxil, pcc_lr_stack_t *stack) {
    PCC_FREE(auxil, stack->buf);
}

static pcc_context_t *pcc_context__create(pcc_auxil_t auxil) {
    pcc_context_t *const ctx = (pcc_context_t *)PCC_MALLOC(auxil, sizeof(pcc_context_t));
    ctx->pos = 0;
    ctx->cur = 0;
    ctx->level = 0;
    pcc_char_array__init(auxil, &ctx->buffer);
    pcc_lr_table__init(auxil, &ctx->lrtable);
    pcc_lr_stack__init(auxil, &ctx->lrstack);
    pcc_thunk_array__init(auxil, &ctx->thunks);
    pcc_memory_recycler__init(auxil, &ctx->thunk_chunk_recycler, sizeof(pcc_thunk_chunk_t));
    pcc_memory_recycler__init(auxil, &ctx->lr_head_recycler, sizeof(pcc_lr_head_t));
    pcc_memory_recycler__init(auxil, &ctx->lr_answer_recycler, sizeof(pcc_lr_answer_t));
    ctx->auxil = auxil;
    return ctx;
}

static void pcc_context__destroy(pcc_context_t *ctx) {
    if (ctx == NULL) return;
    pcc_thunk_array__term(ctx->auxil, &ctx->thunks);
    pcc_lr_stack__term(ctx->auxil, &ctx->lrstack);
    pcc_lr_table__term(ctx, &ctx->lrtable);
    pcc_char_array__term(ctx->auxil, &ctx->buffer);
    pcc_memory_recycler__term(ctx->auxil, &ctx->thunk_chunk_recycler);
    pcc_memory_recycler__term(ctx->auxil, &ctx->lr_head_recycler);
    pcc_memory_recycler__term(ctx->auxil, &ctx->lr_answer_recycler);
    PCC_FREE(ctx->auxil, ctx);
}

static size_t pcc_refill_buffer(pcc_context_t *ctx, size_t num) {
    if (ctx->buffer.len >= ctx->cur + num) return ctx->buffer.len - ctx->cur;
    while (ctx->buffer.len < ctx->cur + num) {
        const int c = PCC_GETCHAR(ctx->auxil);
        if (c < 0) break;
        pcc_char_array__add(ctx->auxil, &ctx->buffer, (char)c);
    }
    return ctx->buffer.len - ctx->cur;
}

MARK_FUNC_AS_USED
static void pcc_commit_buffer(pcc_context_t *ctx) {
    memmove(ctx->buffer.buf, ctx->buffer.buf + ctx->cur, ctx->buffer.len - ctx->cur);
    ctx->buffer.len -= ctx->cur;
    ctx->pos += ctx->cur;
    pcc_lr_table__shift(ctx, &ctx->lrtable, ctx->cur);
    ctx->cur = 0;
}

MARK_FUNC_AS_USED
static const char *pcc_get_capture_string(pcc_context_t *ctx, const pcc_capture_t *capt) {
    if (capt->string == NULL)
        ((pcc_capture_t *)capt)->string =
            pcc_strndup_e(ctx->auxil, ctx->buffer.buf + capt->range.start, capt->range.end - capt->range.start);
    return capt->string;
}

MARK_FUNC_AS_USED
static pcc_bool_t pcc_apply_rule(pcc_context_t *ctx, pcc_rule_t rule, pcc_thunk_array_t *thunks, pcc_value_t *value) {
    static pcc_value_t null;
    pcc_thunk_chunk_t *c = NULL;
    const size_t p = ctx->pos + ctx->cur;
    pcc_bool_t b = PCC_TRUE;
    pcc_lr_answer_t *a = pcc_lr_table__get_answer(ctx, &ctx->lrtable, p, rule);
    pcc_lr_head_t *h = pcc_lr_table__get_head(ctx, &ctx->lrtable, p);
    if (h != NULL) {
        if (a == NULL && rule != h->rule && pcc_rule_set__index(ctx->auxil, &h->invol, rule) == PCC_VOID_VALUE) {
            b = PCC_FALSE;
            c = NULL;
        }
        else if (pcc_rule_set__remove(ctx->auxil, &h->eval, rule)) {
            b = PCC_FALSE;
            c = rule(ctx);
            a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_CHUNK, ctx->pos + ctx->cur);
            a->data.chunk = c;
            pcc_lr_table__hold_answer(ctx, &ctx->lrtable, p, a);
        }
    }
    if (b) {
        if (a != NULL) {
            ctx->cur = a->pos - ctx->pos;
            switch (a->type) {
            case PCC_LR_ANSWER_LR:
                if (a->data.lr->head == NULL) {
                    a->data.lr->head = pcc_lr_head__create(ctx, rule);
                    pcc_lr_table__hold_head(ctx, &ctx->lrtable, p, a->data.lr->head);
                }
                {
                    size_t i = ctx->lrstack.len;
                    while (i > 0) {
                        i--;
                        if (ctx->lrstack.buf[i]->head == a->data.lr->head) break;
                        ctx->lrstack.buf[i]->head = a->data.lr->head;
                        pcc_rule_set__add(ctx->auxil, &a->data.lr->head->invol, ctx->lrstack.buf[i]->rule);
                    }
                }
                c = a->data.lr->seed;
                break;
            case PCC_LR_ANSWER_CHUNK:
                c = a->data.chunk;
                break;
            default: /* unknown */
                break;
            }
        }
        else {
            pcc_lr_entry_t *const e = pcc_lr_entry__create(ctx->auxil, rule);
            pcc_lr_stack__push(ctx->auxil, &ctx->lrstack, e);
            a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_LR, p);
            a->data.lr = e;
            pcc_lr_table__set_answer(ctx, &ctx->lrtable, p, rule, a);
            c = rule(ctx);
            pcc_lr_stack__pop(ctx->auxil, &ctx->lrstack);
            a->pos = ctx->pos + ctx->cur;
            if (e->head == NULL) {
                pcc_lr_answer__set_chunk(ctx, a, c);
            }
            else {
                e->seed = c;
                h = a->data.lr->head;
                if (h->rule != rule) {
                    c = a->data.lr->seed;
                    a = pcc_lr_answer__create(ctx, PCC_LR_ANSWER_CHUNK, ctx->pos + ctx->cur);
                    a->data.chunk = c;
                    pcc_lr_table__hold_answer(ctx, &ctx->lrtable, p, a);
                }
                else {
                    pcc_lr_answer__set_chunk(ctx, a, a->data.lr->seed);
                    if (a->data.chunk == NULL) {
                        c = NULL;
                    }
                    else {
                        pcc_lr_table__set_head(ctx, &ctx->lrtable, p, h);
                        for (;;) {
                            ctx->cur = p - ctx->pos;
                            pcc_rule_set__copy(ctx->auxil, &h->eval, &h->invol);
                            c = rule(ctx);
                            if (c == NULL || ctx->pos + ctx->cur <= a->pos) break;
                            pcc_lr_answer__set_chunk(ctx, a, c);
                            a->pos = ctx->pos + ctx->cur;
                        }
                        pcc_thunk_chunk__destroy(ctx, c);
                        pcc_lr_table__set_head(ctx, &ctx->lrtable, p, NULL);
                        ctx->cur = a->pos - ctx->pos;
                        c = a->data.chunk;
                    }
                }
            }
        }
    }
    if (c == NULL) return PCC_FALSE;
    if (value == NULL) value = &null;
    memset(value, 0, sizeof(pcc_value_t)); /* in case */
    pcc_thunk_array__add(ctx->auxil, thunks, pcc_thunk__create_node(ctx->auxil, &c->thunks, value));
    return PCC_TRUE;
}

MARK_FUNC_AS_USED
static void pcc_do_action(pcc_context_t *ctx, const pcc_thunk_array_t *thunks, pcc_value_t *value) {
    size_t i;
    for (i = 0; i < thunks->len; i++) {
        pcc_thunk_t *const thunk = thunks->buf[i];
        switch (thunk->type) {
        case PCC_THUNK_LEAF:
            thunk->data.leaf.action(ctx, thunk, value);
            break;
        case PCC_THUNK_NODE:
            pcc_do_action(ctx, thunk->data.node.thunks, thunk->data.node.value);
            break;
        default: /* unknown */
            break;
        }
    }
}

static void pcc_action_FILE_0(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    __.
    $__
    $_0
    _0.
    $0$
    $01
    _0s.
    $0s$
    $0s1
    $01s
    _0e.
    $0e$
    $0e1
    $01e
    _123.
    $123$
    $123_
    _123s.
    $123s$
    $123s_
    _123e.
    $123e$
    $123e_
    foo1
    FOO1
    ${bar}
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_1(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    __
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_2(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _0
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_3(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _0s
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_4(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _0e
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_5(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _123
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_6(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _123s
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static void pcc_action_FILE_7(foo1_context_t *__pcc_ctx, pcc_thunk_t *__pcc_in, pcc_value_t *__pcc_out) {
#define auxil (__pcc_ctx->auxil)
#define __ (*__pcc_out)
#define _0 pcc_get_capture_string(__pcc_ctx, &__pcc_in->data.leaf.capt0)
#define _0s ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.start))
#define _0e ((const size_t)(__pcc_ctx->pos + __pcc_in->data.leaf.capt0.range.end))
    _123e
#undef _0e
#undef _0s
#undef _0
#undef __
#undef auxil
}

static pcc_thunk_chunk_t *pcc_evaluate_rule_FILE(pcc_context_t *ctx);

static pcc_thunk_chunk_t *pcc_evaluate_rule_FILE(pcc_context_t *ctx) {
    pcc_thunk_chunk_t *const chunk = pcc_thunk_chunk__create(ctx);
    chunk->pos = ctx->cur;
    PCC_DEBUG(ctx->auxil, PCC_DBG_EVALUATE, "FILE", ctx->level, chunk->pos, (ctx->buffer.buf + chunk->pos), (ctx->buffer.len - chunk->pos));
    ctx->level++;
    {
        MARK_VAR_AS_USED
        const size_t p = ctx->cur;
        MARK_VAR_AS_USED
        const size_t n = chunk->thunks.len;
        if (
            pcc_refill_buffer(ctx, 1) < 1 ||
            ctx->buffer.buf[ctx->cur] != '_'
        ) goto L0002;
        ctx->cur++;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_0, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0002:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 ||
            ctx->buffer.buf[ctx->cur] != '$'
        ) goto L0003;
        ctx->cur++;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_1, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0003:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 ||
            ctx->buffer.buf[ctx->cur] != '0'
        ) goto L0004;
        ctx->cur++;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_2, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0004:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 || (ctx->buffer.buf + ctx->cur)[0] != '0' ||
            pcc_refill_buffer(ctx, 2) < 2 || (ctx->buffer.buf + ctx->cur)[1] != 's'
        ) goto L0005;
        ctx->cur += 2;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_3, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0005:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 || (ctx->buffer.buf + ctx->cur)[0] != '0' ||
            pcc_refill_buffer(ctx, 2) < 2 || (ctx->buffer.buf + ctx->cur)[1] != 'e'
        ) goto L0006;
        ctx->cur += 2;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_4, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0006:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 || (ctx->buffer.buf + ctx->cur)[0] != '1' ||
            pcc_refill_buffer(ctx, 2) < 2 || (ctx->buffer.buf + ctx->cur)[1] != '2' ||
            pcc_refill_buffer(ctx, 3) < 3 || (ctx->buffer.buf + ctx->cur)[2] != '3'
        ) goto L0007;
        ctx->cur += 3;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_5, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0007:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 || (ctx->buffer.buf + ctx->cur)[0] != '1' ||
            pcc_refill_buffer(ctx, 2) < 2 || (ctx->buffer.buf + ctx->cur)[1] != '2' ||
            pcc_refill_buffer(ctx, 3) < 3 || (ctx->buffer.buf + ctx->cur)[2] != '3' ||
            pcc_refill_buffer(ctx, 4) < 4 || (ctx->buffer.buf + ctx->cur)[3] != 's'
        ) goto L0008;
        ctx->cur += 4;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_6, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0008:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        if (
            pcc_refill_buffer(ctx, 1) < 1 || (ctx->buffer.buf + ctx->cur)[0] != '1' ||
            pcc_refill_buffer(ctx, 2) < 2 || (ctx->buffer.buf + ctx->cur)[1] != '2' ||
            pcc_refill_buffer(ctx, 3) < 3 || (ctx->buffer.buf + ctx->cur)[2] != '3' ||
            pcc_refill_buffer(ctx, 4) < 4 || (ctx->buffer.buf + ctx->cur)[3] != 'e'
        ) goto L0009;
        ctx->cur += 4;
        {
            pcc_thunk_t *const thunk = pcc_thunk__create_leaf(ctx->auxil, pcc_action_FILE_7, 0, 0);
            thunk->data.leaf.capt0.range.start = chunk->pos;
            thunk->data.leaf.capt0.range.end = ctx->cur;
            pcc_thunk_array__add(ctx->auxil, &chunk->thunks, thunk);
        }
        goto L0001;
    L0009:;
        ctx->cur = p;
        pcc_thunk_array__revert(ctx->auxil, &chunk->thunks, n);
        goto L0000;
    L0001:;
    }
    ctx->level--;
    PCC_DEBUG(ctx->auxil, PCC_DBG_MATCH, "FILE", ctx->level, chunk->pos, (ctx->buffer.buf + chunk->pos), (ctx->cur - chunk->pos));
    return chunk;
L0000:;
    ctx->level--;
    PCC_DEBUG(ctx->auxil, PCC_DBG_NOMATCH, "FILE", ctx->level, chunk->pos, (ctx->buffer.buf + chunk->pos), (ctx->cur - chunk->pos));
    pcc_thunk_chunk__destroy(ctx, chunk);
    return NULL;
}

foo1_context_t *foo1_create(long auxil) {
    return pcc_context__create(auxil);
}

int foo1_parse(foo1_context_t *ctx, double *ret) {
    if (pcc_refill_buffer(ctx, 1) < 1) return 0;
    if (pcc_apply_rule(ctx, pcc_evaluate_rule_FILE, &ctx->thunks, ret))
        pcc_do_action(ctx, &ctx->thunks, ret);
    else
        PCC_ERROR(ctx->auxil);
    pcc_commit_buffer(ctx);
    pcc_thunk_array__revert(ctx->auxil, &ctx->thunks, 0);
    return 1;
}

void foo1_destroy(foo1_context_t *ctx) {
    pcc_context__destroy(ctx);
}

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
