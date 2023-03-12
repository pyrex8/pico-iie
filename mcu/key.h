/******************************************************************************
 *
 *  description: Scan keyboard matrix and logic for deciding how to process
 *
 ******************************************************************************
 */

#ifndef __KEY_H__
#define __KEY_H__

// structs/enums/typedefs -----------------------------------------------------
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

// function prototypes --------------------------------------------------------

/**
 *  \brief      Initalize internal workings.
 */
void key_init(void);

/**
 *  \brief      Advance clock and check for data from matrix
 */
void key_update(void);

/**
 *  \brief      Get key operation, if any action is required
 *  \param[IN]  operation       return function pointer
 *  \param[IN]  data            return data to pass to function
 */
void key_operation_get(KeyOperation *operation, uint8_t *data);

#endif /* __KEY_H__ */
