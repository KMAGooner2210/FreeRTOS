* **Bài 1: Cuộc đua biến đếm (Race Condition)**

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
