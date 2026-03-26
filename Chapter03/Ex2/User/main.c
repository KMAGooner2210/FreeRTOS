#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

void vPrintHeapStats(const char* pcTag){
	size_t xFreeHeap = xPortGetFreeHeapSize();
	size_t xMinEverFree = xPortGetMinimumEverFreeHeapSize();
	
	printf("[%s] Free: %u | MinEverFree: %u\r\n", pcTag, (unsigned int)xFreeHeap, (unsigned int)xMinEverFree);
}

void vMonitoringTask(void *pvParameters){
	char *pcData;
	const size_t xAllocSize = 100;
	
	printf("--- Heap Monitoring Demo ---\r\n");
	vPrintHeapStats("INITIAL");
	
	for(;;){
		printf("\r\n--- New Cycle ---\r\n");
		vPrintHeapStats("BEFORE MALLOC");
		
		pcData = (char *)pvPortMalloc(xAllocSize);
		
		if(pcData != NULL){
			vPrintHeapStats("AFTER MALLOC");
			
			sprintf(pcData, "Hello FreeRTOS Heap!");
			printf("Data: %s\r\n", pcData);
			
			vTaskDelay(pdMS_TO_TICKS(500));
			
			vPortFree(pcData);
			printf("Memory Freed.\r\n");
			
			vPrintHeapStats("AFTER FREE");
		}
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void UART1_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

int fputc(int ch, FILE *f) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
    return ch;
}

int main(void){
	SystemInit();
	UART1_Config();
	
	xTaskCreate(vMonitoringTask, "Monitor", 256, NULL, 1, NULL);
	
	vTaskStartScheduler();
	while(1);
}