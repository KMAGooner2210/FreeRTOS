


# TH1: KHÔNG DÙNG CRITICAL


		
		// Task A: Tăng Counter
		void TaskA(void *pvParameters){
			uint32_t temp;
			while(1){
				temp = g_Counter;  // B1:Đọc giá trị
				vTaskDelay(1);		// B2:Tạo khoảng trống
				g_Counter = temp + 1; 	// B3: Ghi giá trị
				}
		}
		
		// Task B: Giảm Counter
		void TaskB(void *pvParameters){
			uint32_t temp;
			while(1){
				temp = g_Counter;		// B1:Đọc giá trị
				vTaskDelay(1);			// B2: Tạo khoảng trống
				g_Counter = temp = 1;		// B3: Ghi giá trị
				}
		}

* **Output:**

		g_Counter: 0
		g_Counter: 1013
		g_Counter: 2028
		g_Counter: 3043
		g_Counter: 4058
		g_Counter: 5073
		g_Counter: 6088
		g_Counter: 7103
		g_Counter: 8118


# TH2: DÙNG CRITICAL
		
		// Task A: Tăng counter
		void TaskA(void *pvParameters){
			(void)pvParameters;
			while(1){
				taskENTER_CRITICAL();
				g_Counter++;
				taskEXIT_CRITICAL();
				vTaskDelay(10 / portTICK_RATE_MS);
				}
		 }

		 // Task B: Giảm counter
		 void TaskB(void *pvParameters){
			 (void)pvParameters;
			 while(1){
				 taskENTER_CRITICAL();
				 g_Counter--;
				 taskEXIT_CRITICAL();
				 vTaskDelay(10 / portTICK_RATE_MS);
				 }
		 }
		
		

* **Output:**

		g_Counter: 0
		g_Counter: 0
		g_Counter: 0
		g_Counter: 0
		g_Counter: 0
