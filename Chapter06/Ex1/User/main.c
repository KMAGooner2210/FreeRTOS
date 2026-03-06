#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"


volatile uint32_t g_Counter = 0;


void UART_Print(const char *msg){
    while(*msg){
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *msg++);
    }
}

//void TaskA(void *pvParameters){
//    uint32_t temp;
//    while(1){
//        temp = g_Counter;
//        vTaskDelay(1); 
//        g_Counter = temp + 1;
//    }
//}

//void TaskB(void *pvParameters){
//    uint32_t temp;
//    while(1){

//        temp = g_Counter;
//        vTaskDelay(1);
//        g_Counter = temp - 1;
//    }
//}

// Task A: Tang counter
void TaskA(void *pvParameters){
    (void)pvParameters;
    while(1){
        taskENTER_CRITICAL(); 
        g_Counter++;
        taskEXIT_CRITICAL();
        

        vTaskDelay(10 / portTICK_RATE_MS); 
    }
}

// Task B: Giam counter
void TaskB(void *pvParameters){
    (void)pvParameters;
    while(1){
        taskENTER_CRITICAL(); 
        g_Counter--;
        taskEXIT_CRITICAL();
        
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void TaskMonitor(void *pvParameters){
    (void)pvParameters;
    char buffer[64];
    while(1){
        sprintf(buffer, "g_Counter: %ld\r\n", (int32_t)g_Counter);
        UART_Print(buffer);
        vTaskDelay(1000 / portTICK_RATE_MS);
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
}

int main(void){
    SystemInit(); 
    SystemCoreClockUpdate();
    
    UART_Config();
    
    xTaskCreate(TaskA, "TaskA", 128, NULL, 2, NULL);
    xTaskCreate(TaskB, "TaskB", 128, NULL, 2, NULL);
    xTaskCreate(TaskMonitor, "TaskMonitor", 128, NULL, 1, NULL);
    
    vTaskStartScheduler();
    
    while(1);
}
