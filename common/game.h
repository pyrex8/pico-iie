#ifndef __GAME_H__
#define __GAME_H__

void game_update(uint8_t read, uint16_t address, uint8_t *byte);
void game_state_set(uint8_t btn0, uint8_t btn1, uint8_t pdl0, uint8_t pdl1);
void game_btn0_set(uint8_t btn0);
void game_btn1_set(uint8_t btn1);
void game_pdl0_set(uint8_t pdl0);
void game_pdl1_set(uint8_t pdl1);

#endif /* __GAME_H__ */
