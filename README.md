# **FreeRTOS**

<details>
	<summary><strong>BÀI 1: GIỚI THIỆU FREERTOS</strong></summary>

## **BÀI 1: GIỚI THIỆU FREERTOS**

### **I. Khái niệm**

* FreeRTOS (Free Real-Time Operating System) là một hệ điều hành thời gian thực (RTOS) mã nguồn mở, được thiết kế đặc biệt cho các hệ thống nhúng (embedded systems) chạy trên vi điều khiển (MCU) và vi xử lý (MPU).

<img width="76" height="33" alt="Image" src="https://github.com/user-attachments/assets/da8b213b-e41a-4540-9997-38f9907b5afd" />


### **II.Tính cần thiết**

#### **2.1. Kiến Trúc Super Loop**

##### **2.1.1.Super Loop** 

* Đây là mô hình lập trình nhúng kinh điển, nơi chương trình chính là một vòng lặp while(1) vô tận, lần lượt gọi các hàm xử lý.

        // MÔ HÌNH SUPER LOOP ĐIỂN HÌNH
        void main(void) {
            hardware_init(); // Khởi tạo phần cứng

            while (1) { // Vòng lặp chính - "Super Loop"
                read_sensor();       // 50ms → block toàn bộ hệ thống
                process_data();      // 10ms
                update_display();    // 20ms
                check_button();      // 1ms
                // Tổng thời gian 1 vòng lặp: 81ms
            }
        }

##### **2.1.2.Vấn đề thường gặp** 

* **Blocking Code:**

  ◦ Bản chất: 

        Vi điều khiển chỉ có 1 luồng thực thi (CPU core). 

        Khi một hàm được gọi, CPU sẽ thực thi hàm đó đến khi kết thúc thì mới chuyển sang hàm tiếp theo.

  ◦ Hậu quả: 
  
        Một hàm có thời gian thực thi lâu (ví dụ: read_sensor() mất 50ms) sẽ làm delay toàn bộ hệ thống trong 50ms đó. 
        
        Các hàm khác như check_button() dù chỉ mất 1ms nhưng vẫn phải chờ 50ms mới được thực hiện.

  ◦ Ví dụ thực tế: 
  
        Hệ thống đang đọc cảm biến nhiệt độ (chậm) thì người dùng nhấn nút → Phản hồi nút nhấn bị trễ, gây ra cảm giác "giật", "lag".


* **Priority  &  Real-Time**

  ◦ Bản chất: 
  
        Trong Super Loop, tất cả các công việc đều có mức độ ưu tiên ngang nhau theo thứ tự trong vòng lặp. 
  
        Công việc quan trọng (ví dụ: xử lý báo động) không thể "cướp quyền" CPU từ một công việc ít quan trọng hơn nhưng đang chạy trước nó.

  ◦ Thời gian phản hồi (Response Time) không xác định: 
  
        Bạn không thể đảm bảo hệ thống sẽ phản hồi một sự kiện trong bao lâu. 
  
        Thời gian phản hồi tồi tệ nhất (Worst-Case Response Time) chính là thời gian của cả vòng lặp (81ms trong ví dụ). 
  
        Điều này là tối kỵ trong các hệ thống thời gian thực.

* **Co-operative Multitasking &  Time Management**

  ◦ Co-operative: 
  
        Để khắc phục phần nào vấn đề blocking, lập trình viên thường phải viết các hàm ở dạng Non-Blocking (trạng thái máy - State Machine). 
  
        Điều này làm code trở nên rất phức tạp, khó đọc và bảo trì.


        // VIẾT THEO KIỂU STATE MACHINE - PHỨC TẠP
        enum sensor_state { S_START, S_READING, S_DONE };
        enum sensor_state current_state = S_START;

        void handle_sensor_non_blocking() {
            switch (current_state) {
                case S_START:
                    start_sensor_conversion();
                    current_state = S_READING;
                    break;
                case S_READING:
                    if (is_sensor_data_ready()) {
                        read_sensor_data();
                        current_state = S_DONE;
                    }
                    break;
                case S_DONE:
                    // ... xử lý dữ liệu
                    current_state = S_START;
                    break;
            }
        }

  ◦ Time Management:
  
        Việc đảm bảo các tác vụ chạy đúng chu kỳ (ví dụ: đọc cảm biến mỗi 100ms, cập nhật màn hình mỗi 50ms) trở nên rất "thủ công" và dễ sai sót.

