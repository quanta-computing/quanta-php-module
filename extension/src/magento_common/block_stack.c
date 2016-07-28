#include "quanta_mon.h"

magento_block_t *block_stack_pop(magento_context_t *context) {
  magento_block_t *block;
  block_stack_t *stack;

  stack = context->block_stack;
  if (!stack)
    return NULL;
  context->block_stack = context->block_stack->prev;
  block = stack->block;
  efree(stack);
  return block;
}

void block_stack_push(magento_context_t *context, magento_block_t *block) {
  block_stack_t *stack;

  if (!(stack = ecalloc(1, sizeof(*stack))))
    return;
  stack->block = block;
  stack->prev = context->block_stack;
  context->block_stack = stack;
}

magento_block_t *block_stack_top(magento_context_t *context) {
  if (!context->block_stack)
    return NULL;
  else
    return context->block_stack->block;
}
