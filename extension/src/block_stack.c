#include "quanta_mon.h"

generate_renderize_block_details *block_stack_pop(void) {
  generate_renderize_block_details *block;
  block_stack_t *stack;

  stack = hp_globals.block_stack;
  if (!stack)
    return NULL;
  hp_globals.block_stack = hp_globals.block_stack->prev;
  block = stack->block;
  efree(stack);
  return block;
}

void block_stack_push(generate_renderize_block_details *block) {
  block_stack_t *stack;

  if (!(stack = ecalloc(1, sizeof(*stack))))
    return;
  stack->block = block;
  stack->prev = hp_globals.block_stack;
  hp_globals.block_stack = stack;
}

generate_renderize_block_details *block_stack_top(void) {
  if (!hp_globals.block_stack)
    return NULL;
  else
    return hp_globals.block_stack->block;
}
