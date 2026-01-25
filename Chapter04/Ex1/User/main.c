/* 
   Bai 1: Dual LED Blinking
   Muc tieu: Tao 2 task doc lap dieu khien 2 LED voi chu ky khác nhau
   STM32F103 (Blue Pill)
   - LED1: PC13 
   - LED2: PA5  
*/

#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h" 
#include "task.h"

void GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIOx->ODR ^= GPIO_Pin;
}

// Task1: Blink LED1 (PC13) , Chu ky: 1000 ms

void vTaskLed1(void *pvParameters){
	for(;;)
	{
		GPIO_TogglePin(GPIOC, GPIO_Pin_13);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

// Task2: Blink LED2 (PA5) , Chu ky: 500 ms

void vTaskLed2(void *pvParameters){
	for(;;)
	{
		GPIO_TogglePin(GPIOA, GPIO_Pin_5);
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}

int main(){
	SystemInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
	
	xTaskCreate(vTaskLed1, "LED1", 128, NULL, 2, NULL);
	xTaskCreate(vTaskLed2, "LED2", 128, NULL, 2, NULL);
	
	vTaskStartScheduler();
	
	for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName){
	while(1);
}
