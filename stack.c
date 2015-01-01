#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "quark.h"

#ifndef QUARK_PARSE_STACK_INIT_SIZE
#define QUARK_PARSE_STACK_INIT_SIZE 256
#endif

void* quark_context_push(quark_context *context, size_t size) {
    void *rtn;
    assert(size > 0);
    if (context->stack.top + size >= context->stack.size) {
        if (context->stack.size == 0)
            context->stack.size = QUARK_PARSE_STACK_INIT_SIZE;
        while (context->stack.top + size >= context->stack.size)
            context->stack.size += context->stack.size >> 1; /* stack.size*1.5 */
        context->stack.value = (char*)realloc(context->stack.value, context->stack.size);
    }
    rtn = context->stack.value + context->stack.top;
    context->stack.top += size;
    return rtn;
}

void* quark_context_pop(quark_context *context, size_t size) {
    assert(context->stack.top >= size);
    return context->stack.value + (context->stack.top -= size);
}
