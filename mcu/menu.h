#ifndef __MENU_H__
#define __MENU_H__

#define MENU_CHARACTERS_SIZE 0x400

void menu_init(void);
void menu_data_get(uint8_t *data);
void menu_bank_set(uint8_t data);
void menu_name_set(uint8_t data);
void menu_bin_size_lsb(uint8_t data);
void menu_bin_size_msb(uint8_t data);
void menu_bin_addr_lsb(uint8_t data);
void menu_bin_addr_msb(uint8_t data);
void menu_bin_data_set(uint8_t data);
void menu_bin_store(void);
void menu_up(void);
void menu_down(void);
void menu_bin_select(uint8_t data);

#endif /* __MENU_H__ */
