#ifndef __MENU_H__
#define __MENU_H__

#define MENU_CHARACTERS_SIZE 0x400

void menu_init(void);
void menu_update(void);
void menu_data_get(uint8_t *data);
void menu_bank_set(uint8_t data);
void menu_name_set(uint8_t data);

#endif /* __MENU_H__ */
