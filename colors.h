#ifndef COLORS_H
#define COLORS_H

#define BLACK_FG   "\033[30m"
#define RED_FG     "\033[31m"
#define GREEN_FG   "\033[32m"
#define YELLOW_FG  "\033[33m"
#define BLUE_FG    "\033[34m"
#define MAGENTA_FG "\033[35m"
#define CYAN_FG    "\033[36m"
#define WHITE_FG   "\033[37m"

#define BLACK_BG   "\033[40m"
#define RED_BG     "\033[41m"
#define GREEN_BG   "\033[42m"
#define YELLOW_BG  "\033[43m"
#define BLUE_BG    "\033[44m"
#define MAGENTA_BG "\033[45m"
#define CYAN_BG    "\033[46m"
#define WHITE_BG   "\033[47m"

#define RESET     "\033[0m"
#define BOLD      "\033[1m"
#define DIM       "\033[2m"
#define ITALIC    "\033[3m"
#define UNDERLINE "\033[4m"
#define BLINK     "\033[5m"
#define REVERSE   "\033[6m"
#define HIDDEN    "\033[7m"

void enable_colors();

#endif /* COLORS_H */
