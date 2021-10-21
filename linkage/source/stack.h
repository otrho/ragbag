#pragma once

#include <gba_types.h>

void stack_init();
s32 stack_empty();

s32 stack_top();
s32 stack_bot();

void stack_push(s32 value);

void stack_pop_top();
void stack_pop_bot();

