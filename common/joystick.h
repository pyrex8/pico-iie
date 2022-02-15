#pragma once

void joystick_update(uint8_t read, uint16_t address, uint8_t *byte);
void joystick_state_set(uint8_t btn0, uint8_t btn1, uint8_t pdl0, uint8_t pdl1);