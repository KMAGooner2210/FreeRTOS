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

