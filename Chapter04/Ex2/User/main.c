/* 
   Bai 2: UART Logger
   Muc tieu: Tao 2 task doc lap gui qua UART
   STM32F103 (Blue Pill)
	 - UART1
   - PA9 (TX)
   - PA10  (RX)
*/

#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>


int fputc(int ch, FILE *f){
    USART_SendData(USART1, (uint8_t)ch);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}

// Task A: In log moi 1 s
void vTaskA(void *pvParameters){
	for(;;)
	{
		printf("Task A is running...\r\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


// Task B: In log moi 2s
void vTaskB(void *pvParameters){
	for(;;)
	{
		printf("Task B is running...\r\n");
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

int main(void){
		SystemInit();
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);

		xTaskCreate(vTaskA, "TaskA", 128, NULL, 2, NULL);
		xTaskCreate(vTaskB, "TaskB", 128, NULL, 2, NULL);	

		vTaskStartScheduler();
		for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}



//#include "stm32f10x.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h" // Thu vi?n cho Semaphore
//#include <stdio.h>

//// Khai báo Mutex d? b?o v? UART
//SemaphoreHandle_t xUARTMutex;

//// Hàm fputc dã s?a l?i
//int fputc(int ch, FILE *f){
//    // G?i ký t?
//    USART_SendData(USART1, (uint8_t)ch);
//    // Ch? cho d?n khi g?i xong (TC = Transmission Complete)
//    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
//    return ch;
//}

//// Task A: In log moi 1s
//void vTaskA(void *pvParameters){
//    for(;;)
//    {
//        // Xin quy?n s? d?ng UART
//        if(xSemaphoreTake(xUARTMutex, portMAX_DELAY) == pdTRUE)
//        {
//            printf("Task A is running...\r\n");
//            // Tr? quy?n s? d?ng UART
//            xSemaphoreGive(xUARTMutex);
//        }
//        vTaskDelay(pdMS_TO_TICKS(1000));
//    }
//}

//// Task B: In log moi 2s
//void vTaskB(void *pvParameters){
//    for(;;)
//    {
//        // Xin quy?n s? d?ng UART
//        if(xSemaphoreTake(xUARTMutex, portMAX_DELAY) == pdTRUE)
//        {
//            printf("Task B is running...\r\n");
//            // Tr? quy?n s? d?ng UART
//            xSemaphoreGive(xUARTMutex);
//        }
//        vTaskDelay(pdMS_TO_TICKS(2000));
//    }
//}

//int main(void){
//    SystemInit();
//    
//    // C?p Clock
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
//    
//    GPIO_InitTypeDef GPIO_InitStructure;
//    
//    // C?u hình PA9 (TX)
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // C?u hình PA10 (RX)
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // C?u hình UART1
//    USART_InitTypeDef USART_InitStructure;
//    USART_InitStructure.USART_BaudRate = 9600;
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_Init(USART1, &USART_InitStructure);

//    USART_Cmd(USART1, ENABLE);

//    // T?o Mutex tru?c khi t?o Task
//    xUARTMutex = xSemaphoreCreateMutex();

//    if(xUARTMutex != NULL){
//        xTaskCreate(vTaskA, "TaskA", 128, NULL, 2, NULL);
//        xTaskCreate(vTaskB, "TaskB", 128, NULL, 2, NULL);   
//        vTaskStartScheduler();
//    }

//    for(;;);
//}