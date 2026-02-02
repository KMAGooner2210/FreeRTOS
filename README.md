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

##### **3.2.1.Running**

*    **Đặc điểm:**

        ◦   Task đang chiếm quyền điều khiển CPU và code của nó đang được thực thi.
    
        ◦   Trên hệ thống single-core (hầu hết MCU nhúng): chỉ có đúng 1 task ở trạng thái Running tại một thời điểm.
    
        ◦   Task running có thể:
    
            Tự nguyện nhường CPU (gọi vTaskDelay(), taskYIELD(), hoặc blocking API như queue receive)
    
            Bị preempt (bị cướp CPU) bởi task có ưu tiên cao hơn hoặc bởi ngắt (ISR).

*    **VD:**

        void vLedTask(void *pvParameters)
        {
            while(1)
            {
                GPIOC->ODR ^= (1 << 13); // LED ON/OFF
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }

        ◦   Khi CPU đang thực thi dòng `GPIOC->ODR ^= ...`, vLedTask ở trạng thái Running
    
        ◦   Nếu có task khác priority cao hơn: LED task bị preempt, chuyển về Ready  
  
##### **3.2.2.Ready**

*    **Đặc điểm:**

        ◦   Task đã sẵn sàng để chạy (không bị chặn, không bị tạm dừng), nhưng chưa được chọn để chạy vì hiện tại có task ưu tiên cao hơn (hoặc bằng) đang Running.
    
        ◦   Các task Ready được kernel lưu trong ready list (danh sách sẵn sàng), được sắp xếp theo ưu tiên (priority descending).
    
        ◦   Scheduler luôn chọn task có ưu tiên cao nhất trong ready list để đưa vào Running → đây là cơ sở của preemptive priority-based scheduling.

*    **VD:**

        xTaskCreate(vLedTask,  "LED", 128, NULL, 1, NULL);
        xTaskCreate(vUartTask, "UART",256, NULL, 1, NULL);

        ◦   Cả hai: Không delay, block -> Ready
        
        ◦   Scheduler:  Chia CPU theo time slicing (nếu `configUSE_TIME_SLICING = 1`)
    
##### **3.2.3.Blocked**

*    **Đặc điểm:**
  
        ◦   Task đang chờ một sự kiện (event) và không thể chạy ngay được.
    
        ◦   Các trường hợp phổ biến gây Blocked:
    
            Đang delay: vTaskDelay() hoặc vTaskDelayUntil().
    
            Đang chờ nhận dữ liệu từ queue: xQueueReceive() (với timeout ≠ 0).
    
            Đang chờ semaphore/mutex: xSemaphoreTake().
    
            Đang chờ bit event group: xEventGroupWaitBits().
    
        ◦   Task Blocked không tiêu tốn CPU
    
        ◦   Khi sự kiện xảy ra (timeout hết), task tự động chuyển từ Blocked->Ready   

*    **VD1: Delay**

            vTaskDelay(pdMS_TO_TICKS(1000));
    
        ◦   Task chuyển sang Blocked 
    
        ◦   Không tiêu tốn CPU 

*    **VD2: Chờ dữ liệu UART (Queue)**

            xQueueReceive(uartQueue, &rxData, portMAX_DELAY);
    
        ◦   Task Blocked cho đến khi
    
            ISR UART nhận dữ liệu
    
            Gọi xQueueSendFromISR()
    
        ◦   Task Blocked → Ready → Running    
  
##### **3.2.4.Suspended:**

*    **Đặc điểm:**
  
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

*    **VD1: Tạm dùng task ADC**

            ADC task:Không delay, timeout, tự wake
    
        ◦   Chỉ wakeup khi:
    
            vTaskResume(adcTaskHandle);

    
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

        bitmap = 0b001011 (Có task Ready tại priority: 0, 1, 3)


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

##### **2.3.3. VD**

*   **USART ISR đánh thức task**

        xQueueSendFromISR(uartQueue, &data, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

*   Nếu:

    ◦   TaskUART priority = 3

    ◦   Task đang chạy priority = 1

    ◦   Context switch ngay khi thoát ISR, không cần đợi tick
    
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

*    Hai task cùng priority = 2
  
        xTaskCreate(TaskA, "A", 256, NULL, 2, NULL);
        xTaskCreate(TaskB, "B", 256, NULL, 2, NULL);

*    Nếu:

            #define configUSE_TIME_SLICING 1

*    Mỗi tick:

            Tick 1: A
            Tick 2: B
            Tick 3: A
            Tick 4: B



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

        SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);


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
  <details>
    <summary><strong>BÀI 3: TASK DELAY & IDLE TASK</strong></summary>

## **BÀI 3: TASK DELAY & IDLE TASK**

### **I. Task Delay**

#### **1.1. vTaskDelay**

*   `vTaskDelay()` là API dùng để đưa task vào trạng thái Blocked trong một khoảng thời gian tương đối, tính từ thời điểm task gọi hàm

*   **Prototype:**

        void vTaskDelay(TickType_t xTicksToDelay);

*   **Tham số: xTicksToDelay**

    ◦   Số tick task bị block

    ◦   Đơn vị: tick hệ thống

    ◦   Thường dùng macro quy đổi:

        vTaskDelay(pdMS_TO_TICKS(100));
        
    ◦   Lưu ý:

        vTaskDelay(0); -> Task nhường CPU (yield) nhưng không block   

*   **Cơ chế:**

    ◦   Khi task gọi:   `vTaskDelay(xTicksToDelay);`

    ◦   FreeRTOS sẽ:

        Lấy tick count hiện tại 

        Tính: `WakeTime = CurrentTick + xTicksToDelay`

        Đưa task vào Blocked list 

        Khi tick count đạt WakeTime -> Task chuyển sang ready      

*   **Chu kỳ thực tế:**

        for(;;)
        {
            DoWork();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    ◦   Chu kỳ thực tế:

        Chu kỳ = Thời gian xử lý + Thời gian delay

    ◦   VD:

        Xử lý 7 ms -> chu kỳ = 107 ms

*    **VD:**
  
        void vTaskLED(void *pvParameters)
        {
            for (;;)
            {
                GPIOC->BRR  = GPIO_Pin_13;   // LED ON
                vTaskDelay(pdMS_TO_TICKS(100));
        
                GPIOC->BSRR = GPIO_Pin_13;   // LED OFF
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }

#### **1.2. vTaskDelayUntil**

##### **1.2.1. Khái niệm**

*   `vTaskDelayUntil()` là API dùng để tạo task chạy định kỳ với chu kỳ chính xác, bằng cách delay task đến một mốc thời gian tuyệt đối trong tương lai, thay vì delay tương đối kể từ thời điểm gọi hàm 

*   **FreeRTOS sử dụng:**

    ◦   **System Tick:** bộ đếm tăng đều theo `configTICK_RATE_HZ`

    ◦   **Tick Count:** biến toàn cụ đếm số tick từ khi scheduler start

    ◦   Mọi cơ chế delay đều dựa trên tick count, không dựa trên thời gian thực tuyệt đối (RTC)
        
##### **1.2.2. Prototype**

        void vTaskDelayUntil( TickType_t *pxPreviousWakeTime,
                              TickType_t xTimeIncrement );


*   `pxPreviousWakeTime`

    ◦   Con trỏ tới biến lưu thời điểm task wake gần nhất

    ◦   Được dùng làm mốc thời gian tham chiếu tuyệt đối

    ◦   Kernel tự cập nhật giá trị này sau mỗi chu kỳ

    ◦   Khởi tạo một lần duy nhất, trước `for(;;)` của task

        xLastWakeTime = xTaskGetTickCount();

*   `xTimeIncrement` 

    ◦   Chu kỳ task

    ◦   Đơn vị: Tick

    ◦   Thường quy đổi từ ms:

        pdMS_TO_TICKS(period_ms)

*    **VD:**

            void vTaskADC(void *pvParameters)
            {
                TickType_t xLastWakeTime;
                const TickType_t period = pdMS_TO_TICKS(100);
            
                xLastWakeTime = xTaskGetTickCount();
            
                for (;;)
                {
                    Read_ADC();
                    Process_ADC_Data();
            
                    vTaskDelayUntil(&xLastWakeTime, period);
                }
            }

##### **1.2.3. Cơ chế**

*   Giả sử:

    ◦   `xLastWakeTime = T0`

    ◦   `xTimeIncrement = P`

*   Mỗi lần gọi

        vTaskDelayUntil(&xLastWakeTime, P);

*   FreeRTOS thực hiện:

    ◦   1. Tính thời điểm wake tiếp theo 

        NextWakeTime = xLastWakeTime + P

    ◦   2. So sánh với tick hiện tại

        Nếu CurrentTick < NextWakeTime -> Task bị đưa vào Blocked state

        Nếu CurrentTick >= NextWakeTime -> Task không bị block, chạy ngay

    ◦   3. Cập nhật

        xLastWakeTime = NextWakeTime

*   Mốc thời gian không bao giờ reset theo thời điểm gọi hàm

### **II. Idle Task và Idle Hook**

#### **2.1. Idle Task**

##### **2.1.1. Khái niệm**

*   Idle Task là task được kernel FreeRTOS tự động tạo khi scheduler start.

*   Nó đảm bảo rằng luôn có ít nhất một task ở trạng thái Ready, để scheduler không bao giờ rơi vào trạng thái không có gì để chạy.

##### **2.1.2. Đặc điểm**

*   Được kernel tự tạo, người dùng không cần và không được tạo

*   Priority: `Priority = 0`

*   Luôn tồn tại trong suốt vòng đời hệ thống

*   Chỉ chạy khi không có task Ready nào có priority > 0

*   Không thể:

    ◦   Xóa `vTaskDelete`

    ◦   Suspend `vTaskSuspend`

*   Idle task luôn phải được phép chạy, nếu không hệ thống sẽ lỗi.


#### **2.2. Vai trò**

*   Thu hồi tài nguyên task đã bị delete

    ◦   Khi gọi:   `vTaskDelete(taskHandle);`

        Task không bị xóa ngay lập tức 

        Tài nguyên chỉ được thu hồi khi Idle Task chạy

*   Đảm bảo scheduler luôn có task để chạy

    ◦   Scheduler luôn chọn task Ready có priority cao nhất

    ◦   Nếu không có task Ready:

        Idle task sẽ ready

        CPU không bị idle vô định nghĩa   

#### **2.2. Idle Hook**

##### **2.2.1. Khái niệm**

*   Idle Hook là một callback function do người dùng định nghĩa, được kernel gọi mỗi lần Idle Task chạy.

        void vApplicationIdleHook(void);

    ◦   Kernel sẽ gọi hàm này bên trong vòng lặp của Idle Task

##### **2.2.2. Điều kiện**

*   Idle Hook chỉ được gọi khi:

    ◦   `configUSE_IDLE_HOOK == 1` trong `FreeRTOSConfig.h`

    ◦    Scheduler đang chạy

    ◦    Không có task Ready nào khác

    
     </details> 
<details>
    <summary><strong>BÀI 4: TASK CHANGE & TASK DELETE</strong></summary>

## **BÀI 4: TASK CHANGE & TASK DELETE**

### **I. Changing Task Priority**

#### **1.1. Khái niệm**

*   Trong FreeRTOS, priority của task mặc định là cố định (fixed priority) và được thiết lập khi tạo task.

*   Kernel cho phép thay đổi priority tại runtime, nhằm đáp ứng các tình huống động của hệ thống.

#### **1.2. Nguyên tắc lập lịch**

*   FreeRTOS sử dụng priority-based preemptive scheduling (khi configUSE_PREEMPTION = 1):

    ◦   Scheduler luôn chọn task Ready có priority cao nhất

    ◦   Khi priority của một task thay đổi:

        Scheduler đánh giá lại ngay lập tức 

        Có thể xảy ra context switch tức thì

    ◦   Thay đổi priority là thao tác ảnh hưởng trực tiếp đến toàn bộ hệ thống scheduling. 

#### **1.3. Thay đổi Priority**

##### **1.3.1. Đặt Priority mới**

        void vTaskPrioritySet( taskHandle_t xTask,
                               UBaseType_t uxNewPriority);

*   **xTask:**

    ◦   Handle của task cần thay đổi priority

    ◦   Truyền NULL → thay đổi priority của task hiện tại

*   **uxNewPriority:**

    ◦   Priority mới

    ◦   Giá trị hợp lệ: `0 -> configMAX_PRIORITIES - 1`

            void vTaskWorker(void *pvParameters)
            {
                for (;;)
                {
                    if (urgent_event)
                    {
                        vTaskPrioritySet(NULL, 4);  // nâng priority task hiện tại
                    }
            
                    DoWork();
            
                    vTaskPrioritySet(NULL, 1);      // hạ priority sau khi xong
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }


##### **1.3.2. Hành vi kernel khi đổi priority**

*   **Tăng priority:**

    ◦   Nếu priority mới > task đang chạy

    ◦   Task có thể preempt ngay lập tức

*   **Giảm priority:**

    ◦   Task có thể bị preempt

    ◦   CPU được nhường cho task khác

*   Preempt xảy ra ngay sau khi gọi API, không cần chờ tick.

#### **1.4. Lấy priority hiện tại**

##### **1.4.1. Cú pháp**

        UBaseType_t uxTaskPriorityGet( TaskHandle_t xTask );

*   **Tham số:**

    ◦   **xTask:** 

        Handle của task cần đọc priority

        Truyền NULL → lấy priority của task hiện tại

*   **Giá trị trả về:**

    ◦   Priority hiện tại của task
        
### **II. Deleting Tasks**

#### **2.1. Khái niệm**

*   Trong FreeRTOS, task có thể được xóa (delete) trong quá trình runtime, khi task:

    ◦   Hoàn thành nhiệm vụ

    ◦   Không còn cần thiết

    ◦   Được quản lý bởi task khác (task manager)

*   Việc xóa task giúp:

    ◦   Giải phóng RAM (stack, TCB)

    ◦   Giảm tải scheduling

#### **2.2. vTaskDelete**

##### **2.2.1. Prototype**

        void vTaskDelete( TaskHandle_t xTaskToDelete );

##### **2.2.2. Tham số**

*   **xTaskToDelete:**

    ◦   Handle của task cần xóa

    ◦   Truyền NULL → task hiện tại tự xóa chính nó

##### **2.2.3. Cơ chế**

*   **1.**: Task bị loại khỏi:

    ◦   Ready list

    ◦   Blocked list

    ◦   Suspended list (nếu có)

*   **2.**: Task chuyển sang trạng thái Deleted

*   **3.**: Task không bao giờ được schedule lại

*   **4.**: Tài nguyên được xử lý:

    ◦   Dynamic allocation:

        Stack + TCB được giải phóng

        Nhưng chỉ khi Idle Task chạy

    ◦   Static allocation:

        Kernel không free

        Người dùng tự quản lý bộ nhớ    

*   Idle Task đóng vai trò thu hồi tài nguyên task bị delete


#### **2.3. Deleted State**

*   Không phải trạng thái chạy thực sự

*   Task:

    ◦   Không Ready

    ◦   Không Blocked

    ◦   Không Suspended 

*   Chỉ tồn tại tạm thời cho đến khi Idle Task cleanup xong

### **III. Thread-Local Storage (TLS)**

#### **3.1. Khái niệm**

*   Thread-Local Storage (TLS) cho phép mỗi task có vùng dữ liệu riêng, nhưng dùng chung một chỉ số (index).

*   Cùng một key, nhưng mỗi task giữ giá trị khác nhau

#### **3.2. Cấu hình**

        #define configNUM_THREAD_LOCAL_STORAGE_POINTERS 4

#### **3.3. API**

##### **3.3.1. Gán TLS cho task**

        void vTaskSetThreadLocalStoragePointer(
            TaskHandle_t xTask,
            BaseType_t xIndex,
            void *pvValue
        );


*   xTask: handle task (NULL → task hiện tại)

*   xIndex: chỉ số TLS (0 → max-1)

*   pvValue: con trỏ dữ liệu

*   **VD:**

            void vTaskA(void *pvParameters)
            {
                const char *taskName = "TaskA";
            
                vTaskSetThreadLocalStoragePointer(
                    NULL,
                    TLS_LOGGER_ID,
                    (void *)taskName
                );
            
                for (;;)
                {
                    Log("Hello");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
            }

##### **3.3.2. Lấy TLS**

        void *pvTaskGetThreadLocalStoragePointer(
            TaskHandle_t xTask,
            BaseType_t xIndex
        );


</details> 

# CHƯƠNG 5: QUEUE MANAGEMENT 
<details>
    <summary><strong>BÀI 1: FUNDAMETALS AND OPERATING PRINCIPLES OF QUEUES</strong></summary>

## **BÀI 1: FUNDAMETALS AND OPERATING PRINCIPLES OF QUEUES**

### **I. Giới thiệu**

#### **1.1. Khái niệm**

*   **Queue (hàng đợi)** là một trong những cơ chế đồng bộ hóa và giao tiếp giữa các task (**Inter Task Communication - ITC**)

*   **Queue** cho phép các task hoặc ngắt (ISR) trao đổi dữ liệu một cách an toàn, đồng bộ, và hiệu quả mà không gây race condition (tình trạng dữ liệu bị hỏng do truy cập đồng thời)

#### **1.2. Vai trò**

*   Trong hệ thống đa nhiệm, các task thường chạy độc lập nhưng cần trao đổi thông tin:

    ◦   Task A đọc cảm biến -> gửi dữ liệu cho Task B xử lý

    ◦   ISR nhận byte từ UART -> gửi cho task xử lý giao tiếp 

*   Nếu dùng biến toàn cục để chia sẻ dữ liệu:

    ◦   Dễ xảy ra race condition (VD: Task A đang ghi biến thì Task B đọc -> dữ liệu cũ mới lẫn lộn)

    ◦   Cần thêm mutex/semaphore để bảo vệ

*   Queue giúp giải quyết triệt để các vấn đề nhờ:

    ◦   Sao chép dữ liệu thay vì chia sẻ tham chiếu dẫn tới mỗi bên có bản sao riêng

    ◦   Kernel FreeRTOS bảo vệ toàn bộ thao tác enqueue/dequeue 

    ◦   Task có thể chờ dữ liệu hoặc chỗ trống  giúp tiết kiệm CPU 

    ◦   Đảm bảo thứ tự dữ liệu logic (FIFO)

#### **1.3. Phạm vi**

*   Queue được thiết kế linh hoạt cho nhiều tình huống:

    ◦   **Task-To-Task:** Producer gửi dữ liệu -> Consumer nhận

        VD: Task cảm biến -> Task xử lý

    ◦   **ISR-To-Task:** ISR gửi dữ liệu nhanh vào queue -> task xử lý chậm hơn 

    ◦   **Task-To-ISR:** Do ISR không được phép block, cơ chế này chỉ được áp dụng cho việc truyền tín hiệu hoặc trạng thái điều khiển đơn giản

    ◦   **Multiple producers / consumers:** Nhiều task gửi vào cùng queue, nhiều task nhận từ cùng queue

    ◦   **Truyền dữ liệu kích thước cố định:** Kích thước phần tử được xác định ngay khi tạo Queue, mỗi lần gửi nhận thì kernel copy đúng số byte cố định

### **II. Đặc điểm**

#### **2.1. Lưu trữ dữ liệu**

*   Queue lưu trữ bản sao hoàn toàn của dữ liệu (copy by value), không lưu tham chiếu

        int value = 10;
        xQueueSend(q, &value, 0);
        value = 99;   // thay đổi biến gốc

        int rx;
        xQueueReceive(q, &rx, portMAX_DELAY);
        // rx == 10

 
*   Mỗi item có kích thước cố định (xác định khi tạo queue)

        QueueHandle_t q;
        q = xQueueCreate(5, sizeof(float));  // mỗi item là float

        float f = 3.14f;
        xQueueSend(q, &f, 0);   // hợp lệ

        int i = 10;
        xQueueSend(q, &i, 0);   // Sai kiểu / sai kích thước



*   Queue sử dụng bộ đệm có kích thước cố định, trong đó số lượng phần tử tối đa không thay đổi trong suốt vòng đời của hàng đợi

        QueueHandle_t q;
        q = xQueueCreate(3, sizeof(uint8_t));

        xQueueSend(q, &a, 0);  // item 1
        xQueueSend(q, &b, 0);  // item 2
        xQueueSend(q, &c, 0);  // item 3
        xQueueSend(q, &d, 0);  // queue full


*   Hoạt động theo nguyên tắc FIFO

        xQueueSend(q, &item1, 0);
        xQueueSend(q, &item2, 0);
        xQueueSend(q, &item3, 0);

        xQueueReceive(q, &rx, portMAX_DELAY);  // rx = item1
        xQueueReceive(q, &rx, portMAX_DELAY);  // rx = item2
        xQueueReceive(q, &rx, portMAX_DELAY);  // rx = item3

*   Cơ chế nội bộ:

    ◦   Queue bao gồm một vùng bộ nhớ đệm + hai con trỏ: head (vị trí đọc) và tail (vị trí ghi)

    ◦   Khi gửi: Sao chép dữ liệu vào tail -> tăng tail

    ◦   Khi nhận: Sao chép dữ liệu từ head ra bộ nhớ của task nhận -> tăng head   

    ◦   Trong suốt thao tác gửi / nhận, kernel bảo vệ hàng đợi bằng cơ chế nội bộ (tạm thời vô hiệu hóa ngắt hoặc sử dụng vùng bảo vệ thích hợp), đảm bảo an toàn ngay cả với các phần tử có kích thước lớn 

#### **2.2. Truy cập bởi nhiều tác vụ**

*   **Thread-Safe:**

    ◦   Nhiều task có thể đồng thời gửi và nhận dữ liệu mà không gây ra xung đột hay lỗi dữ liệu   

    ◦   Kernel tự động bảo vệ toàn bộ thao tác enqueue/dequeue bằng các cơ chế đồng bộ nội bộ, đảm bảo tính toàn vẹn dữ liệu

        xQueueSend(sensorQueue, &sensorData, 0);        // Task A
        xQueueSend(statusQueue, &statusData, 0);        // Task B
        xQueueSendFromISR(eventQueue, &event, NULL);    // ISR

*   **Multiple Producers:**

    ◦   Nhiều task có thể đồng thời gửi dữ liệu vào cùng một hàng đợi

    ◦   Kernel đảm bảo các phần tử được xếp vào queue theo đúng thứ tự FIFO dựa trên thời điểm gửi

        // Task A: đọc cảm biến nhiệt độ

        void TempTask(void *arg)
        {
            int temp;
            while (1)
            {
                temp = readTemperature();
                xQueueSend(dataQueue, &temp, portMAX_DELAY);
            }
        }

        // Task B: đọc cảm biến độ ẩm

        void HumiTask(void *arg)
        {
            int humi;
            while (1)
            {
                humi = readHumidity();
                xQueueSend(dataQueue, &humi, portMAX_DELAY);
            }
        }

        // Cả hai gửi về Task xử lý trung tâm

        void ProcessingTask(void *arg)
        {
            int value;
            while (1)
            {
                xQueueReceive(dataQueue, &value, portMAX_DELAY);
                process(value);
            }
        }

*   **Multiple Consumers:**

    ◦   có thể cùng chờ nhận dữ liệu từ một queue

    ◦   Khi có phần tử mới, task ở trạng thái sẵn sàng (Ready) với mức ưu tiên phù hợp sẽ nhận phần tử đầu tiên trong hàng đợi.

        // Task Logger: ghi log

        void LoggerTask(void *arg)
        {
            Event_t event;
            while (1)
            {
                xQueueReceive(eventQueue, &event, portMAX_DELAY);
                logEvent(event);
            }
        }

        // Task Display: cập nhật màn hình

        void DisplayTask(void *arg)
        {
            Event_t event;
            while (1)
            {
                xQueueReceive(eventQueue, &event, portMAX_DELAY);
                updateDisplay(event);
            }
        }


#### **2.3. Chặn khi đọc Queue**

*   Khi queue rỗng và task gọi hàm `xQueueReceive()`, hành vi của hệ thống phụ thuộc vào giá trị timeout:

    ◦   Timeout = 0 -> Hàm trả về errQUEUE_EMPTY, task không bị chặn 

    ◦   Timeout > 0 hoặc portMAX_DELAY -> Task được chuyển sang trạng thái Blocked và được đưa vào danh sách chờ đọc của hàng đợi 

    ◦   Khi có dữ liệu mới được gửi vào queue (gửi thành công):

        Kernel đánh thức task có mức ưu tiên cao nhất trong danh sách chờ đọc, chuyển task đó sang trạng thái Ready

        Bộ lập lịch sẽ thực thi task ngay nếu điều kiện ưu tiên cho phép.


#### **2.4. Chặn khi ghi Queue**

*   Khi hàng đợi đã đầy và một task gọi hàm gửi dữ liệu (`xQueueSend()` hoặc `xQueueSendToBack()`), hành vi của hệ thống phụ thuộc vào giá trị timeout:

    ◦   Timeout = 0 -> Hàm trả về errQUEUE_FULL, task không bị chặn 

    ◦   Timeout > 0 hoặc portMAX_DELAY -> Task được chuyển sang trạng thái Blocked và được đưa vào danh sách chờ ghi của hàng đợi 

    ◦   Khi có chỗ trống trong queue (do một thao tác nhận dữ liệu thành công):

        Kernel cho phép task đang chờ ghi thực hiện thao tác gửi, sau đó đánh thức task đó và chuyển nó sang trạng thái Ready

        Bộ lập lịch sẽ thực thi task ngay nếu điều kiện ưu tiên cho phép.

#### **2.5. Chặn trên nhiều Queue**

*   Trong nhiều hệ thống RTOS, một task có thể cần chờ dữ liệu từ nhiều nguồn khác nhau, mỗi nguồn được truyền qua một hàng đợi riêng biệt.

    ◦   Nếu dùng receive thông thường → task bị block tại queue đầu tiên, có thể bỏ lỡ dữ liệu từ queue khác.

    ◦   FreeRTOS cung cấp Queue Set để block trên toàn bộ tập hợp queue → thức dậy khi bất kỳ queue nào có sự kiện.

*   Quy trình hoạt động

    ◦   Tạo một Queue Set

    ◦   Thêm các queue cần theo dõi vào Queue Set

    ◦   Task gọi xQueueSelectFromSet() để block

    ◦   Khi có sự kiện:

        Hàm trả về queue nào đã kích hoạt

        Task tiếp tục gọi xQueueReceive() trên queue đó

#### **2.6. Tạo Queue**

##### **2.6.1. Dynamic Allocation**

*   **Nguyên lý hoạt động:**

    ◦   Bộ nhớ cho Queue Control Block (QCB) và vùng lưu trữ item được cấp phát từ heap của hệ thống tại thời điểm chạy (runtime).

    ◦   Việc cấp phát được thực hiện thông qua hàm `pvPortMalloc()` nội bộ.

*   **Đặc điểm:**

    ◦   Bộ nhớ chỉ được cấp phát khi hàm tạo queue được gọi

    ◦   Nếu heap không đủ dung lượng, việc tạo queue sẽ thất bại

    ◦   Bộ nhớ có thể bị phân mảnh trong hệ thống chạy lâu dài


##### **2.6.2. Static Allocation:**

*   **Nguyên lý hoạt động:**

    ◦   Toàn bộ bộ nhớ cần thiết cho queue được cấp phát sẵn bởi lập trình viên

        Bộ nhớ cho Queue Control Block

        Bộ đệm lưu trữ item

    ◦   FreeRTOS không thực hiện bất kỳ thao tác cấp phát heap nào trong suốt vòng đời của queue.

*   **Đặc điểm:**

    ◦   Bộ nhớ được cấp phát: Tại compile-time hoặc ngay khi khởi động hệ thống

    ◦   Kích thước queue: Số lượng item tối đa, kích thước mỗi item

     </details> 
