#include "gol.h"
#include "console.h"
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
        width = BOARD_WIDTH - 1;
    if (height > HEIGHT)
        height = HEIGHT - 1;

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
        if (!kbhit())
        {
            Sleep(10);
            continue;
        }

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

void gol_settings(uint16_t *FPS_CAP)
{
    system("cls||clear");

    printf("SETTINGS (save: 's'):\n");

    if ((*FPS_CAP) < UINT16_MAX)
        printf("FPS CAP: %u", *FPS_CAP);
    else
        printf("FPS CAP: Unlimited");
    
    fflush(stdout);
    
    while (true)
    {
        if (!kbhit())
        {
            Sleep(10);
            continue;
        }

        int ch = getch();

        if (ch == 's' || ch == 'S') break;
        if (ch != ARROW_KEY) continue;

        switch (getch())
        {
            case ARROW_UP:
            {
                (*FPS_CAP)++;
                if (!(*FPS_CAP)) (*FPS_CAP) = 1;
                break;
            }
            case ARROW_DOWN:
            {
                (*FPS_CAP)--;
                if (!(*FPS_CAP)) (*FPS_CAP) = UINT16_MAX;
                break;
            }
        }

        if ((*FPS_CAP) < UINT16_MAX)
            printf("\r                           \rFPS CAP: %u", *FPS_CAP);
        else
            printf("\r                           \rFPS CAP: Unlimited");
        fflush(stdout);
    }

    // for now just clear the line
    fputs("\r                                \r", stdout);
}

void handle_keypress(uint16_t *FPS_CAP)
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
        case 'S':
        case 's':
        {
            gol_settings(FPS_CAP);
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

    board_init(&b, 0.6);

    console_dimensions d = get_console_dimensions();
    console_dimensions d_sav = d;

    uint16_t FPS_CAP = 15;

    uint32_t framerate = 0;
    uint64_t frames = 0;
    uint64_t prev_frames = 0;
    time_t capture = time(0);

    while (true)
    {

        if (difftime(time(0), capture))
        {
            framerate = frames - prev_frames;
            prev_frames = frames;
            capture++;
        }

        // when console gets resized might aswell clear screen
        if (d_sav.x != d.x || d_sav.y != d.y)
        {
            system("cls||clear");
            d_sav = d;
        }

        // redraw the board
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD) {0, 0});

        for (uint16_t i = 2; i < d.x + (BORDER_WIDTH << 1); i++) fputc(' ', stdout);
        printf("\rFPS: %d\tFrames: %lld\tKeybinds -> Pause: 'p'\tNavigation: Arrow keys\tSettings: s\tDimensions: %dx%d\n", framerate, frames, d.x, d.y);
        print_board(&b, d.x - 2, d.y - 3);


        if (kbhit())
            handle_keypress(&FPS_CAP);
        
        // update the board for next iteration
        update_board(&b);
        frames++;

        d = get_console_dimensions();

        // bit less than 1000/FPS_CAP just because it needs a few milliseconds to print
        // to the output, 925 seems like the best
        if (FPS_CAP < 1800) Sleep(925/FPS_CAP);
    }

    return 0;
}