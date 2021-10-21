#include "stack.h"

static s32 g_stack[1024];
static s32 g_stack_top;
static s32 g_stack_bot;

void stack_init() {
  g_stack_top = 0;
  g_stack_bot = 0;
}

s32 stack_empty() {
  return g_stack_top == g_stack_bot;
}

s32 stack_top() {
  return g_stack[g_stack_top - 1];
}

s32 stack_bot() {
  return g_stack[g_stack_bot];
}

void stack_push(s32 value) {
  g_stack[g_stack_top++] = value;
}

void stack_pop_top() {
  g_stack_top--;
}

void stack_pop_bot() {
  g_stack_bot++;
}