* **Shared Resources &  Synchronization**

  ◦ Bản chất: 
  
        Trong Super Loop, vì chỉ có 1 luồng thực thi nên không xảy ra xung đột khi truy cập biến toàn cục,UART,I2C,...

        Tuy nhiên, khi sử dụng Ngắt (Interrupt), vấn đề này xuất hiện. Một biến được sửa trong hàm ngắt và cũng được đọc trong main() có thể dẫn đến race condition nếu không được bảo vệ (bằng cách tắt ngắt - rất nguy hiểm)
        
        FreeRTOS cung cấp các cơ chế (Semaphore, Mutex) để giải quyết vấn đề này một cách an toàn và hệ thống.


##### **2.1.3.Giải pháp**

* **Loại bỏ "Blocking Code" bằng cách chia nhỏ thành các "Task" độc lập.**

        void vTaskSensor(void *pvParameters) {
            for (;;) { // Vòng lặp vô tận của task Sensor
                read_sensor(); // Hàm này vẫn chậm 50ms, NHƯNG...
                vTaskDelay(pdMS_TO_TICKS(100)); // ...nó chỉ "block" chính task Sensor này.
            }
            // Trong 50ms đó, các task khác VẪN CHẠY BÌNH THƯỜNG.
        }

        void vTaskDisplay(void *pvParameters) {
            for (;;) {
                update_display();
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        void vTaskButton(void *pvParameters) {
            for (;;) {
                check_button(); // Task này chạy rất nhanh và thường xuyên
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        int main(void) {
            // 1. Khai báo các task với các độ ưu tiên khác nhau
            xTaskCreate(vTaskSensor,  "Sensor",  128, NULL, 1, NULL); // Ưu tiên 1
            xTaskCreate(vTaskDisplay, "Display", 128, NULL, 1, NULL); // Ưu tiên 1
            xTaskCreate(vTaskButton,  "Button",  128, NULL, 2, NULL); // Ưu tiên 2 (CAO HƠN)

            // 2. Giao quyền điều khiển cho Kernel
            vTaskStartScheduler();

            // Sẽ không bao giờ chạy đến đây
            for (;;) {}
        }

  ◦ Scheduler của FreeRTOS luôn tìm task có trạng thái Ready (sẵn sàng) và có mức ưu tiên cao nhất để trao quyền sử dụng CPU.

  ◦ Khi một task gọi vTaskDelay(), nó tự chuyển mình sang trạng thái Blocked (bị khóa). 
  
  ◦ Lúc này, Scheduler ngay lập tức chọn một task khác đang sẵn sàng để chạy. CPU không bao giờ "bị chôn" trong một task.

  ◦ Nếu một task ưu tiên cao (như vTaskButton) trở nên sẵn sàng, nó sẽ ngay lập tức cướp quyền (preempt) CPU từ một task ưu tiên thấp hơn đang chạy.

* **Đảm bảo Real-Time và Preemption**

  ◦ Thời gian phản hồi xác định: Giả sử có một task xử lý khẩn cấp vTaskEmergency với độ ưu tiên 3 (cao nhất). 
        
  ◦ Khi sự kiện khẩn cấp xảy ra, task này sẽ được kích hoạt. 

  ◦ Bộ điều phối sẽ ngay lập tức dừng task đang chạy (dù là vTaskSensor đang trong 50ms đọc cảm biến) và chuyển CPU cho vTaskEmergency. 
        
  ◦ Điều này đảm bảo phản hồi khẩn cấp là cực nhanh và có thể dự đoán được.


### **III.Các thành phần chính của FreeRTOS**

#### **3.1. Task (Tác vụ)**

##### **3.1.1.Khái niệm** 

* Task là đơn vị thực thi độc lập, có ngăn xếp riêng, ưu tiên, và trạng thái.

        // MÔ HÌNH SUPER LOOP ĐIỂN HÌNH
        void main(void) {
            hardware_init(); // Khởi tạo phần cứng

            while (1) { // Vòng lặp chính - "Super Loop"
                read_sensor();       // 50ms → block toàn bộ hệ thống
                process_data();      // 10ms
                update_display();    // 20ms
                check_button();      // 1ms
                // Tổng thời gian 1 vòng lặp: 81ms
            }
        }

##### **3.1.2.Đặc điểm** 

* **Cấu trúc TCB (Task Control Block):**

        typedef struct tskTaskControlBlock {
            StackType_t *pxTopOfStack;
            ListItem_t xStateListItem;
            UBaseType_t uxPriority;
            StackType_t *pxStack;
            char pcTaskName[configMAX_TASK_NAME_LEN];
            // ...
        } tskTCB;


* **Tạo Task:**

        BaseType_t xTaskCreate(
            TaskFunction_t pvTaskCode,     // Hàm task
            const char *pcName,            // Tên (debug)
            uint16_t usStackDepth,         // Số word (không phải byte!)
            void *pvParameters,            // Tham số truyền vào
            UBaseType_t uxPriority,        // Ưu tiên (0 thấp nhất)
            TaskHandle_t *pxCreatedTask    // Handle (NULL nếu không cần)
        );



* **VD: Blink LED**

        /* Private function prototypes -----------------------------------------------*/
        void RCC_Configuration(void);
        void GPIO_Configuration(void);
        void vBlinkTask(void *pvParameters);

        /* Task definitions ----------------------------------------------------------*/
        /**
        * @brief  Task nhấp nháy LED
        */
        void vBlinkTask(void *pvParameters)
        {
            for(;;) {
                // Đảo trạng thái LED
                GPIO_WriteBit(LED_PORT, LED_PIN, 
                            (BitAction)(1 - GPIO_ReadOutputDataBit(LED_PORT, LED_PIN)));
                
                // Delay 1000ms - sử dụng FreeRTOS delay
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        /**
        * @brief  Cấu hình clock hệ thống
        */
        void RCC_Configuration(void)
        {
            // Bật clock cho GPIOC
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        }

        /**
        * @brief  Cấu hình GPIO cho LED
        */
        void GPIO_Configuration(void)
        {
            GPIO_InitTypeDef GPIO_InitStructure;
            
            // Cấu hình chân LED (PC13)
            GPIO_InitStructure.GPIO_Pin = LED_PIN;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // Output push-pull
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
            GPIO_Init(LED_PORT, &GPIO_InitStructure);
        }

        /**
        * @brief  Hàm main
        */
        int main(void)
        {
            // Cấu hình hệ thống
            RCC_Configuration();
            GPIO_Configuration();
            
            // Tạo task blink LED
            xTaskCreate(vBlinkTask,          // Hàm thực thi task
                        "Blink",             // Tên task
                        128,                 // Kích thước stack
                        NULL,                // Tham số
                        1,                   // Độ ưu tiên
                        NULL);               // Task handle
            
            // Khởi động scheduler - KHÔNG BAO GIỜ TRẢ VỀ
            vTaskStartScheduler();
            
            // Nếu đến đây có nghĩa là có lỗi
            while(1) {
                // Xử lý lỗi ở đây
            }
        }

* **Trạng thái Task:**

| Trạng thái | Mô tả |
|------------|-------|
| **Running** | Đang chạy trên CPU |
| **Ready** | Sẵn sàng chạy, chờ CPU |
| **Blocked** | Đang chờ queue, semaphore, delay |
| **Suspended** | Bị tạm dừng bởi vTaskSuspend() |

#### **3.2.Queue (Hàng đợi) – Giao tiếp Task**

##### **3.2.1.Khái niệm** 

* Queue là cơ chế an toàn luồng (thread-safe) để truyền dữ liệu giữa các task hoặc ISR → task.

##### **3.2.2.Đặc điểm** 

* **Tạo Queue:**

        QueueHandle_t xQueueCreate(
            UBaseType_t uxQueueLength,   // Số phần tử
            UBaseType_t uxItemSize       // Kích thước mỗi phần tử (byte)
        );


* **Ví dụ: Producer – Consumer:**

        QueueHandle_t xDataQueue;

        void vProducer(void *pv) {
            uint32_t data = 0;
            for (;;) {
                xQueueSend(xDataQueue, &data, portMAX_DELAY);
                data++;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void vConsumer(void *pv) {
            uint32_t received;
            for (;;) {
                if (xQueueReceive(xDataQueue, &received, portMAX_DELAY) == pdPASS) {
                    printf("Received: %lu\n", received);
                }
            }
        }

        // main()
        xDataQueue = xQueueCreate(10, sizeof(uint32_t));
        xTaskCreate(vProducer, "Prod", 128, NULL, 1, NULL);
        xTaskCreate(vConsumer, "Cons", 128, NULL, 1, NULL);



* **Queue từ ISR**

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(xQueue, &data, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

#### **3.3.Semaphore (Tín hiệu) – Đồng bộ hóa**

##### **3.3.1.Khái niệm** 

* Semaphore là một cơ chế đồng bộ hóa trong lập trình đa nhiệm, dùng để kiểm soát truy cập vào tài nguyên dùng chung giữa các task hoặc đồng bộ hoạt động giữa chúng.

  ◦ Được định nghĩa trong: `semphr.h`
        
  ◦ Tất cả API đều thread-safe và ISR-safe (nếu dùng `_FromISR`)

  ◦ Dùng chung `SemaphoreHandle_t` làm kiểu handle

##### **3.3.2.Phân loại Semaphore** 

* **Binary Semaphore:**

  ◦ Chỉ có 2 giá trị: 0 (không có) hoặc 1 (có)
        
  ◦ Dùng để đồng bộ sự kiện giữa các task

  ◦ Dùng để bảo vệ tài nguyên đơn giản

  ◦ **API Tạo Binary Semaphore**

        SemaphoreHandle_t xSemaphoreCreateBinary( void );

    | Giá trị trả về | Mô tả |
    |----------------|-------|
    | `SemaphoreHandle_t` | Handle hợp lệ |
    | `NULL` | Tạo thất bại (hết RAM) |


  ◦ **API Give (Báo hiệu)**
        
        Từ Task:
        BaseType_t xSemaphoreGive( SemaphoreHandle_t xSemaphore );

        Từ ISR:
        BaseType_t xSemaphoreGiveFromISR( 
            SemaphoreHandle_t xSemaphore, 
            BaseType_t *pxHigherPriorityTaskWoken 
        );  

    | Tham số | Mô tả |
    |---------|-------|
    | `xSemaphore` | Handle của semaphore |
    | `pxHigherPriorityTaskWoken` | `pdTRUE` nếu cần `portYIELD_FROM_ISR()` |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdPASS` | Thành công |
    | `pdFAIL` | Thất bại (đã đầy) |

  ◦ **API Take (Chờ tín hiệu)**
        
        Từ Task:
        BaseType_t xSemaphoreTake( 
            SemaphoreHandle_t xSemaphore, 
            TickType_t xTicksToWait 
        );

        Từ ISR:
        BaseType_t xSemaphoreTakeFromISR( 
            SemaphoreHandle_t xSemaphore, 
            BaseType_t *pxHigherPriorityTaskWoken 
        );


    | Tham số | Mô tả |
    |---------|-------|
    | `xTicksToWait` | Thời gian chờ (tick): `portMAX_DELAY` = chờ mãi |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdTRUE` | Lấy được semaphore |
    | `pdFALSE` | Hết thời gian chờ |


  ◦ Ví dụ đầy đủ: Đồng bộ ISR → Task


        SemaphoreHandle_t xBinarySemaphore;

        void EXTI0_IRQHandler(void) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        void vButtonTask(void *pvParameters) {
            xBinarySemaphore = xSemaphoreCreateBinary();
            for (;;) {
                if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
                    printf("Button pressed!\n");
                }
            }
        }

* **Counting Semaphore:**

  ◦ Giá trị từ 0 đến N (N > 1)
        
  ◦ Dùng để quản lý nhiều instance của tài nguyên (buffer, connection, slot...)


  ◦ **API Tạo Counting Semaphore**

        SemaphoreHandle_t xSemaphoreCreateCounting( 
            UBaseType_t uxMaxCount, 
            UBaseType_t uxInitialCount 
        );

    | Tham số | Mô tả |
    |---------|-------|
    | `uxMaxCount` | Giá trị tối đa (N) |
    | `uxInitialCount` | Giá trị khởi tạo (≤ N) |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    |  Handle | Thành công |
    | `NULL` | Thất bại |


  ◦ **API Give – Tăng giá trị semaphore**
        
        Từ Task (dùng được cho Counting, Binary, Mutex)

        BaseType_t xSemaphoreGive( SemaphoreHandle_t xSemaphore );

    | Tham số | Mô tả |
    |---------|-------|
    | `xSemaphore` | Handle của semaphore |


    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdPASS` | Tăng thành công |
    | `pdFAIL` | Thất bại: đã đạt uxMaxCount (chỉ với Counting) |


        Từ ISR (chỉ dùng cho Counting và Binary – KHÔNG dùng cho Mutex)

        BaseType_t xSemaphoreGiveFromISR( 
        SemaphoreHandle_t xSemaphore, 
        BaseType_t *pxHigherPriorityTaskWoken 
    );


  ◦ **API Take (Chờ tín hiệu) – Giảm giá trị semaphore**
        
        Từ Task:
        BaseType_t xSemaphoreTake( 
            SemaphoreHandle_t xSemaphore, 
            TickType_t xTicksToWait 
        );

        Từ ISR:
        BaseType_t xSemaphoreTakeFromISR( 
            SemaphoreHandle_t xSemaphore, 
            BaseType_t *pxHigherPriorityTaskWoken 
        );


    | Tham số | Mô tả |
    |---------|-------|
    | `xTicksToWait` | Thời gian chờ (tick): `portMAX_DELAY` = chờ mãi |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdTRUE` | Lấy được (giảm thành công) |
    | `pdFALSE` | Hết thời gian chờ |


  ◦ Ví dụ đầy đủ: Quản lý 3 buffer slot


        #define BUFFER_SLOTS  3
        SemaphoreHandle_t xBufferSemaphore;

        void vProducer(void *pv) {
            xBufferSemaphore = xSemaphoreCreateCounting(BUFFER_SLOTS, BUFFER_SLOTS);
            for (;;) {
                if (xSemaphoreTake(xBufferSemaphore, portMAX_DELAY) == pdTRUE) {
                    write_to_buffer();  // Ghi vào 1 slot
                    vTaskDelay(100);
                }
            }
        }

        void vConsumer(void *pv) {
            for (;;) {
                process_data();  // Xử lý dữ liệu
                xSemaphoreGive(xBufferSemaphore);  // Trả lại 1 slot
                vTaskDelay(150);
            }
        }



* **Mutex:**

  ◦ Là binary semaphore có cơ chế ưu tiên
        
  ◦ Ngăn hiện tượng priority inversion

  ◦ Dùng để bảo vệ tài nguyên quan trọng


  ◦ **API Tạo Mutex**
        
        SemaphoreHandle_t xSemaphoreCreateMutex( void );

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `SemaphoreHandle_t (handle hợp lệ)` | Tạo thành công, mutex sẵn sàng (available) |
    | `NULL` | Tạo thất bại (hết bộ nhớ heap) |

        xMutex = xSemaphoreCreateMutex();
        if (xMutex == NULL) {
            // Xử lý lỗi: không đủ RAM
        }


  ◦ **API Take – Chiếm mutex**
        
        BaseType_t xSemaphoreTake( 
            SemaphoreHandle_t xMutex, 
            TickType_t xTicksToWait 
        );


    | Tham số | Mô tả |
    |---------|-------|
    | `xMutex` | Handle của mutex |
    | `xTicksToWait` | Thời gian chờ tối đa (tick) 
 portMAX_DELAY = chờ mãi |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdTRUE` | Chiếm được mutex |
    | `pdFALSE` | Hết thời gian chờ |

  ◦ **API Give – Nhả mutex**
        
        BaseType_t xSemaphoreGive( SemaphoreHandle_t xMutex );


    | Tham số | Mô tả |
    |---------|-------|
    | `xMutex` | Handle của mutex |

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `pdPASS` | Nhả thành công |
    | `pdFAIL` | Lỗi (gọi từ task không giữ mutex) |


* **Recursive Mutex (Mutex lồng nhau):**

  ◦ Cho phép cùng một task gọi Take nhiều lần


  ◦ **API Tạo Mutex**
        
        SemaphoreHandle_t xSemaphoreCreateRecursiveMutex( void );

    | Giá trị trả về | Mô tả |
    |---------------|-------|
    | `SemaphoreHandle_t (handle hợp lệ)` | Tạo thành công, mutex sẵn sàng (available) |
    | `NULL` | Tạo thất bại (hết bộ nhớ heap) |

        xMutex = xSemaphoreCreateMutex();
        if (xMutex == NULL) {
            // Xử lý lỗi: không đủ RAM
        }


  ◦ **API Take/ Give Recursive**
        
        BaseType_t xSemaphoreTakeRecursive( 
            SemaphoreHandle_t xMutex, 
            TickType_t xTicksToWait 
        );

        BaseType_t xSemaphoreGiveRecursive( SemaphoreHandle_t xMutex );


 
 </details>
