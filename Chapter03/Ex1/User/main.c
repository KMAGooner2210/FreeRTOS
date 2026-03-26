#include "stm32f10x.h"                  // Device header
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
	
	printf("Task Heap Management Started!\r\n");
	
	for(;;){
		pcDynamicString = (char *)pvPortMalloc(50 * sizeof(char));
		
		if(pcDynamicString != NULL){
			TickType_t xCurrentTick = xTaskGetTickCount();
			sprintf(pcDynamicString,  "System Tick: %u | Memory: OK\r\n", (unsigned int)xCurrentTick);
			
			printf("%s", pcDynamicString);
			
			vPortFree(pcDynamicString);
			
			pcDynamicString = NULL;
			
		} else {
			
			printf("Error: Malloc Failed!\r\n");
			
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
	