#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

void joystick_init(void);
void joystick_update(void);
uint8_t joystick_btn0_get(void);
uint8_t joystick_btn1_get(void);
uint8_t joystick_pdl0_get(void);
uint8_t joystick_pdl1_get(void);

#endif /* __JOYSTICK_H__ */
