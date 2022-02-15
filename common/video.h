#pragma once

void video_scan_line_pixel_conversion(void);
void video_scan_line_zero(void);
void video_scan_line_buffer_transfer(uint8_t *buffer);
void video_hires_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_text_line_update(uint16_t video_line_number, uint8_t *video_line_data);
void video_buffer_clear(void);
void video_buffer_get(uint8_t *buf);
