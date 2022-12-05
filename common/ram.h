#ifndef __RAM_H__
#define __RAM_H__

typedef enum
{
    MEMORY_WRITE = 0,
    MEMORY_READ
} MemoryRWStatus;

void ram_init(void);
void ram_deinit(void);
void ram_update(uint8_t read, uint16_t address, uint8_t *byte);
void ram_data_get(uint8_t length, uint16_t address, uint8_t *data);
void ram_bin_reset(uint8_t unused);
void ram_bin_addr_lsb(uint8_t data);
void ram_bin_addr_msb(uint8_t data);
uint16_t ram_bin_addr_get(void);
void ram_bin_data_set(uint8_t data);

#endif /* __RAM_H__ */
