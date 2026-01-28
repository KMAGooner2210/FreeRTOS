* **Bài 1: Dual LED Blinking**

    •	Mô tả: Tạo 2 task điều khiển 2 đèn LED khác nhau.

        o	Task 1: Chớp tắt LED 1 chu kỳ 1000ms.

        o	Task 2: Chớp tắt LED 2 chu kỳ 500ms.

    •	Yêu cầu: Sử dụng API xTaskCreate để tạo task và vTaskDelay để tạo độ trễ.

* **Bài 2: UART Logger**

    •	Mô tả: Tạo 2 task cùng gửi dữ liệu qua UART lên máy tính

        o	Task A: In chuỗi "Task A is running..." mỗi 1s.

        o	Task B: In chuỗi "Task B is running..." mỗi 2s.

    •	Yêu cầu: Sử dụng vTaskDelay để định thời gian.

* **Bài 3: Generic LED Task**

    •	Mô tả: Chỉ viết 01 hàm task duy nhất (VD: vLedTaskCode).

        o	Tạo ra 2 Task (Green, Red) từ cùng hàm vLedTaskCode đó.

        o	Truyền tham số (số Pin LED và thời gian delay) qua pvParameters.

* **Bài 4: Preemption Demo**

    •	Mô tả: Cấu hình configUSE_PREEMPTION = 1

        o	Task A (Prio 1) đang chạy một vòng lặp tính toán dài

        o	Tạo Task B (Prio 2) thức dậy (sau delay).

    •	Yêu cầu: 

        o	Sử dụng UART hoặc GPIO (logic analyzer) để ghi thời điểm: Chứng minh Task A bị ngắt ngay khi Task B thức dậy, dù chưa xong tính toán.

        o	Thử nghiệm: Tắt preemption (configUSE_PREEMPTION = 0) và so sánh (Task A chạy xong mới nhường).

* **Bài 5: Timing**

    •	Mô tả: Tạo hai task với cùng chu kỳ 1000ms nhưng sử dụng delay khác nhau

        o	Tạo Task 1 dùng vTaskDelay(1000): Trong vòng lặp while, chèn một đoạn code giả lập tốn 200ms xử lý (dummy loop).

        o	Tạo Task 2 dùng vTaskDelayUntil(..., 1000): Cũng chèn đoạn code tốn 200ms xử lý tương tự.

    •	Yêu cầu: 

        o	Sử dụng xTaskGetTickCount() để in thời điểm bắt đầu mỗi chu kỳ qua UART.

* **Bài 6: The Manager**

    •	Mô tả: Xây dựng hệ thống với nút nhấn để điều khiển vòng đời của một task worker.

        o	Task Worker: In dòng "Working..." và nháy LED liên tục

        o	Task KeyInput: Đọc nút nhấn.

    •	Yêu cầu:

        o	Nhấn nút lần 1: vTaskSuspend để tạm dừng Worker (LED giữ trạng thái hiện tại).

        o	Nhấn nút lần 2: Gọi vTaskResume để Task Worker chạy tiếp.

        o	Nhấn giữ 3s: Gọi vTaskDelete để xóa sổ Task Worker (LED tắt hẳn).

* **Bài 7: One-Shot Task & Memory**

    •	Mô tả: Tạo task khởi động chạy một lần rồi tự hủy để tiết kiệm tài nguyên.

        o	Trong main: Tạo StartupTask.

        o	StartupTask: In logo khởi động, init biến, chớp LED 3 lần báo ready, rồi vTaskDelete(NULL).

    •	Yêu cầu:

        o	Sử dụng uxTaskGetNumberOfTasks() để in số task trước/sau delete qua UART.
