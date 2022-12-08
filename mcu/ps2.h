#ifndef __PS2_H__
#define __PS2_H__

void ps2_init(void);
void ps2_update(void);
bool ps2_data_ready(void);
uint8_t ps2_data_get(void);

#endif /* __PS2_H__ */
