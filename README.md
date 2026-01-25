# CHƯƠNG 4: TASK MANAGEMENT 
<details>
    <summary><strong>BÀI 1: TẠO VÀ QUẢN LÝ TASK</strong></summary>

## **BÀI 1: TẠO VÀ QUẢN LÝ TASK**

### **I. Giới thiệu**

#### **1.1. Khái niệm**

* Task là đơn vị thực thi độc lập cơ bản nhất trong FreeRTOS, tương đương với thread trong các hệ điều hành hiện đại

* Mỗi task đại diện cho một luồng xử lý logic riêng như:

    ◦ Đọc cảm biến

    ◦ Giao tiếp UART/SPI/I2C

    ◦ Xử lý dữ liệu

    ◦ Điều khiển động cơ

* Trong FreeRTOS, toàn bộ ứng dụng được xây dựng bằng nhiều task chạy song song, chứ không phải một vòng `while(1)` duy nhất

#### **1.2. Thành phần**

*  **Task Function (Hàm thực thi)**
    
    ◦ Cú pháp

        void vTaskFunction(void *pvParameteres);

    ◦ Chạy trong vòng lặp vô hạn (`for(;;)` hoặc `while(1)`) hoặc kết thúc khi hoàn thành nhiệm vụ 

    ◦ Task không được return trừ khi dùng `vTaskDelete(NULL)`

    ◦ Task phải chủ động nhường CPU bằng delay, block hoặc chờ sự kiện 

    ◦ VD:

        void vLedTask(void *pvParameters){
            for(;;)
            {
                ToggleLED();
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }

*  **Task Stack:**

    ◦ Mỗi task có ngăn xếp (stack) độc lập

    ◦ Stack dùng để lưu: biến cục bộ, tham số hàm, context khi context switch

    ◦ Kích thước stack cấp khi tạo task (đơn vị là word, không phải byte)

    ◦ Nếu stack quá nhỏ sẽ dẫn đế crash hệ thống 

*  **Task Control Block (TCB):**

    ◦ TCB là cấu trúc dữ liệu nội bộ của FreeRTOS, mỗi task có đúng một TCB

    ◦ TCB chứa các thông tin như:

        Con trỏ stack hiện tại 
        
        Priority của task 

        Trạng thái task 

        Thông tin timeout 

        Handle của task

    ◦ Người dùng không truy cập trực tiếp TCB, mà thao tác thông qua API FreeRTOS

#### **1.3. State của Task**

* Một task trong FreeRTOS có thể nằm ở một trong các trạng thái sau:

    ◦ Running: Task đang được thực thi 

    ◦ Ready: Task sẵn sàng chạy, chờ CPU 

    ◦ Blocked: Task đang chờ sự kiện hoặc timeout

    ◦ Suspended: Task bị tạm dừng hoàn toàn  

#### **1.4. Priority**

* Priority là số nguyên 

* Giá trị càng lớn, priority càng cao 

* Phạm vi: **0 -> (configMAX_PRIORITIES - 1)**

* Giá trị configMAX_PRIORITIES được định nghĩa trong `FreeRTOSConfig.h`

* **VD:**

        xTaskCreate(vTaskA, "A", 256, NULL, 1, NULL);
        xTaskCreate(vTaskB, "B", 256, NULL, 3, NULL);

        -> Task B có priority cao hơn Task A

#### **1.5. Scheduler trong FreeRTOS**

##### **1.5.1. Mô hình Scheduler**

* FreeRTOS là RTOS sử dụng :

    ◦ Preemptive

    ◦ Priority-based scheduler 

* Scheduler luôn chọn task có priority cao nhất 

* CPU không bị giữ độc quyền bởi task đang chạy 

##### **1.5.2. Preemption (Chiếm quyền)**

* Nếu Task A đang Running, task B có priority cao hơn chuyển sang Ready

* Context switch xảy ra ngay lập tức

    ◦ Task A bị tạm dừng

    ◦ Task B được chạy 

* Điều này góp phần đảm bảo đáp ứng thời gian thực và task quan trọng luôn được xử lý sớm nhất 

##### **1.5.3. Time Slicing**

* Nếu nhiều task cùng priority, thì `configUSE_TIME_SLICING = 1`

* Các task sẽ:

    ◦ Chia CPU theo từng tick

    ◦ Chạy luân phiên (Round Robin) 

##### **1.5.4. Idle Task**

* FreeRTOS luôn có Idle Task

* Priority thấp nhất (`0`)

* Chạy khi không còn task Ready nào khác

* Idle Task dùng để:

    ◦ Dọn dẹp task đã delete

    ◦ Có thể hook để tiết kiệm năng lượng (sleep)

### **II.Hàm tạo Task**

#### **2.1. xTaskCreate() – Tạo task động**

* Hàm này cấp phát bộ nhớ cho TCB và stack từ heap của FreeRTOS

* **Cú pháp:**

        BaseType_t xTaskCreate(
            TaskFunction_t pxTaskCode,
            const char * const pcName,
            const uint32_t usStackDepth,
            void * const pvParameters,
            UBaseType_t uxPriority,
            TaskHandle_t * const pxCreatedTask
        );

* **Tham số:**

    ◦   **pxTaskCode:** Hàm thực thi của task

        Con trỏ tới hàm task

        Prototype:

            void vTaskFunction(void *pvParameters);

    ◦   **pcName:** Tên của task

        Chuỗi ký tự đặt tên cho task 

        Không ảnh hưởng đến scheduler 

    ◦   **usStackDepth:** Kích thước stack

        Kích thước stack tính bằng word (4 bytes)

    ◦   **pvParameters:** Tham số truyền vào task

        Con trỏ truyền dữ liệu vào task 

        Được nhận trong task function 

        Được sử dụng để: truyền struct, ID task, handle   

    ◦   **uxPriority:** Priority của task

        Độ ưu tiên của task 

        Priority cao hơn -> được chạy trước  

    ◦   **pxCreatedTask:** Handle của task

        Con trỏ để nhận handle của task vừa tạo 

        Có thể truyền NULL nếu không cần 

* **Giá trị trả về:**

    ◦   **pdPASS:** Tạo task thành công

    ◦   **errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:** Không đủ heap   

* **Ví dụ**

        void vTask1(void *pvParameters){
            for(;;)
            {
                printf("Task 1 running\r\n");
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }

        void app_main(){
            TaskHandle_t xHandleTask1 = NULL;

            xTaskCreate(
                vTask1,         // Hàm task 
                "Task1",        // Tên
                2048,           // Stack depth 
                NULL,           // Tham số 
                2,              // Priority = 2
                &xHandleTask1   // Handle
            )
        }

#### **2.2. xTaskCreateStatic() – Tạo task tĩnh**

* Hàm này không dùng heap

* Người dùng phải tự cấp phát bộ nhớ cho TCB và stack (thường là mảng static/global)

* **Cú pháp:**

        TaskHandle_t xTaskCreateStatic(
            TaskFunction_t pxTaskCode,
            const char * const pcName,
            const uint32_t ulStackDepth,
            void * const pvParameters,
            UBaseType_t uxPriority,
            StackType_t * const puxStackBuffer,
            StaticTask_t * const pxTaskBuffer
        );

* **Tham số:**

    ◦   **pxTaskCode:** Hàm thực thi của task

        Con trỏ tới hàm task

        Prototype:

            void vTaskFunction(void *pvParameters);

    ◦   **pcName:** Tên của task

        Chuỗi ký tự đặt tên cho task 

        Không ảnh hưởng đến scheduler 

    ◦   **ulStackDepth:** Kích thước stack

        Kích thước stack tính bằng word (4 bytes)

    ◦   **pvParameters:** Tham số truyền vào task

        Con trỏ truyền dữ liệu vào task 

        Được nhận trong task function 

        Được sử dụng để: truyền struct, ID task, handle   

    ◦   **uxPriority:** Priority của task

        Độ ưu tiên của task 

        Priority cao hơn -> được chạy trước  

    ◦   **puxStackBuffer:** Bộ nhớ stack do người dùng cấp 

        Con trỏ tới mảng stack 

        Kích thước >= ulStackDepth  

    ◦   **pxTaskBuffer:** Bộ nhớ TCB do người dùng cấp 

        Con trỏ tới Task Control Block 

        Phải là biến static/global

* **Giá trị trả về:**

    ◦   **TaskHandle_t ≠ NULL:** Thành công 

    ◦   **NULL:** Tham số không hợp lệ   

* **Ví dụ**

    ◦   Khi **configSUPPORT_STATIC_ALLOCATION = 1** trong FreeRTOSConfig.h

    ◦   Hệ thống không cho phép heap hoặc cần kiểm soát bộ nhớ hoàn toàn

        // Cấp phát bộ nhớ tĩnh 
        static StackType_t xStack1[2048];
        static StaticTask_t xTaskBuffer1;

        void vTask1(void *pvParamters){
            for(;;)
            {
                printf("Static Task 1 running\r\n");
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }

        void app_main(){
            TaskHandle_t xHandleTask1 = NULL;
            xHandleTask1 = xTaskCreateStatic(
                vTask1,
                "StaticTask1",
                2048,
                NULL,
                2,
                xStack1,        // Stack buffer 
                &xTaskBuffer1   // TCB buffer 
            );

            if(xHandleTask1 == NULL){
                // Tạo thất bại
            }
        }

  
     </details> 

<details>
    <summary><strong>BÀI 2: DELAY VÀ PERIODIC TASK</strong></summary>

## **BÀI 2: DELAY VÀ PERIODIC TASK**

### **I. Tick**

#### **1.1. Khái niệm**

* Tick là đơn vị thời gian cơ bản nhất mà FreeRTOS sử dụng để

    ◦ Lập lịch

    ◦ Delay task

    ◦ Timeout queue / semaphore / mutex

    ◦ Đo thời gian tương đối và tuyệt đối

* FreeRTOS không làm việc trực tiếp với ms mà làm việc với tick

#### **1.2. Cấu tạo**

*  Tick được sinh ra bởi ngắt timer định kỳ

*  Trên Cortex-M
    
    ◦ Thường dùng SysTick timer

*  Trên kiến trúc khác:

    ◦ Dùng timer phần cứng tương đương

*  Mỗi lần timer ngắt 

    ◦ ISR của FreeRTOS chạy

    ◦ Tick count tăng lên 1 

#### **1.3. Tần số Tick - configTICK_RATE_HZ**

*  Được định nghĩa trong `FreeRTOSConfig.h`

        #define configTICK_RATE_HZ      1000

    ◦ 1000 tick / giây 

    ◦ 1 tick = 1 ms     

*  Một số giá trị phổ biến 

    | configTICK_RATE_HZ | Độ phân giải  |
    | ------------------ | ------------- |
    | 100                | 10 ms / tick  |
    | 250                | 4 ms / tick   |
    | 1000               | 1 ms / tick   |
    | 2000               | 0.5 ms / tick |

#### **1.4. Biến Tick toàn cục - xTickCount**

*  FreeRTOS duy trì biến tick toàn cục 

        volatile TickType_t xTickCount;

*  Lưu số tick đã trôi qua kể từ khi scheduler khởi động 

*  Người dùng không truy cập trực tiếp mà dùng API:

        TickType_t xTaskGetTickCount(void);


#### **1.5. Chuyển đổi Tick - ms**

*  FreeRTOS cung cấp macro tiện lợi  

        #define portTICK_PERIOD_MS (( TickType_t) 1000 / configTICK_RATE_HZ)


### **II.vTaskDelay() - Delay tương đối**

#### **2.1. Khái niệm**

* **Cú pháp:**

        void vTaskDelay(const TickType_t xTicksToDelay);

    ◦   Task bị block trong xTicksToDelay
    
    ◦   Thời gian tính kể từ lúc gọi hàm 

#### **2.2. Cơ chế**

* **1.** Task gọi `vTaskDelay()`

* **2.** FreeRTOS:

    ◦   Đưa task sang trạng thái Blocked
    
    ◦   Ghi lại tick hiện tại + thời gian delay

* **3.** Khi tick count đạt mốc:

    ◦   Task chuyển sang ready

* **4.** Scheduler chạy task khi đến lượt (theo priority)

* **Lưu ý:** Task bị block không tốn CPU

#### **2.3. Cách sử dụng**

        vTaskDelay(pdMS_TO_TICKS(500)); // delay 500 ms

    ◦   Hoặc:

        vTaskDelay(500 / portTICK_PERIOD_MS);

#### **2.4. Nhược điểm**

* Sai số tích lũy sau mỗi chu kỳ 

* Phù hợp với Task không yêu cầu chu kỳ chính xác và delay đơn lẻ

        void vPeriodicTask( void *pvParameters )
        {
            for( ;; )
            {
                do_work_50ms();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

    ◦   Giả sử:

        Công việc mất 50 ms 

        50 ms (work) + 1000 ms (delay) = 1050 ms 

        Sai số tích lũy 50 ms sau mỗi chu kỳ 

### **III.vTaskDelayUntil() – Delay tuyệt đối**

#### **3.1. Khái niệm**

* `vTaskDelayUntil()` là API dùng để block task cho đến một mốc thời gian tuyệt đối, được xác định dựa trên tick count hệ thống.


#### **3.2. Cú pháp**

        void vTaskDelayUntil(
            TickType_t * const pxPreviousWakeTime,
            const TickType_t xTimeIncrement
        );

    ◦   Task bị block đến một mốc tick tuyệt đối
    
    ◦   Dựa trên tick count hệ thống

* **Tham số:**

    ◦   **pxPreviousWakeTime**

        Con trỏ tới biến lưu thời điểm task thức dậy lần trước 

        Phải khởi tạo trước vòng lặp, không thay đổi thủ công 
    
    ◦   **xTimeIncrement**

        Chu kỳ task (tính bằng tick)

        Thường dùng 

            pdMS_TO_TICKS(period_ms)

* **Cách dùng:**

        void vPeriodicTask(void *pvParameters){
            TickType_t xLastWakeTime;
            const TickType_t xPeriod = pdMS_TO_TICKS(1000);

            xLastWakeTime = xTaskGetTickCount();
            for(;;){
                do_work();
                vTaskDelayUntil(&xLastWakeTime, xPeriod);
            }
        }

* **Điều kiện:**

    ◦   Thời gian thực thi 

        work_time < period
    
    ◦   Nếu work_time > period, TASK bị miss deadline    

     </details> 

<details>
    <summary><strong>BÀI 3: TASK STATE VÀ RUNTIME</strong></summary>

## **BÀI 3: TASK STATE VÀ RUNTIME**

### **I. Task States**

#### **1.1. Khái niệm**

* Mỗi task luôn ở 4 trạng thái chính

    ◦ **Running:** Task đang thực thi trên CPU

    ◦ **Ready:** Task sẵn sàng chạy, chờ scheduler chọn

    ◦ **Blocked:** Task đang chờ một sự kiện

    ◦ **Suspend:** Task bị tạm dừng hoàn toàn

* **Scheduler** luôn chọn task có priority cao nhất ở trạng thái **Ready** để chạy 

#### **1.2. Đặc điểm**

##### **1.2.1. Running**

*  Task hiện đang chiếm CPU và thực thi code

*  Mỗi core chỉ có 1 task Running

*  Task running luôn có priority cao nhất trong các task ready

*  Task đang running có thể:

    ◦ Tự nhường CPU

    ◦ Bị tước quyền thực thi (preempt) bởi task priority cao hơn 

    ◦ Bị chuyển sang Blocked/Suspended

*  Task rời trạng thái Running khi:

    ◦ Gọi `vTaskDelay()`, `xQueueReceive()`, `xSemaphoreTake()` -> `Blocked`

    ◦ Gọi `vTaskSuspend()` -> `Suspend`

    ◦ Có task priority cao hơn Ready -> `Bị Preempt`

    ◦ Hết time slice (cùng priority, time slicing bật) -> `Ready`

        void TaskA(void *pvParameters){
            while(1){
                // Task đang ở trạng thái Running
                do_sth();

                vTaskDelay(pdMS_TO_TICKS(1000));
                // Chuyển sang Blocked
            }
        }


##### **1.2.2. Ready**

*   Task có thể chạy ngay

*   Không chờ sự kiện nào 

*   Chỉ thiếu CPU

*   Cách scheduler xử lý:

    ◦ Scheduler luôn chọn task Ready có priority cao nhất

    ◦ Nếu nhiều task Ready cùng priority: 

        configUSE_TIME_SLICING = 1 → các task được thực thi luân phiên.

        configUSE_TIME_SLICING = 0 → task đang chạy giữ CPU cho tới khi tự block/yield

*   Các nguyên nhân khiến task vào Ready

    ◦ Vừa được tạo bằng (`xTaskCreate`)

    ◦ Kết thúc thời gian delay

    ◦ ISR gọi API dạng `FromISR`

    ◦ Được `vTaskResume()`     

##### **1.2.3. Blocked**

*   Task ở trạng thái Blocked là task tạm thời không thể chạy do đang chờ thời gian, tài nguyên hoặc sự kiện đồng bộ.

*   Trạng thái Blocked không phải là ngủ vô thời hạn. 

*   Khi hết timeout hoặc có sự kiện xảy ra, task sẽ quay lại trạng thái Ready.

*   Các API thường gây Blocked:

    ◦ `vTaskDelay()`

    ◦ `vTaskDelayUntil()`

    ◦ `xQueueReceive()`

    ◦ `xSemaphoreTake()` 

    ◦ `xEventGroupWaitBits()` 


##### **1.2.4. Suspended**

*   Task ở trạng thái này là task bị loại khỏi quá trình lập lịch và không có cơ chế tự động quay lại trạng thái **Ready**

*   Cách vào và thoát trạng thái **Suspended**

    ◦ `vTaskSuspend(NULL)`: Tạm dừng chính task đang chạy

    ◦ `vTaskSuspend(taskHandle)`: Tạm dừng task khác

    ◦  Task chỉ quay lại Ready khi được gọi `vTaskResume()` hoặc `xTaskResumeFromISR()` 

##### **1.2.5. Deleted**

*   Task ở trạng thái này là task đã bị yêu cầu xóa thông qua hàm `vTaskDelete()`

*   Sau khi xóa, task không còn được scheduler xét tới và không thể tiếp tục thực thi

*   Khi một task bị xóa:

    ◦ Task Control Block (TCB) và stack của task không nhất thiết được giải phóng ngay lập tức.

    ◦ Việc giải phóng bộ nhớ thường được thực hiện bởi Idle task, nhằm tránh ảnh hưởng đến hoạt động lập lịch của hệ thống.

*   Việc tạo và xóa task thường xuyên có thể:

    ◦ Gây phân mảnh heap, đặc biệt trong các hệ thống nhúng có bộ nhớ hạn chế.

    ◦ Làm tăng chi phí quản lý bộ nhớ và giảm độ ổn định của hệ thống.

*   Trong các ứng dụng thực tế, task thường:

    ◦ Được tạo một lần khi khởi động hệ thống.

    ◦ Được tạm dừng hoặc kích hoạt lại bằng cơ chế suspend/resume thay vì xóa và tạo lại.

        void SensorTask(void *pvParameters)
        {
            // Thực hiện đo cảm biến một lần
            read_sensor();

            // Sau khi hoàn thành nhiệm vụ, tự xóa
            vTaskDelete(NULL);
        }

##### **1.2.6. Idle Task**

*   Idle task là task do FreeRTOS tự động tạo ra khu scheduler được khởi động

*   Task này:

    ◦ Luôn tồn tại trong hệ thống

    ◦ Có mức ưu tiên thấp nhất

    ◦ Không thể bị xóa bởi người dùng    

*   Idle task chỉ chạy khi không còn task nào khác ở trạng thái Ready, tức khi CPU đang rảnh

*   Idle Task thực hiện: 

    ◦ Giải phóng bộ nhớ của các task đã bị xóa (Deleted).

    ◦ Đảm bảo hệ thống luôn có một task để thực thi, giúp scheduler hoạt động ổn định.

*   FreeRTOS cho phép người dùng mở rộng hành vi của Idle task thông qua hàm Idle Hook (`vApplicationIdleHook()`).

### **II.Cấu hình trong FreeRTOSConfig.h**

#### **2.1. configUSE_TRACE_FACILITY**

* **Cú pháp:**

        #define configUSE_TRACE_FACILITY      1

* Macro cho phép **FreeRTOS** thu thập thông tin nội bộ về task, bao gồm:

    ◦   Tên task
    
    ◦   Priority

    ◦   Trạng thái task
    
    ◦   Stack còn trống  

* Nếu được đặt bằng `0`, các API liên quan đến việc giám sát task sẽ không hoạt động hoặc bị loại bỏ khi biên dịch.


#### **2.2. configGENERATE_RUN_TIME_STATS**

* **Cú pháp:** 

        #define configGENERATE_RUN_TIME_STATS       1

* Macro này cho phép FreeRTOS thu thập thống kê thờ gian CPU mà mỗi task đã sử dụng kể từ khi hệ thống khởi động

    ◦   Tổng thời gian CPU mỗi task đã chạy
    
    ◦   Tỷ lệ phần trăm CPU mà task chiếm dụng

* Khi `configGENERATE_RUN_TIME_STATS = 1`, người dùng bắt buộc phải cung cấp hai macro sau:

    ◦   `void portCONFIGURE_TIMER_FOR_RUN_TIME_STATS(void);`

        Khởi tạo bộ đếm thời gian (timer)

        Đặt tần số đếm phù hợp

        Bắt đầu quá trình đo thời gian runtime 

    ◦   `uint32_t portGET_RUN_TIME_COUNTER_VALUE(void);`

        Ghi nhận thời điểm task bắt đầu chạy 

        Ghi nhận thời điểm task kết thúc chạy 

        Tính toán tổng thời gian CPU mà task đã sử dụng

#### **2.3. configUSE_STATS_FORMATTING_STRINGS**

* **Cú pháp:** 

        #define configUSE_STATS_FORMATTING_STRINGS      1

* Macro này cho phép FreeRTOS tạo chuỗi định dạng sẵn khi xuất thông tin task

* Nếu tắt macro này:

    ◦   Người dùng vẫn có thể lấy dữ liệu thô nhưng phải tự định dạng lại chuỗi hiển thị


### **III.Task Monitoring APIs**

#### **3.1. vTaskList()**

##### **3.1.1. Khái niệm**

*   `vTaskList()` dùng để lấy danh sách toàn bộ task trong hệ thống, kèm theo các thông tin cơ bản như:

    ◦   Tên task

    ◦   Trạng thái hiện tại    

    ◦   Mức ưu tiên

    ◦   Stack còn trống

    ◦   Số thứ tự task    

##### **3.1.2. Cú pháp**

        void vTaskList(char *pcWriteBuffer);

* **VD:**

        char buffer[512];

        vTaskList(buffer);
        printf("Name   State Prio Stack Num\n%s\n", buffer);

* **Output:**

        TaskA   R     2    120   1
        TaskB   B     1    200   2
        IDLE    R     0    300   3


#### **3.2. vTaskGetRunTimeStats()**

##### **3.2.1. Khái niệm**

*   `vTaskGetRunTimeStats()` cung cấp thống kê thời gian CPU mà mỗi task đã sử dụng kể từ khi hệ thống khởi động   

##### **3.2.2. Cú pháp**

        void vTaskGetRunTimeStats(char *pcWriteBuffer);

* **VD:**

        char buffer[512];

        vTaskGetRunTimeStats(buffer);
        printf("Task Runtime Stats:\n%s\n", buffer);

* **Output:**

        TaskA    3500    35%
        TaskB    1500    15%
        IDLE     5000    50%

#### **3.3. eTaskGetState()**

##### **3.3.1. Khái niệm**

*   `eTaskGetState()` trả về trạng thái hiện tại của một task cụ thể.

*   Các trạng thái có thể trả về:

    ◦   eRunning

    ◦   eReady   

    ◦   eBlocked

    ◦   eSuspended

    ◦   eDeleted

*   Áp dụng cho từng task riêng lẻ 

##### **3.3.2. Cú pháp**

        eTaskState eTaskGetState(taskHandle_t xTask);

* **VD:**

        eTaskState state = eTaskGetState(sensorTaskHandle);

        if (state == eBlocked){
            // Task đang chờ sự kiện
        }

#### **3.4. uxTaskPriorityGet()**

##### **3.4.1. Khái niệm**

*   `eTaskPriorityGet()` dùng để truy vấn mức ưu tiên hiện tại của một task tại runtime

##### **3.4.2. Cú pháp**

        UBaseType_t uxTaskPriorityGet(TaskHandle_t xTask);
    
*   **xTask:** handle của task cần kiểm tra

* **VD:**

        UBaseType_t prio = uxTaskPriorityGet(controlTaskHandle);
        printf("Task priority: %lu\n", prio);

* **Output:**

        Task priority: 3
   
#### **3.5. ulTaskGetStackHighWaterMark()**

##### **3.5.1. Khái niệm**

*   `ulTaskGetStackHighWaterMark()` trả về mức stack thấp nhất từng đạt được của task kể từ khi bắt đầu chạy.

*    Dùng để đánh giá kích thước stack đã cấp phát có hợp lý hay không.

##### **3.5.2. Cú pháp**

        uint32_t ulTaskGetStackHighWaterMark(TaskHandle_t xTask);

*   Giá trị trả về tính theo word, không phải byte.

* **VD:**

        uint32_t freeStack = ulTaskGetStackHighWaterMark(sensorTaskHandle);

        printf("Free stack: %lu words\n", freeStack);

        if (freeStack < 50)
        {
            printf("Warning: Stack nearly full!\n");
        }

* **Output:**

        Free stack: 32 words
        Warning: Stack nearly full!


#### **3.6. uxTaskGetNumberOfTasks()**

##### **3.6.1. Khái niệm**

*   `uxTaskGetNumberOfTasks()` trả về tổng số task đang tồn tại trong hệ thống, bao gồm:

    ◦   Các task người dùng

    ◦   Idle task  

    ◦   (Nếu bật) Timer service task

*   Dùng để giám sát số lượng task tại runtime.

##### **3.6.2. Cú pháp**

        UBaseType_t uxTaskGetNumberOfTasks(void);

* **VD:**

        UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        printf("Number of tasks: %lu\n", taskCount);

* **Output:**

        Number of tasks: 6

#### **3.7. xTaskGetTickCount()**

##### **3.7.1. Khái niệm**

*   `xTaskGetTickCount()` trả về số tick kể từ khi scheduler bắt đầu chạy.

*   Giá trị tăng đều theo chu kỳ tick.

*   Độ phân giải phụ thuộc `configTICK_RATE_HZ`.

##### **3.7.2. Cú pháp**

        TickType_t xTaskGetTickCount(void);

* **VD:**

        TickType_t start = xTaskGetTickCount();

        /* Thực hiện công việc */
        vTaskDelay(pdMS_TO_TICKS(100));

        TickType_t elapsed = xTaskGetTickCount() - start;
        printf("Elapsed ticks: %lu\n", elapsed);


* **Output:**

        Elapsed ticks: 100


     </details> 

