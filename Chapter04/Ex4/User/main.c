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

void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    // PA0 - Task A 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);

    // PC13 - Task B 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void USART1_Init(void)
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


void vTaskA(void *pvParameters)
{
    volatile uint32_t i;

    while (1)
    {
        printf("Task A running\r\n");

        for (i = 0; i < 3000000; i++)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_0);   // Task A active
            GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        }
    }
}


void vTaskB(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000)); 
        printf(">>> Task B PREEMPT <<<\r\n");

        GPIO_ResetBits(GPIOC, GPIO_Pin_13); // LED ON
        vTaskDelay(pdMS_TO_TICKS(200));
        GPIO_SetBits(GPIOC, GPIO_Pin_13);   // LED OFF
    }
}


int main(void)
{
    GPIO_Config();
    USART1_Init();

    printf("Preemption demo start\r\n");

    xTaskCreate(vTaskA,
                "TaskA",
                256,
                NULL,
                1,      
                NULL);

    xTaskCreate(vTaskB,
                "TaskB",
                256,
                NULL,
                2,    
                NULL);

    vTaskStartScheduler();

    while (1);
}
