#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include <stdint.h>

// Configuration
#define SPEAKER_PIN_NUMBER 28

void speaker_init(void);

void speaker_update(uint8_t read, uint16_t address, uint8_t *byte);

#endif /* __SPEAKER_H__ */
