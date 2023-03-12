#ifndef __KEYS_H__
#define __KEYS_H__

typedef enum
{
    KEYS_MAIN_NULL = 0,
    KEYS_MAIN_PAUSE,
    KEYS_MAIN_RESET,
    KEYS_MAIN_REBOOT,
    KEYS_MAIN_MENU,
    KEYS_KEYBOARD_KEY,
    KEYS_OPERATIONS_TOTAL,
} KeysOperation;

void keys_init(void);
void keys_update(void);
void keys_operation_update(KeysOperation *operation, uint8_t *data);

#endif /* __KEYS_H__ */
