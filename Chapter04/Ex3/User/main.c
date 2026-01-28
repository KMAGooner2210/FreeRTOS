#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t      pin;
    TickType_t    delay;
    BitAction     onLevel;   
} LedTaskParam_t;


void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);   

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);  
}

void vLedTaskCode(void *pvParameters)
{
    LedTaskParam_t *led = (LedTaskParam_t *)pvParameters;

    while (1)
    {
        /* LED ON */
        GPIO_WriteBit(led->port, led->pin, led->onLevel);
        vTaskDelay(led->delay);

        /* LED OFF */
        GPIO_WriteBit(led->port,
                      led->pin,
                      (led->onLevel == Bit_SET) ? Bit_RESET : Bit_SET);
        vTaskDelay(led->delay);
    }
}

int main(void)
{
    GPIO_Config();


    static LedTaskParam_t ledGreen =
    {
        GPIOC,
        GPIO_Pin_13,
        pdMS_TO_TICKS(1000),
        Bit_RESET        // PC13 active LOW
    };

    static LedTaskParam_t ledRed =
    {
        GPIOA,
        GPIO_Pin_0,
        pdMS_TO_TICKS(500),
        Bit_SET          // PA0 active HIGH
    };


    xTaskCreate(vLedTaskCode,
                "LED_PC13",
                128,
                &ledGreen,
                1,
                NULL);

    xTaskCreate(vLedTaskCode,
                "LED_PA0",
                128,
                &ledRed,
                1,
                NULL);

    vTaskStartScheduler();

    while (1);
}
