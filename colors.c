#include <windows.h>

// Default cmd.exe terminal and some crappy electron ones don't support
// proper color output by default, so we have to enable it manually
void enable_colors()
{
    HANDLE h   = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);

    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
