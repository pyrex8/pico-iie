#ifndef __ROM_H__
#define __ROM_H__

void rom_init(void);
void rom_update(uint8_t read, uint16_t address, uint8_t *byte);
void rom_reset_vector_write(uint8_t lower_byte, uint8_t upper_byte);

#endif /* __ROM_H__ */
