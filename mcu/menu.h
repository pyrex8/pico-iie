#ifndef __MENU_H__
#define __MENU_H__

#define MENU_CHARACTERS_SIZE 0x400

void menu_init(void);
void menu_data_get(uint8_t *data);
void menu_bank_set(uint8_t data);
void menu_name_set(uint8_t data);
void menu_bin_size_lsb_set(uint8_t data);
void menu_bin_size_msb_set(uint8_t data);
void menu_bin_addr_lsb_set(uint8_t data);
void menu_bin_addr_msb_set(uint8_t data);
void menu_bin_data_set(uint8_t data);
uint16_t menu_bin_size_get(void);
uint16_t menu_bin_addr_get(void);
uint8_t menu_bin_data_get(void);
void menu_bin_store(void);
void menu_bin_select(uint8_t bank_select);

#endif /* __MENU_H__ */
