#pragma once

#include "gol.h"

typedef struct console_dimensions
{
    uint32_t x;
    uint32_t y;
} console_dimensions;


console_dimensions
get_console_dimensions(void);