#ifndef __KEY_H__
#define __KEY_H__

typedef enum
{
    KEY_MAIN_NULL = 0,
    KEY_MAIN_PAUSE,
    KEY_MAIN_RESUME,
    KEY_MAIN_RESET,
    KEY_MAIN_MENU,
    KEY_MAIN_MENU_SELECT,
    KEY_OPERATIONS_TOTAL,
} KeyOperation;

void key_init(void);
void key_update(void);
void key_command(KeyOperation *operation, uint8_t *data);
uint8_t key_data_waiting(void);
uint8_t key_data_get(void);

#endif /* __KEY_H__ */
