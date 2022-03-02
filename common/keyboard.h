#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void keyboard_update(uint8_t read, uint16_t address, uint8_t *byte);
void keyboard_key_code_set(uint8_t key_code);

#endif /* __KEYBOARD_H__ */
