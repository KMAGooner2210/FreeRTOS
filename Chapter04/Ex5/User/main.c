#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>


int fputc(int ch, FILE *f)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
    return ch;
}


void USART1_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_USART1 |
                           RCC_APB2Periph_AFIO, ENABLE);

    // PA9 - TX
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10 - RX
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate   = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}


void DummyWork_200ms(void)
{
    volatile uint32_t i;
    for (i = 0; i < 2000000; i++); 
}

void vDriftTask(void *pvParameters)
{
    TickType_t tick;

    while (1)
    {
        tick = xTaskGetTickCount();
        printf("[DRIFT ] Start tick = %lu\r\n", tick);

        DummyWork_200ms();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vPrecisionTask(void *pvParameters)
{
    TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        printf("[PRECI ] Start tick = %lu\r\n", xLastWakeTime);

        DummyWork_200ms();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    USART1_Config();

    printf("Demo start\r\n");

    xTaskCreate(vDriftTask,
                "Drift",
                256,
                NULL,
                1,
                NULL);

    xTaskCreate(vPrecisionTask,
                "Precision",
                256,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();

    while (1);
}
