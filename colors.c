#include <windows.h>

void enable_colors()
{
    HANDLE h   = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    GetConsoleMode(h, &mode);

    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
