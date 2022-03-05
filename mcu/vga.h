#ifndef __VGA_H__
#define __VGA_H__

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/structs/mpu.h"
#include "hardware/structs/vreg_and_chip_reset.h"



// Pixel freq 25.175MHz for VGA Signal 640 x 480 @ 60 Hz
#define PCLK_DIVIDER_INTEGER 16
#define PCLK_DIVIDER_FRACT 12
#define PCLK_PWM_COUNT 1
#define PCLK_PWM_VALUE 1

// Vertical refresh 31.46875 kHz
#define VGA_H_VISIBLE_AREA 640
#define VGA_H_FRONT_PORCH 16
#define VGA_H_SYNC_PULSE 96
#define VGA_H_BACK_PORCH 48
#define VGA_H_WHOLE_LINE 800

#define HSYNC_DIVIDER 1
#define HSYNC__DIVIDER_INTEGER (PCLK_DIVIDER_INTEGER)
#define HSYNC__DIVIDER_FRACT (PCLK_DIVIDER_FRACT)
#define HSYNC_PWM_COUNT (VGA_H_WHOLE_LINE - 1)
#define HSYNC_PWM_VALUE (VGA_H_WHOLE_LINE - VGA_H_SYNC_PULSE)

// Screen refresh rate 60 Hz
#define VGA_V_VISIBLE_AREA 480
#define VGA_V_FRONT_PORCH 10
#define VGA_V_SYNC_PULSE 2
#define VGA_V_BACK_PORCH 33
#define VGA_V_WHOLE_FRAME 525

// // 800 / 8 = 100
#define VSYNC_CLK_MULTIPLIER 8
#define VSYNC_SCAN_MULTIPLIER (VGA_H_WHOLE_LINE / VSYNC_CLK_MULTIPLIER)

#define VSYNC_DIVIDER_INTEGER 134 // 8 * (16 + 12 / 16)
#define VSYNC_DIVIDER_FRACT 0
#define VSYNC_PWM_COUNT (VGA_V_WHOLE_FRAME * VSYNC_SCAN_MULTIPLIER - 1)
#define VSYNC_PWM_VALUE (VSYNC_SCAN_MULTIPLIER * (VGA_V_WHOLE_FRAME - VGA_V_SYNC_PULSE))

#define VGA_TO_VIDEO_SCAN_LINES_DIVIDER 2
#define VIDEO_SCAN_LINES (VGA_V_WHOLE_FRAME / VGA_TO_VIDEO_SCAN_LINES_DIVIDER)

#define VIDEO_RESOLUTION_X 280
#define VIDEO_RESOLUTION_Y 192
#define VIDEO_BORDER_X ((VGA_H_VISIBLE_AREA / 2 - VIDEO_RESOLUTION_X) / 2)
#define VIDEO_BORDER_Y ((VGA_V_VISIBLE_AREA / 2 - VIDEO_RESOLUTION_Y) / 2)

#define VIDEO_SCAN_BUFFER_OFFSET ((VGA_H_BACK_PORCH + (VGA_H_VISIBLE_AREA - VIDEO_RESOLUTION_X * 2) / 2) / 2) //44
#define VIDEO_SCAN_LINE_OFFSET ((VGA_V_BACK_PORCH + (VGA_V_VISIBLE_AREA - VIDEO_RESOLUTION_Y * 2) / 2))   //40.5 rounded down

// one last value of zero otherwise last pixel repeats to the end of the scan line
#define VIDEO_SCAN_BUFFER_LEN ((VGA_H_BACK_PORCH + VGA_H_VISIBLE_AREA) / 2 + 1)
#define VIDEO_SCAN_LINE_LEN (VGA_H_WHOLE_LINE / 2)

#define VIDEO_BYTES_PER_LINE 40

#define VIDEO_SEGMENT_OFFSET 0x28
#define SCREEN_LINE_OFFSET 0x80

void vga_init(void);
uint16_t vga_overscan_line_get(void);
uint16_t *vga_scan_line_buffer(void);
void vga_blank_scan_line_set(void);
bool vga_scan_line_not_ready(void);
bool vga_scan_line_not_ready_reset(void);

#endif /* __VGA_H__ */
