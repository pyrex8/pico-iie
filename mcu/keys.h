#ifndef __KEYS_H__
#define __KEYS_H__

void keys_init(void);
void keys_update(void);
uint8_t keys_data_waiting(void);
uint8_t keys_data_get(void);

#endif /* __KEYS_H__ */
