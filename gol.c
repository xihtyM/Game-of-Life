#include "gol.h"
#include <windows.h>
#include <conio.h>

#define ARROW_KEY   0xE0

#define ARROW_UP    0x48
#define ARROW_DOWN  0x50

#define ARROW_LEFT  0x4B
#define ARROW_RIGHT 0x4D


void board_init(
    board *b,
    float cell_frequency)
{
    for (uint32_t idx = 0; idx < BOARD_SIZE; idx++)
    {
        if (rand() < (RAND_MAX * cell_frequency))
            set_cell_value(b, idx, true);
        else
            set_cell_value(b, idx, false);
    }
}

void print_board(
    board *b,
    uint32_t width,
    uint32_t height)
{
    if (width > BOARD_WIDTH)
        width = BOARD_WIDTH;
    if (height > HEIGHT)
        height = HEIGHT;

    char buf[BOARD_BUFSIZE];

    uint32_t buf_idx = 0;

    for (; buf_idx < width + (BORDER_WIDTH << 1); buf_idx++) buf[buf_idx] = '#';

    buf[buf_idx++] = '\n';
    
    for (uint32_t y = 0; y < height; y++)
    {
        buf[buf_idx++] = '#';

        for (uint32_t x = 0; x < width; x++)
        {
            buf[buf_idx++] = (47 * get_cell_value(b, x + (y * BOARD_WIDTH))) + ' ';
        }

        buf[buf_idx++] = '#';
        buf[buf_idx++] = '\n';
    }

    for (uint16_t i = 0; i < width + 2; buf_idx++, i++) buf[buf_idx] = '#';

    buf[buf_idx] = '\0';

    fputs(buf, stdout);
    fflush(stdout);
}

uint8_t get_cell_neighbours_line(
    board *b,
    uint32_t idx)
{
    uint32_t block = idx/8;
    uint8_t block_mid = idx - (block * 8);

    // max: 3 bits
    uint8_t neighbour_bits;

    // if idx is the start of a new block
    if (!block_mid)
    {
        neighbour_bits = b->board_cells[block].cell_pack & 0b11;
        
        if (block > 0 && idx % BOARD_WIDTH) neighbour_bits |= (b->board_cells[block - 1].cell_pack & 128) >> 5;
    }
    // else if idx is the end of a new block
    else if (block_mid == 7)
    {
        neighbour_bits = (b->board_cells[block].cell_pack & 0b11000000) >> 5;
        
        if ((block + 1) < BOARD_LENGTH && (idx + 1) % BOARD_WIDTH) neighbour_bits |= b->board_cells[block + 1].cell_pack & 1;
    }
    else
    {
        neighbour_bits = (b->board_cells[block].cell_pack >> (block_mid - 1)) & 0b111;
    }

    return (neighbour_bits & 1) + ((neighbour_bits & 2) >> 1) + ((neighbour_bits & 4) >> 2);
}

uint8_t get_cell_neighbours(
    board *b,
    uint32_t idx)
{
    uint8_t neighbours = get_cell_neighbours_line(b, idx);
    
    if (idx > BOARD_WIDTH)
        neighbours += get_cell_neighbours_line(b, idx - BOARD_WIDTH);
    
    if (idx + BOARD_WIDTH < (BOARD_LENGTH << 3))
        neighbours += get_cell_neighbours_line(b, idx + BOARD_WIDTH);
    
    return neighbours - get_cell_value(b, idx);
}

void update_board(
    board *b)
{
    board board_copy;
    for (uint32_t idx = 0; idx < BOARD_LENGTH; idx++) board_copy.board_cells[idx] = b->board_cells[idx];


    for (uint32_t idx = 0; idx < BOARD_SIZE; idx++)
    {
        uint8_t neighbours = get_cell_neighbours(&board_copy, idx);
        bool cell_value = get_cell_value(&board_copy, idx);
        
        if (cell_value && (neighbours < 2 || neighbours > 3))
            set_cell_value(b, idx, false);
        else if (!cell_value && neighbours == 3)
            set_cell_value(b, idx, true);
    }
}

void handle_arrow_press(int id, COORD *cpos, console_dimensions *d)
{
    switch (id)
    {
        case ARROW_LEFT:
            if (cpos->X > BORDER_WIDTH) cpos->X--;
            break;
        case ARROW_RIGHT:
            if (cpos->X < (d->x - BORDER_WIDTH - 1)) cpos->X++;
            break;
        case ARROW_UP:
            if (cpos->Y > (BORDER_WIDTH + 1)) cpos->Y--;
            break;
        case ARROW_DOWN:
            if (cpos->Y < (d->y - BORDER_WIDTH - 1)) cpos->Y++;
            break;
    }

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), *cpos);
}

void pause_gol(void)
{
    console_dimensions d = get_console_dimensions();
    COORD cursor_position = {BORDER_WIDTH, BORDER_WIDTH + 1};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor_position);

    while (true)
    {
        Sleep(10);

        if (kbhit())
        {
            int ch = _getch();

            switch (ch)
            {
                case 'P':
                case 'p':
                    return;
                case ARROW_KEY:
                {
                    handle_arrow_press(_getch(), &cursor_position, &d);
                    break;
                }
                default:
                {
                    // clear stdout
                    while (kbhit()) _getch();
                }
            }
        }
    }
}

void handle_keypress(void)
{
    int ch = _getch();
    
    switch (ch)
    {
        case 'P':
        case 'p':
        {
            pause_gol();
            break;
        }
        default:
        {
            // clear stdout
            while (kbhit()) _getch();
        }
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    setvbuf(stdout, NULL, _IOFBF, BOARD_BUFSIZE); 

    board b;

    board_init(&b, 0.3);

    console_dimensions d;

    uint32_t framerate = 0;
    uint64_t frames = 0;
    uint64_t prev_frames = 0;
    time_t capture = time(0);

    while (true)
    {
        d = get_console_dimensions();

        if (difftime(time(0), capture))
        {
            framerate = frames - prev_frames;
            prev_frames = frames;
            capture++;
        }

        // redraw the board
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) {0, 0});

        for (uint16_t i = 2; i < d.x + (BORDER_WIDTH << 1); i++) fputc(' ', stdout);
        printf("\rFPS: %d\tIterations: %lld\tKeybinds --> Pause: 'p'\tNavigation: Arrow keys\n", framerate, frames);
        print_board(&b, d.x - 2, d.y - 3);


        if (kbhit())
            handle_keypress();
        
        // update the board for next iteration
        update_board(&b);
        frames++;

        // bit less than 1000/FPS just because it needs a few milliseconds to print
        // to the output, 925 seems like the best
        Sleep(925/FPS);
    }

    return 0;
}