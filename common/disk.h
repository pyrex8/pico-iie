#pragma once

void disk_init(void);
void disk_update(uint8_t read, uint16_t address, uint8_t *byte);
uint8_t disk_not_spinning_test(void);
void disk_nib_file_data_set(uint32_t location, uint8_t data);