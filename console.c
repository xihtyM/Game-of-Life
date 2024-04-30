#include "console.h"

console_dimensions
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