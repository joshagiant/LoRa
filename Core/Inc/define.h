#define REDLED_ON HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin, RESET)
#define REDLED_OFF HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin, SET)
#define BLUELED_ON HAL_GPIO_WritePin(LED2_GPIO_Port,LED1_Pin, RESET)
#define BLUELED_OFF HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin, SET)

// Just change this one:
#define REDPLAYER 1

// Don't change these:
#if REDPLAYER > 0
    #define THIS_PLAYER RED
    #define OTHER_PLAYER BLUE
#else
    #define THIS_PLAYER BLUE
    #define OTHER_PLAYER RED
#endif
