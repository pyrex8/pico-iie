#ifndef __MENU_H__
#define __MENU_H__

#define MENU_CHARACTERS_X 40
#define MENU_CHARACTERS_Y 20
#define MENU_CHARACTERS_SIZE (MENU_CHARACTERS_Y * MENU_CHARACTERS_X)

void menu_init(void);
void menu_update(void);
void menu_data_get(uint8_t *data);

#endif /* __MENU_H__ */
