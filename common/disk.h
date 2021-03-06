#ifndef __DISK_H__
#define __DISK_H__

void disk_init(void);
void disk_update(uint8_t read, uint16_t address, uint8_t *byte);
uint8_t disk_is_spinning(void);
void disk_file_reset(uint8_t unused);
void disk_file_data_set(uint8_t data);

#endif /* __DISK_H__ */
