#ifndef __TEST_H__
#define __TEST_H__

#include <stdint.h>

// Configuration
#define TEST0_PIN_NUMBER 22
#define TEST1_PIN_NUMBER 26
#define TEST_PIN_NUMBER (TEST0_PIN_NUMBER)
// I/O slot 7 device select 0xC0F0 to 0xC0FF
#define TEST_PIN_ADDR 0xC0F0

void test0_pin_init(void);
void test0_pin_low(void);
void test0_pin_high(void);

void test1_pin_init(void);
void test1_pin_low(void);
void test1_pin_high(void);

void test_pin_init(void);
void test_pin_update(uint8_t read, uint16_t address, uint8_t *byte);

#endif /* __TEST_H__ */
