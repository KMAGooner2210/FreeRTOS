#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" 
#include "task.h"



void UART1_Config(void){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

int fputc(int ch, FILE *f){
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, (uint8_t)ch);
	return ch;
}



void vHeapTask(void *pvParameters){
	char *pcDynamicString;
	TickType_t xLastWakeTime;
	const TickType_t xDelay1000ms = pdMS_TO_TICKS(1000);
	
	xLastWakeTime = xTaskGetTickCount();
	
	printf("--- Heap Management Demo ---\r\n");
	printf("Initial Free Heap Size: %u bytes\r\n\n", (unsigned int)xPortGetFreeHeapSize());
	
	for(;;){

		size_t freeHeapBefore = xPortGetFreeHeapSize();
		printf("Loop start. Free Heap: %u\r\n", (unsigned int)freeHeapBefore);
		

		pcDynamicString = (char *)pvPortMalloc(50);
		
		if(pcDynamicString != NULL){
			printf("  -> Malloc OK. Free Heap is now: %u\r\n", (unsigned int)xPortGetFreeHeapSize());
			

			sprintf(pcDynamicString, "  Message sent at Tick: %u", (unsigned int)xTaskGetTickCount());
			printf("%s\r\n", pcDynamicString);
			

			vPortFree(pcDynamicString);
			pcDynamicString = NULL;
			

			printf("  -> vPortFree called. Free Heap is now: %u\r\n\n", (unsigned int)xPortGetFreeHeapSize());
			
		} else {
			printf("!!! MALLOC FAILED! Not enough memory. Free Heap: %u bytes.\r\n\n", (unsigned int)freeHeapBefore);
		}
		
		vTaskDelayUntil(&xLastWakeTime, xDelay1000ms);
	}
}

int main(){
	SystemInit();
	UART1_Config();
	
	xTaskCreate(vHeapTask,
							"HeapTask",
							256,
							NULL,
							1,
							NULL);
	
	vTaskStartScheduler();
	while(1);
}