#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

volatile uint8_t isPrinting = 1;		// 1: Dang in, 0: Tam dung

void UART_Print(const char *msg){
	while(*msg){
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, *msg++);
	}
}

void UART_std_print(const char * msg){
	vTaskSuspendAll();
	UART_Print(msg);
	xTaskResumeAll();
}

// Task 1: Long String
void Task1(void *PvParameters){
	while(1){
		if(isPrinting){
		UART_std_print("TASK1 IS RUNNING.............\r\n");
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

// Task 2: Print another long string
void Task2(void *pvParameters){
	while(1){
		if(isPrinting){
		UART_std_print("TASK2 IS INTERRUPTING.............\r\n");
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

void UART_Config(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    // TX: PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // RX: PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
		
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler(void){
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		uint8_t received = USART_ReceiveData(USART1);
		isPrinting = !isPrinting;
		
		if(isPrinting) UART_Print("---- RESUMED ----\r\n");
		else UART_Print(" ---- PAUSED ---- \r\n");
	}
}

int main(){
	SystemInit();
	SystemCoreClockUpdate();
	
	UART_Config();
	
	xTaskCreate(Task1, "Task1", 256, NULL, 2, NULL);
	xTaskCreate(Task2, "Task2", 256, NULL, 2, NULL);
	
	vTaskStartScheduler();
	
	while(1);
}
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
    while(1); 
}
void vApplicationMallocFailedHook(void) {
    while(1); 
}
		