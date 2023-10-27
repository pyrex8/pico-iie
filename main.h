#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>

void main_null(uint8_t unused);
void main_reboot(uint8_t unused);
void main_reset(uint8_t unused);
void main_pause(uint8_t unused);
void main_resume(uint8_t unused);
void main_menu(uint8_t unused);
void main_bin_addr_lsb(uint8_t data);
void main_bin_addr_msb(uint8_t data);
void main_bin_data_set(uint8_t data);
void main_start_bin(uint8_t unused);

void main_store_bin(uint8_t unused);
#endif /* __MAIN_H__ */
