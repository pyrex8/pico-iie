#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdbool.h>
#include <stdint.h>

#define VIDEO_BYTES_PER_LINE 40
#define SCREEN_LINE_OFFSET 0x80
#define VIDEO_SEGMENT_OFFSET 0x28

void video_init(void);
void video_update(uint8_t read, uint16_t address, uint8_t *byte);
void video_hires_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_text_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_scan_line_set(uint16_t line);
void video_buffer_clear(void);
void video_buffer_get(uint16_t *buf);
bool video_is_mode_text(void);
uint16_t video_page_get(void);

#endif /* __VIDEO_H__ */
