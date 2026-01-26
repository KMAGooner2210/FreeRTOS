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
<details>
    <summary><strong>BÀI 2: TASK CREATION, PRIORITES, SCHEDULING</strong></summary>

## **BÀI 2: TASK CREATION, PRIORITES, SCHEDULING**

### **I. Task Creation**

#### **1.1. Định nghĩa**

*   `xTaskCreate()` là API cốt lõi dùng để:

    ◦   Tạo task mới

    ◦   Cấp phát stack

    ◦   Khởi tạo TCB (Task Control Block)

    ◦   Đưa task vào hệ thống scheduler

#### **1.2. Prototype**

        BaseType_t xTaskCreate(
            TaskFunction_t pvTaskCode,
            const char * const pcName,
            uint16_t usStackDepth,
            void *pvParameters,
            UBaseType_t uxPriority,
            TaskHandle_t *pxCreatedTask
        );

*   `pvTaskCode`

    ◦   Con trỏ tới hàm task

    ◦   Task là hàm C không bao giờ kết thúc

    ◦   Thường chứa vòng lặp `f`or(;;)`

*   `pcName` 

    ◦   Tên mô tả task

    ◦   Độ dài tối đa: `configMAX_TASK_NAME_LEN`

    ◦   Chuỗi dài hơn sẽ bị cắt tự động

*   `usStackDepth`

    ◦   Kích thước stack của task

    ◦   Đơn vị là word, không phải byte

*   `pvParameters` 

    ◦   Tham số truyền vào task

    ◦   Kiểu void*

    ◦   Dùng để: Truyền cấu hình, ID, struct

*   `uxPriority` 

    ◦   Mức ưu tiên của task

    ◦   Phạm vi: 0 - `configMAX_PRIORITIES - 1`

*   `pxCreatedTask` 

    ◦   Con trỏ nhận handle của task

    ◦   Dùng để: Thay đổi priority, delete task, suspend/resume

    ◦   Không cần dùng → set NULL

#### **1.3. Giá trị trả về**

*   **pdPASS:** Tạo task thành công 

*   **pdFAIL:** Không đủ heap để cấp phát stack & TCB

        void vTaskLedToggle(void *pvParameters)
        {
            uint16_t pin = (uint16_t)pvParameters;          // Ép kiểu tham số

            for(;;)
            {
                GPIOC->ODR ^= pin;                              // Toggle pin cụ thể
                vTaskDelay(pdMS_TO_TICKS(300 + pin * 100));     // Delay khác nhau
            }
        }

### **II. Priorities and Scheduling Mechanisms**

#### **2.1. Mô hình lập lịch của FreeRTOS**

*   FreeRTOS sử dụng fixed-priority preemptive scheduling, trong đó:

    ◦   Mỗi task được gán một mức ưu tiên cố định tại thời điểm tạo

    ◦   Scheduler luôn chọn task Ready có priority cao nhất để thực thi

    ◦   Khi một task ưu tiên cao hơn chuyển sang trạng thái Ready, nó sẽ ngắt (preempt) ngay lập tức task đang Running có priority thấp hơn

#### **2.2. Ready List – Cấu trúc dữ liệu lập lịch**

##### **2.2.1. Khái niệm**

*   Ready list là tập hợp các task đang ở trạng thái Ready (sẵn sàng chạy)

*   Kernel duy trì một Ready list cho mỗi mức priority

*   Tổng số mức priority được xác định bởi: `configMAX_PRIORITIES`

        configMAX_PRIORITIES = 5
        Priority: 0 1 2 3 4

##### **2.2.2. Cách FreeRTOS tổ chức Ready list**

*   Tùy cấu hình, FreeRTOS sử dụng:

    ◦   Array of linked lists

        Mỗi priority → một danh sách liên kết các task

    ◦   Priority bitmap

        Một bitmask cho biết priority nào hiện đang có task Ready


#### **2.3. Cơ chế Preemption**

##### **2.3.1. Khái niệm**

*   Preemption xảy ra ngay lập tức khi:

    ◦   Một task có priority cao hơn  (Được unblocked/resume)

    ◦   ISR đánh thức task priority cao

*   Điều kiện:

        configUSE_PREEMPTION == 1

##### **2.3.2. Đặc điểm**

*   Không cần chờ tick interrupt

*   Context switch xảy ra:

    ◦   Ngay trong API call

    ◦   Hoặc ngay khi ISR kết thúc (`portYIELD_FROM_ISR()`)

#### **2.4. Time Slicing**

##### **2.4.1. Điều kiện kích hoạt**

*   Time-slicing chỉ áp dụng cho các task cùng priority, khi:

        configUSE_TIME_SLICING == 1

##### **2.4.2. Cơ chế hoạt động**

*   Mỗi tick interrupt:

    ◦   Scheduler xoay vòng các task Ready cùng priority

    ◦   Task hiện tại bị đưa về cuối Ready list

    ◦   Task tiếp theo được chạy

*   Time-slicing không có khi:

    ◦   `configUSE_TIME_SLICING = 0`

    ◦   Hoặc chỉ có một task tại mức priority đó

    ◦   Task sẽ chạy liên tục cho đến khi: `Block`, `Yield`, preempt bởi task priority cao hơn

#### **2.5. VD**

        #include "FreeRTOS.h"
        #include "task.h"
        #include "stm32f10x.h"

        /* ===== Hardware Init ===== */
        void LED_Init(void)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

            GPIO_InitTypeDef gpio;
            gpio.GPIO_Pin   = GPIO_Pin_13;
            gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
            gpio.GPIO_Speed = GPIO_Speed_2MHz;
            GPIO_Init(GPIOC, &gpio);

            GPIOC->BSRR = GPIO_Pin_13; // LED OFF (PC13 active low)
        }

        /* ===== Task 1: High Priority (Real-time) ===== */
        void vTaskCritical(void *pvParameters)   // Priority 4
        {
            for (;;)
            {
                GPIOC->BRR = GPIO_Pin_13;   // LED ON
                vTaskDelay(pdMS_TO_TICKS(50));

                GPIOC->BSRR = GPIO_Pin_13;  // LED OFF
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        /* ===== Task 2: Medium Priority (CPU Heavy) ===== */
        void vTaskHeavy(void *pvParameters)       // Priority 2
        {
            for (;;)
            {
                for (volatile uint32_t i = 0; i < 100000; i++)
                {
                    __NOP();   // giả lập xử lý nặng ~100 ms
                }

                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }

        /* ===== Task 3: Low Priority (Background) ===== */
        void vTaskBackground(void *pvParameters)  // Priority 1
        {
            for (;;)
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        /* ===== Main ===== */
        int main(void)
        {
            SystemInit();
            LED_Init();

            xTaskCreate(vTaskCritical,
                        "Critical",
                        256,
                        NULL,
                        4,
                        NULL);

            xTaskCreate(vTaskHeavy,
                        "Heavy",
                        256,
                        NULL,
                        2,
                        NULL);

            xTaskCreate(vTaskBackground,
                        "Background",
                        128,
                        NULL,
                        1,
                        NULL);

            vTaskStartScheduler();

            /* Không bao giờ chạy tới đây */
            while (1);
        }



### **III. Time Measurement and Tick Interrupt**

#### **3.1. Khái niệm**

*   FreeRTOS không đo thời gian bằng timer độc lập cho từng task, mà sử dụng một nguồn thời gian toàn cục gọi là system tick.

    ◦   System tick được tạo bởi tick interrupt

    ◦   Mỗi tick đại diện cho một đơn vị thời gian nhỏ nhất mà kernel có thể quản lý

*   Mọi khái niệm liên quan đến: delay, timeout, sleep, scheduling đều quy đổi quy đổi về số tick

#### **3.2. Tick Interrupt**

*   **SysTick (Cortex-M):**

    ◦   SysTick là timer 24-bit tích hợp trong Cortex-M

    ◦   Được thiết kế riêng để: tạo interrupt định kỳ, phục vụ hệ điều hành thời gian thực

*   **vai trò: Mỗi lần xảy ra tick interrupt, FreeRTOS sẽ:**

    ◦   Tăng bộ đếm thời gian toàn cục

        xTickCount++;

    ◦   Cập nhật trạng thái các task bị Blocked

        Giảm timeout

        Nếu timeout = 0 → chuyển sang Ready

    ◦   Kiểm tra điều kiện lập lịch

        Có task Ready priority cao hơn không

        Có cần time-slicing không        

    ◦   Yêu cầu context switch nếu cần

        Thông qua PendSV

#### **3.3. Tần số tick**

*   **Định nghĩa:**

        #define configTICK_RATE_HZ   1000

    ◦   Xác định số tick trong 1 giây

    ◦   Là thông số quan trọng nhất liên quan đến thời gian

*   **Chu kỳ tick:**

        #define portTICK_PERIOD_MS (1000 / configTICK_RATE_HZ)
        
    | configTICK_RATE_HZ | portTICK_PERIOD_MS | Ghi chú |
    |--------------------|--------------------|--------|
    | 100                | 10 ms              | Low-power, delay thô |
    | 1000               | 1 ms               | Khuyến nghị cho hệ real-time |
    | 10000              | 0.1 ms             | Độ chính xác cao, overhead lớn |


    ◦   Độ phân giải thời gian = chu kỳ tick

#### **3.4. Tick và cơ chế delay / timeout**

*   **Delay theo tick:**

        vTaskDelay( xTicksToDelay );

    ◦   Task bị đưa vào trạng thái Blocked

    ◦   Kernel ghi nhận:

        wake_up_tick = xTickCount + xTicksToDelay;

    ◦   Mỗi tick: So sánh xTickCount, nếu đến thời điểm → task chuyển Ready

*   **Độ chính xác của delay:**

    ◦   vTaskDelay() không đảm bảo chính xác tuyệt đối

    ◦   Sai số có thể lên tới: ± 1 tick

#### **3.5. Chuyển đổi ms ↔ tick**

        pdMS_TO_TICKS( ms )

*   **VD:**

        vTaskDelay( pdMS_TO_TICKS(100) );  // Delay 100 ms

*   Với `configTICK_RATE_HZ = 1000`:

        100 ms → 100 tick

#### **3.6. Lưu ý**

*   Tần số tick càng cao:

    ◦   SysTick interrupt càng nhiều

    ◦   Context switch xảy ra thường xuyên hơn

    ◦   CPU mất thời gian vào:

        ISR

        Scheduler

        PendSV    

*   Tick quá cao có thể:

    ◦   Giảm thời gian chạy task thật

    ◦   Tăng tiêu thụ năng lượng

    ◦   Gây jitter nếu ISR dài
    
     </details> 
