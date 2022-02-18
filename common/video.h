#pragma once

void video_hires_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_text_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_buffer_clear(void);
void video_buffer_get(uint16_t *buf);
