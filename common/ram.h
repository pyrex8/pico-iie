#ifndef __RAM_H__
#define __RAM_H__

void ram_init(void);
void ram_deinit(void);
void ram_update(uint8_t read, uint16_t address, uint8_t *byte);
void ram_data_get(uint8_t length, uint16_t address, uint8_t *data);

#endif /* __RAM_H__ */
