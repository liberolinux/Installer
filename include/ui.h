#ifndef LIBERO_INSTALLER_UI_H
#define LIBERO_INSTALLER_UI_H

#include "common.h"

int ui_init(void);
void ui_shutdown(void);
void ui_status(const char *message);
void ui_message(const char *title, const char *message);
bool ui_confirm(const char *title, const char *message);
int ui_menu(const char *title,
            const char *subtitle,
            const char **items,
            size_t count,
            int selected);
int ui_prompt_input(const char *title,
                    const char *prompt,
                    char *buffer,
                    size_t buffer_len,
                    const char *initial,
                    bool secret);

#endif /* LIBERO_INSTALLER_UI_H */
