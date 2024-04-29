#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <time.h>


#define FPS 15

#if FPS > 1000
#define CLOCK_CYCLES 1000
#else
#define CLOCK_CYCLES FPS
#endif

#define WIDTH 250
#define HEIGHT 100

#define BOARD_CELLS_WIDTH ((WIDTH + 7) >> 3)
#define BOARD_LENGTH (BOARD_CELLS_WIDTH * HEIGHT)
#define BOARD_WIDTH (BOARD_CELLS_WIDTH << 3)
#define BOARD_SIZE (BOARD_WIDTH * HEIGHT)

#define BORDER_WIDTH 1

#define BOARD_BUFSIZE 1 + (BOARD_LENGTH << 3) + ((BOARD_WIDTH + (BORDER_WIDTH << 1)) << 1) + (HEIGHT << 1)


typedef struct cell_8
{
    int8_t cell_pack;
} cell_8;

inline void _set_cell_value(
    cell_8 *cptr,
    uint8_t index,
    bool is_alive)
{
    if (is_alive)
    {
        cptr->cell_pack |= 1 << index;
        return;
    }
    
    cptr->cell_pack &= ~(1 << index);
}

inline bool _get_cell_value(
    cell_8 *cptr,
    uint8_t index)
{
    return cptr->cell_pack & (1 << index);
}

typedef struct board
{
    cell_8 board_cells[BOARD_LENGTH];
} board;

inline void set_cell_value(
    board *b,
    uint32_t idx,
    bool is_alive)
{
    uint32_t block = idx/8;
    _set_cell_value(&b->board_cells[block], idx - (block << 3), is_alive);
}

inline bool get_cell_value(
    board *b,
    uint32_t idx)
{
    uint32_t block = idx/8;
    return _get_cell_value(&b->board_cells[block], idx - (block << 3));
}

void board_init(
    board *b,
    float cell_frequency);

void print_board(
    board *b,
    uint32_t width,
    uint32_t height);

uint8_t get_cell_neighbours_line(
    board *b,
    uint32_t idx);

typedef struct console_dimensions
{
    uint32_t x;
    uint32_t y;
} console_dimensions;

inline console_dimensions
get_console_dimensions(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    uint32_t x, y;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return (console_dimensions){WIDTH, HEIGHT};
    
    x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return (console_dimensions){x, y};
}