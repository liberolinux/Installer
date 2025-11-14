#include <ncurses.h>

#include "ui.h"

static bool g_ui_ready = false;
static WINDOW *status_win = NULL;

static void draw_header(const char *title, const char *subtitle)
{
    int width = COLS - 2;
    mvhline(0, 0, ' ', COLS);
    mvprintw(0, 2, "%s", title ? title : INSTALLER_NAME);
    if (subtitle) {
        mvprintw(1, 2, "%s", subtitle);
    }
    mvhline(2, 0, ACS_HLINE, COLS);
    refresh();
}

int ui_init(void)
{
    if (g_ui_ready) {
        return 0;
    }

    if (!initscr()) {
        fprintf(stderr, "Unable to initialize ncurses\n");
        return -1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);

    status_win = newwin(1, COLS, LINES - 1, 0);
    g_ui_ready = true;
    return 0;
}

void ui_shutdown(void)
{
    if (!g_ui_ready) {
        return;
    }
    if (status_win) {
        delwin(status_win);
        status_win = NULL;
    }
    endwin();
    g_ui_ready = false;
}

void ui_status(const char *message)
{
    if (!g_ui_ready || !status_win) {
        return;
    }
    werase(status_win);
    mvwprintw(status_win, 0, 1, "%s", message ? message : "");
    wrefresh(status_win);
}

static void wait_for_keypress(void)
{
    int ch;
    while ((ch = getch())) {
        if (ch == '\n' || ch == KEY_ENTER || ch == 27 || ch == 'q') {
            break;
        }
    }
}

void ui_message(const char *title, const char *message)
{
    if (!g_ui_ready) {
        printf("%s\n", message ? message : "");
        return;
    }
    clear();
    draw_header(title ? title : INSTALLER_NAME, NULL);
    mvprintw(4, 2, "%s", message ? message : "");
    mvprintw(LINES - 3, 2, "Press Enter to continue...");
    refresh();
    wait_for_keypress();
}

bool ui_confirm(const char *title, const char *message)
{
    const char *items[] = {"Yes", "No"};
    int choice = ui_menu(title, message, items, 2, 1);
    return choice == 0;
}

int ui_menu(const char *title,
            const char *subtitle,
            const char **items,
            size_t count,
            int selected)
{
    if (!g_ui_ready || !items || count == 0) {
        return -1;
    }

    int highlight = selected;
    if (highlight < 0 || (size_t)highlight >= count) {
        highlight = 0;
    }

    while (1) {
        clear();
        draw_header(title ? title : INSTALLER_NAME, subtitle);
        for (size_t i = 0; i < count; ++i) {
            if ((int)i == highlight) {
                attron(A_REVERSE | COLOR_PAIR(1));
                mvprintw((int)(4 + i), 4, "> %s", items[i]);
                attroff(A_REVERSE | COLOR_PAIR(1));
            } else {
                mvprintw((int)(4 + i), 4, "  %s", items[i]);
            }
        }
        mvprintw(LINES - 3, 2, "Use arrow keys to navigate, Enter to select, q to exit");
        refresh();

        int ch = getch();
        switch (ch) {
        case KEY_UP:
            highlight = (highlight == 0) ? (int)count - 1 : highlight - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight == (int)count - 1) ? 0 : highlight + 1;
            break;
        case 'q':
        case 27:
            return -1;
        case '\n':
        case KEY_ENTER:
            return highlight;
        default:
            break;
        }
    }

    return -1;
}

int ui_prompt_input(const char *title,
                    const char *prompt,
                    char *buffer,
                    size_t buffer_len,
                    const char *initial,
                    bool secret)
{
    if (!buffer || buffer_len == 0) {
        return -1;
    }

    char temp[1024];
    memset(temp, 0, sizeof(temp));
    size_t limit = (buffer_len < sizeof(temp)) ? buffer_len - 1 : sizeof(temp) - 1;
    if ((int64_t)limit < 0) {
        limit = 0;
    }

    size_t len = 0;
    if (initial && *initial) {
        snprintf(temp, sizeof(temp), "%.*s", (int)limit, initial);
        len = strlen(temp);
    } else {
        temp[0] = '\0';
    }

    while (1) {
        clear();
        draw_header(title ? title : INSTALLER_NAME, NULL);
        mvprintw(4, 2, "%s", prompt ? prompt : "Input:");
        if (secret) {
            char mask[1024];
            size_t mask_len = len < sizeof(mask) - 1 ? len : sizeof(mask) - 1;
            memset(mask, '*', mask_len);
            mask[mask_len] = '\0';
            mvprintw(6, 4, "%s", mask);
        } else {
            mvprintw(6, 4, "%s", temp);
        }
        move(6, 4 + (int)len);
        refresh();

        int ch = getch();
        if (ch == '\n' || ch == KEY_ENTER) {
            snprintf(buffer, buffer_len, "%s", temp);
            return 0;
        }
        if (ch == 27) {
            return -1;
        }
        if (ch == KEY_BACKSPACE || ch == 127) {
            if (len > 0) {
                temp[--len] = '\0';
            }
            continue;
        }

        if (isprint(ch) && len < limit) {
            temp[len++] = (char)ch;
            temp[len] = '\0';
        }
    }
}
