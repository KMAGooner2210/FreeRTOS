* **Bài 1: Race Condition**

  * **Mục tiêu:** Quan sát biến toàn cục bị sai lệch khi không có bảo vệ.

  * **Tài nguyên:** Biến toàn cục volatile uint32_t g_Counter = 0;

  * **Thực hiện:**

    *  Task A (Priority Normal): Vòng lặp while(1), thực hiện g_Counter++.
    *  Task B (Priority Normal): Vòng lặp while(1), thực hiện g_Counter--.
    *  Task Monitor (Priority Low): Mỗi 1s in giá trị g_Counter qua UART.


  * **Khi không sử dụng Critical:**

    *  Để dễ thấy lỗi, chèn vTaskDelay(1) hoặc một vòng lặp for rỗng ngắn giữa lệnh đọc và ghi trong phép cộng/trừ.
      
    *  Quan sát UART: Giá trị sẽ trôi đi rất xa (ví dụ: 500, -1200...) thay vì dao động quanh 0.
  
  * **Yêu cầu sửa lỗi:**
  
    *  Sử dụng taskENTER_CRITICAL() và taskEXIT_CRITICAL() bao bọc lệnh cộng/trừ.

    *  Kết quả mong đợi: Giá trị in ra luôn xấp xỉ 0.

* **Bài 2: Scheduler Suspension**

  * **Mục tiêu:** Ngăn chặn hiện tượng các ký tự bị xen lẫn khi nhiều Task cùng in.

  * **Thực hiện:**

    *  Task 1: In chuỗi dài: TASK1 IS RUNNING.............

    *  Task 2: In chuỗi dài: TASK2 IS INTERRUPTING.............

    *  Cả 2 task chạy song song với vTaskDelay(100) sau mỗi lần in.

  * **Hiện tượng:**

    *  Output sẽ bị loạn
  
  * **Yêu cầu sửa lỗi:**
  
    *  Dùng vTaskSuspendAll() trước khi gọi UART_SendString và xTaskResumeAll() sau khi in xong.

* **Bài 3: Deferred Interrupt Processing**

  * **Mục tiêu:** Kỹ thuật chuẩn để xử lý ngắt trong RTOS (ISR chỉ báo hiệu, Task thực thi)

  * **Phần cứng:** Nút nhấn nối PA0 (hoặc PB1), cấu hình EXTI (External Interrupt) kích hoạt cạnh xuống/lên.

  * **Yêu cầu:**

    *  1. Tạo Binary Semaphore: xBinSem_Button.

    *  2. Trong hàm ngắt EXTI0_IRQHandler
     
          Kiểm tra cờ ngắt: if(EXTI_GetITStatus(EXTI_Line0) != RESET).
          
          Xóa cờ ngắt: EXTI_ClearITPendingBit(EXTI_Line0).
          
          Gọi: xSemaphoreGiveFromISR(xBinSem_Button, &xHigherPriorityTaskWoken).
          Thực hiện portEND_SWITCHING_ISR(...).

    *  3. Trong Task xử lý:
     
          Gọi xSemaphoreTake(xBinSem_Button, portMAX_DELAY).
       
          Khi nhận được Semaphore: Đảo trạng thái LED PC13 dùng GPIO_WriteBit(GPIOC, GPIO_Pin_13, ...) và in "Button Pressed" ra UART.
  


* **Bài 4: Counting Semaphore**

  * **Mục tiêu:** Mô phỏng Producer-Consumer.

  * **Yêu cầu:**

    *  Tạo Counting Semaphore: xCountSem_Buffer (Max = 5, Init = 0).

    *  Task Producer: Giả lập cứ 500ms tạo ra 1 gói dữ liệu (Give Semaphore).

    *  Task Consumer: Cố gắng lấy dữ liệu để xử lý (Take Semaphore).
    
          Nếu Take thành công: In "Đang xử lý gói tin... Còn lại: [giá trị sem]".

          Nếu Take thất bại (Block 2s): In "Không có dữ liệu, đang chờ...".

    *   Cho Producer chạy nhanh (200ms) để thấy Semaphore đầy dần (lên tới 5), sau đó cho Producer chạy chậm (1000ms) để thấy Semaphore cạn dần về 0.

* **Bài 5: Priority Inheritance**

  * **Mục tiêu:** Khắc phục lỗi Priority Inversion (Đảo ngược độ ưu tiên).

  * **Yêu cầu:**

    *  Tạo Mutex: xMutex_UART.

    *  Task Low (Prio 1): Take Mutex -> In dòng chữ rất dài (mô phỏng giữ tài nguyên lâu) -> Give Mutex.

    *  Task High (Prio 3): Chờ 1 chút rồi Take Mutex -> In dòng chữ ngắn -> Give Mutex.

    *  Task Medium (Prio 2): Chạy liên tục vòng lặp (làm nhiễu CPU).

  * **Thực hiện:**

    *  Trường hợp 1 (Dùng Binary Semaphore làm khóa): Task Medium sẽ chiếm CPU khi Task Low đang giữ khóa. Task High sẽ bị treo vĩnh viễn (hoặc rất lâu).

    *  Trường hợp 2 (Dùng Mutex): Khi Task High chờ Mutex đang bị Task Low giữ, FreeRTOS sẽ tạm thời nâng quyền Task Low lên bằng Task High. Task Low chạy nhanh chóng để nhả Mutex cho Task High.

* **Bài 6: Recursive Mutex**

  * **Mục tiêu:** Tránh Deadlock khi một Task cố khóa chính nó.

  * **Yêu cầu:**

    *  Viết hàm void LogData(char* msg): Trong hàm này Take Mutex, in msg, rồi Give Mutex.

    *  Viết hàm void SystemCheck(): Trong hàm này Take Mutex, thực hiện kiểm tra, sau đó gọi LogData("Checking..."), rồi Give Mutex.

  * **Vấn đề:**

    *  Khi SystemCheck gọi LogData, LogData lại cố Take Mutex mà SystemCheck đang giữ -> Deadlock.

  * **Khắc phục:**

    *  Sử dụng xSemaphoreCreateRecursiveMutex.

    *  Thay thế bằng xSemaphoreTakeRecursive và xSemaphoreGiveRecursive.

    *  Hệ thống phải chạy được mà không bị treo.

* **Bài 7: Deadlock**

  * **Mục tiêu:** Tạo ra Deadlock và tìm cách giải quyết.

  * **Setup:**

    *  2 Mutex: Mutex_A, Mutex_B.

    *  Task 1:

         xSemaphoreTake(Mutex_A, portMAX_DELAY);
         vTaskDelay(100); // Giả lập xử lý, tạo cơ hội cho Task 2 chạy
         xSemaphoreTake(Mutex_B, portMAX_DELAY); // Chờ mãi mãi ở đây

    * Task 2:

         xSemaphoreTake(Mutex_B, portMAX_DELAY);
         vTaskDelay(100);
         xSemaphoreTake(Mutex_A, portMAX_DELAY);


  * **Hiện tượng:**

    *  LED PC13 (đặt trong 1 task khác làm đèn báo) sẽ dừng nhấp nháy. Hệ thống treo.
   
  * **Giải pháp:**

    *  Yêu cầu cả 2 Task đều phải Take Mutex_A trước, rồi mới đến Mutex_B (Quy tắc thứ tự tài nguyên).

    *  Hoặc dùng xSemaphoreTake(..., timeout): Nếu quá 100ms không lấy được thì nhả Mutex đang giữ ra và thử lại sau.  
