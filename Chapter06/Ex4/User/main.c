#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "stdio.h"


SemaphoreHandle_t xCountSem_Buffer;

void UART_Config(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
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
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void UART_Print(const char *msg){
	while(*msg){
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1,*msg++);
	}
}

void UART_std_print(const char *msg){
	vTaskSuspendAll();
	UART_Print(msg);
	xTaskResumeAll();
}

void TaskProducer(void *pvParameters){
    TickType_t delay = 200 / portTICK_RATE_MS; // Ch?y nhanh
    int count = 0;
    
    while(1){
        if(xSemaphoreGive(xCountSem_Buffer) == pdTRUE){
            UART_std_print("Producer: Da tao 1 goi tin.\r\n");
        } else {
            UART_std_print("Producer: Buffer DAY!\r\n");
        }
        
        // Sau 25 l?n t?o tin (~5 giây), chuy?n sang ch?y ch?m l?i
        if(++count > 25) delay = 1000 / portTICK_RATE_MS;
        
        vTaskDelay(delay);
    }
}

void TaskConsumer(void *pvParameters){
	char buffer[60];
	UBaseType_t uxSemaphoreCount;
	
	while(1){
		
		vTaskDelay(200 / portTICK_RATE_MS);
		// Lay semaphore trong 2 giay
		if(xSemaphoreTake(xCountSem_Buffer, 2000 / portTICK_RATE_MS) == pdTRUE){
			
			uxSemaphoreCount = uxSemaphoreGetCount(xCountSem_Buffer);
			
			sprintf(buffer, "Consumer: Dang xu ly... Con lai: %ld\r\n", (uint32_t)uxSemaphoreCount);
			UART_std_print(buffer);
		}else{
			UART_std_print("Consumer: Khong co du lieu, dang cho...\r\n");
		}
	}
}

int main(void){
	SystemInit();
	SystemCoreClockUpdate();
	UART_Config();
	
	// Tao Counting Semaphore: Max = 5, Initial = 0
	xCountSem_Buffer = xSemaphoreCreateCounting(5, 0);
	
	// Tao Task
	xTaskCreate(TaskProducer, "Producer", 256, NULL, 2, NULL);
	xTaskCreate(TaskConsumer, "Consumer", 256, NULL, 2, NULL);
	
	vTaskStartScheduler();
	while(1);
}
			