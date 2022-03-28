#ifndef __CASSETTE_H__
#define __CASSETTE_H__

#include <stdint.h>

// Configuration
#define CASSETTE_PIN_NUMBER 27

void cassette_init(void);

void cassette_update(uint8_t read, uint16_t address, uint8_t *byte);

#endif /* __CASSETTE_H__ */
