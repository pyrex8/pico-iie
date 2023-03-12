#ifndef __KEY_H__
#define __KEY_H__

typedef enum
{
    KEY_MAIN_NULL = 0,
    KEY_MAIN_PAUSE,
    KEY_MAIN_RESUME,
    KEY_MAIN_RESET,
    KEY_MAIN_REBOOT,
    KEY_MAIN_MENU,
    KEY_KEYBOARD_KEY,
    KEY_OPERATIONS_TOTAL,
} KeyOperation;

void key_init(void);
void key_update(void);
void key_operation_update(KeyOperation *operation, uint8_t *data);

#endif /* __KEY_H__ */
