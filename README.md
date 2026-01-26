# CHƯƠNG 4: TASK MANAGEMENT 
<details>
    <summary><strong>BÀI 1: TASK INTRODUCTION, FUNCTIONS & STATES</strong></summary>

## **BÀI 1: TASK INTRODUCTION, FUNCTIONS & STATES**

### **I. Introduction**

*   Trong FreeRTOS, task (tác vụ) là đơn vị thực thi cơ bản nhất – tương đương với một luồng thực thi (thread) trong hệ điều hành thông thường

*   Vai trò task:

    ◦   Mỗi task có code riêng (hàm task), stack riêng và ngữ cảnh riêng.

    ◦   Kernel phân bổ CPU time, chọn task chạy dựa trên ưu tiên và trạng thái.

    ◦   Cho phép đa nhiệm đồng thời trên single-core bằng context switch.

### **II. Super loop**

#### **2.1. Định nghĩa**

*   Super loop là một vòng lặp vô hạn trong main(), liên tục thực hiện các chức năng theo thứ tự cố định.

*   Không có khái niệm task, priority, hay preemption – mọi thứ chạy tuần tự (sequential).

*   Thời gian thực hiện phụ thuộc vào:

    ◦   Thứ tự code trong loop.

    ◦   Thời gian mỗi chức năng mất (có thể dùng delay để "chờ").

    ◦   Polling (liên tục kiểm tra điều kiện, ví dụ đọc button hoặc sensor).

#### **2.2. Nguyên lý cơ bản**

*   Khởi tạo hardware (GPIO, UART, ADC...) một lần ở `main()`.

*   Sau đó vào vòng lặp vô hạn: thực hiện công việc 1 → công việc 2 → ... → công việc n → quay lại công việc 1.

*   Để tránh một công việc block toàn bộ hệ thống, thường dùng non-blocking (không delay lâu) hoặc state machine cho từng chức năng.

#### **2.3. Cấu trúc**


        int main(void)
        {
            // 1. Khởi tạo hardware (chạy 1 lần)
            System_Init();      // Clock, peripheral...
            GPIO_Init();
            UART_Init();
            // ...

            while(1)  // Super loop – vòng lặp vô hạn
            {
                // 2. Thực hiện các công việc tuần tự
                Task1();  // Ví dụ: đọc sensor
                Task2();  // Ví dụ: xử lý dữ liệu
                Task3();  // Ví dụ: gửi UART
                Task4();  // Ví dụ: blink LED

                // Có thể thêm delay nhỏ ở cuối loop để giảm tốn CPU
                // Delay_ms(1);
            }
        }

#### **2.4. Nhược điểm**

*   Không đảm bảo real-time

    ◦   Nếu một công việc mất thời gian lâu (ví dụ Task2 xử lý dữ liệu nặng), các công việc sau (Task3, Task4) bị delay → miss deadline.

*   Khó mở rộng

    ◦   Khi thêm chức năng mới → loop dài hơn → thời gian cycle tăng → tất cả chức năng chậm lại.

    ◦   Phải dùng state machine phức tạp để tránh blocking

*   Xử lý ngắt kém

    ◦   Ngắt (ISR) có thể xử lý nhanh, nhưng dữ liệu từ ngắt thường phải buffer và xử lý trong loop → nếu loop bận → mất dữ liệu.

*   Không có priority

    ◦   Không phân biệt công việc quan trọng và không quan trọng

    ◦   Không preemption → công việc thấp không thể bị chiếm bởi công việc cao ưu tiên.

### **III. Task states**

#### **3.1. Định nghĩa**

*   FreeRTOS định nghĩa 4 trạng thái chính cho mọi task (đây là cách kernel theo dõi và quản lý task):

    ◦   Running

    ◦   Ready

    ◦   Blocked

    ◦   Suspended

#### **3.2. Đặc điểm**

*   **Running:**

    ◦   Task đang chiếm quyền điều khiển CPU và code của nó đang được thực thi.

    ◦   Trên hệ thống single-core (hầu hết MCU nhúng): chỉ có đúng 1 task ở trạng thái Running tại một thời điểm.

    ◦   Task running có thể:

        Tự nguyện nhường CPU (gọi vTaskDelay(), taskYIELD(), hoặc blocking API như queue receive)

        Bị preempt (bị cướp CPU) bởi task có ưu tiên cao hơn hoặc bởi ngắt (ISR).


*   **Ready:**

    ◦   Task đã sẵn sàng để chạy (không bị chặn, không bị tạm dừng), nhưng chưa được chọn để chạy vì hiện tại có task ưu tiên cao hơn (hoặc bằng) đang Running.

    ◦   Các task Ready được kernel lưu trong ready list (danh sách sẵn sàng), được sắp xếp theo ưu tiên (priority descending).

    ◦   Scheduler luôn chọn task có ưu tiên cao nhất trong ready list để đưa vào Running → đây là cơ sở của preemptive priority-based scheduling.

*   **Blocked:**

    ◦   Task đang chờ một sự kiện (event) và không thể chạy ngay được.

    ◦   Các trường hợp phổ biến gây Blocked:

        Đang delay: vTaskDelay() hoặc vTaskDelayUntil().

        Đang chờ nhận dữ liệu từ queue: xQueueReceive() (với timeout ≠ 0).

        Đang chờ semaphore/mutex: xSemaphoreTake().

        Đang chờ bit event group: xEventGroupWaitBits().

    ◦   Task Blocked không tiêu tốn CPU

    ◦   Khi sự kiện xảy ra (timeout hết), task tự động chuyển từ Blocked->Ready   

*   **Suspended:**

    ◦   Task bị tạm dừng chủ động bởi code ứng dụng (thường do task khác hoặc chính nó gọi `vTaskSuspend()`.

    ◦   Không chờ sự kiện gì cả → không tự động tỉnh lại (khác với Blocked).

    ◦   Chỉ trở lại Ready khi có lời gọi rõ ràng: `vTaskResume()` hoặc `xTaskResumeFromISR()`.

    ◦   Thường dùng để: tạm dừng task khi debug, chuyển sang chế độ low-power, hoặc điều khiển runtime

        Đang delay: vTaskDelay() hoặc vTaskDelayUntil().

        Đang chờ nhận dữ liệu từ queue: xQueueReceive() (với timeout ≠ 0).

        Đang chờ semaphore/mutex: xSemaphoreTake().

        Đang chờ bit event group: xEventGroupWaitBits().

    ◦   Task Blocked không tiêu tốn CPU

    ◦   Khi sự kiện xảy ra (timeout hết), task tự động chuyển từ Blocked->Ready

#### **3.3. State Transition Diagram**

<img width="502" height="536" alt="Image" src="https://github.com/user-attachments/assets/da3f7ad0-45f0-41ce-91a3-615e2bc38b67" />

*   **Created -> Ready:** Sau khi `xTaskCreate()` thành công

*   **Ready -> Running:** Scheduler chọn task có ưu tiên cao nhất trong ready list

*   **Running -> Ready:** Bị preempt bởi task ưu tiên cao hơn, hoặc gọi `taskYIELD()`

*   **Running -> Blocked:** Gọi blocking API

*   **Blocked -> Ready:** Sự kiện xảy ra hoặc timeout hết

*   **Running -> Suspended:** Gọi `vTaskSuspend()` (có thể từ task khác hoặc chính nó)

*   **Suspended -> Ready:** Gọi `vTaskResume()`

*   **Bất kỳ trạng thái nào -> Deleted:** Gọi `vTaskDelete()` hoặc task return

     </details> 
