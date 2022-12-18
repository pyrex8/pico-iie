#ifndef __PS2_H__
#define __PS2_H__

typedef enum
{
    PS2_MAIN_NULL = 0,
    PS2_MAIN_REBOOT,
    PS2_MAIN_RESET,
    PS2_MAIN_PAUSE,
    PS2_MAIN_START_BIN,
    PS2_OPERATIONS_TOTAL,
} Ps2Operation;

void ps2_init(void);
void ps2_update(void);
void ps2_command(Ps2Operation *operation, uint8_t *data);
bool ps2_data_ready(void);
uint8_t ps2_data_get(void);

#endif /* __PS2_H__ */
