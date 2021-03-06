#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdbool.h>
#include <stdint.h>

#define VIDEO_BYTES_PER_LINE 40
#define SCREEN_LINE_OFFSET 0x80
#define VIDEO_SEGMENT_OFFSET 0x28

void video_init(void);
void video_update(uint8_t read, uint16_t address, uint8_t *byte);
void video_scan_line_set(uint16_t line);
void video_buffer_clear(void);
void video_buffer_get(uint16_t *buf);
uint16_t video_address_get(void);
void video_line_data_get(uint8_t *video_line_data);

#endif /* __VIDEO_H__ */
