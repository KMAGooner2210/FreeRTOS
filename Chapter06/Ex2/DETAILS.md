


# TH1: Dùng vTaskSuspend, xTaskResume

      void UART_std_print(const char * msg){
        vTaskSuspendAll();
        UART_Print(msg);
        vTaskResumeAll();
      }
		
  		// Task 1:
  		void Task1(void *pvParameters){
        while(1){
          if(isPrinting){
          UART_std_print("TASK1 IS RUNNING.............\r\n");
          }
          vTaskDelay(100 / portTICK_RATE_MS);
          }
  		}
  		
  		// Task 2:
  		void Task2(void *pvParameters){
        while(1){
          if(isPrinting){
          UART_std_print("TASK2 IS INTERRUPTING.............\r\n");
          }
          vTaskDelay(100 / portTICK_RATE_MS);
          }
  		}

* **Output:**

<img width="478" height="498" alt="image" src="https://github.com/user-attachments/assets/63a76115-8269-4323-8d82-3b1b74c73637" />



# TH2: Không sử dụng vTaskSuspend, xTaskResume

    void UART_std_print(const char * msg){
    UART_Print(msg);
    }

  		// Task 1:
  		void Task1(void *pvParameters){
        while(1){
          if(isPrinting){
          UART_std_print("TASK1 IS RUNNING.............\r\n");
          }
          vTaskDelay(100 / portTICK_RATE_MS);
          }
  		}
  		
  		// Task 2:
  		void Task2(void *pvParameters){
        while(1){
          if(isPrinting){
          UART_std_print("TASK2 IS INTERRUPTING.............\r\n");
          }
          vTaskDelay(100 / portTICK_RATE_MS);
          }
  		}
		
		

* **Output:**

<img width="592" height="201" alt="image" src="https://github.com/user-attachments/assets/861b38d7-2e4e-4ef3-997c-f8b7f3b73574" />

		g_Counter: 0
		g_Counter: 0
		g_Counter: 0
