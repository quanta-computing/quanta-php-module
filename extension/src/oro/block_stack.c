#include "quanta_mon.h"

oro_block_t *oro_block_stack_pop(oro_context_t *context) {
  oro_block_t *block;
  oro_block_stack_t *stack;

  stack = context->block_stack;
  if (!stack)
    return NULL;
  context->block_stack = context->block_stack->prev;
  block = stack->block;
  efree(stack);
  return block;
}

void oro_block_stack_push(oro_context_t *context, oro_block_t *block) {
  oro_block_stack_t *stack;

  if (!(stack = ecalloc(1, sizeof(*stack))))
    return;
  stack->block = block;
  stack->prev = context->block_stack;
  context->block_stack = stack;
}

oro_block_t *oro_block_stack_top(oro_context_t *context) {
  if (!context->block_stack)
    return NULL;
  else
    return context->block_stack->block;
}
