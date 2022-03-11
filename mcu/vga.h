#ifndef __VGA_H__
#define __VGA_H__

#include <stdbool.h>
#include <stdint.h>

void vga_init(void);
int16_t vga_overscan_line_get(void);
uint16_t *vga_scan_line_buffer(void);
bool vga_overscan_line_is_odd(void);
uint16_t vga_scan_line_get(void);
void vga_wait_for_new_overscan_line(void);

#endif /* __VGA_H__ */
