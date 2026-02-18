
# CHƯƠNG 3: HEAP MANAGEMENT 
<details>
    <summary><strong>BÀI 1: HEAP INTRODUCTION, HEAP SCHEMES</strong></summary>

## **BÀI 1: HEAP INTRODUCTION, HEAP SCHEMES**

### **I.  Heap Introduction**

#### **1.1. Khái niệm**

*	Trong hệ điều hành thời gian thực FreeRTOS, bộ nhớ đóng vai trò quyết định đến tính ổn định và độ tin cậy của hệ thống
  
*	Heap là vùng bộ nhớ được FreeRTOS sử dụng để cấp phát động cho các đối tượng kernel trong quá trình chạy.
  
*	Việc quản lý heap không hiệu quả có thể dẫn đến các lỗi nghiêm trọng như không tạo được task, hệ thống treo hoặc reset bất ngờ.
  
*   Trong FreeRTOS, heap là vùng RAM dành riêng cho cơ chế cấp phát động, được sử dụng khi hệ thống cần tạo các đối tượng kernel tại runtime.

*   Heap khác với stack của từng task và khác với vùng dữ liệu tĩnh/global, các thành phần quan trọng sử dụng heap bao gồm:
 
    ◦   **Task Control Block (TCB):** cấu trúc quản lý thông tin task

    ◦   **Task Stack:** vùng stack riêng cho từng task nếu không cấp phát tĩnh
 
    ◦   **Queue và Message Buffer:** phục vụ truyền dữ liệu giữa các task

    ◦   **Semaphore và Mutex:** đồng bộ truy cập tài nguyên
 
    ◦   **Software Timer và Event Group:** quản lý sự kiện và thời gian
        
*  FreeRTOS hỗ trợ hai chiến lược quản lý bộ nhớ chính:

    ◦   **Dynamic Allocation (Heap-based)**

    ◦   **Static Allocation (User-provided buffers)**

 
#### **1.2. Vai trò**
 
* 	Heap đóng vai trò nền tảng trong việc khởi tạo và vận hành các đối tượng kernel của FreeRTOS trong quá trình runtime.

*  	Khi lập trình viên tạo task hoặc các object giao tiếp, FreeRTOS sẽ tự động sử dụng heap để cấp phát vùng nhớ cần thiết.

* 	Ví dụ, khi gọi hàm: `xTaskCreate(...)` thì Kernel sẽ cấp phát bộ nhớ heap để tạo:

    ◦   vùng TCB
  	
    ◦   vùng stack cho task

* 	Tương tự, khi gọi: `xQueueCreate(...)`, FreeRTOS cũng sẽ cấp phát heap để khởi tạo queue object.

#### **1.3. Rủi ro**

##### **1.3.1. Heap Exhaustion**
 
* 	Heap exhaustion xảy ra khi hệ thống không còn đủ bộ nhớ trống để cấp phát thêm đối tượng mới

*  	Khi đó, các hàm cấp phát như `pvPortMalloc()` sẽ trả về NULL, dẫn đến việc tạo task hoặc queue thất bại.

* 	Hậu quả của heap exhaustion có thể bao gồm:

    ◦   Task không được khởi tạo

    ◦   Hệ thống mất chức năng quan trọng

    ◦   Reset hoặc crash bất ngờ

##### **1.3.2. Memory Fragmentation**
 
* 	Fragmentation là hiện tượng heap bị chia nhỏ thành nhiều block rời rạc sau nhiều lần cấp phát và giải phóng.

* 	Ví dụ:

    ◦   Tổng heap còn trống 5000 byte

    ◦   Nhưng block liên tục lớn nhất chỉ còn 200 byte

    ◦   Task cần stack 512 byte → cấp phát thất bại


#### **1.4. Đặc điểm**
 
* 	Heap trong FreeRTOS không phải heap chuẩn của thư viện C.

*  	FreeRTOS không bắt buộc sử dụng `malloc()` và `free()` của libc mà cung cấp cơ chế quản lý heap riêng phù hợp hơn cho các hệ thống nhúng.

* 	FreeRTOS hỗ trợ nhiều heap scheme khác nhau thông qua các file:

    ◦   heap_1.c

    ◦   heap_2.c

    ◦   heap_3.c

    ◦   heap_4.c

    ◦   heap_5.c

* 	Dung lượng heap được cấu hình trong `FreeRTOSConfig.h`:

		#define configTOTAL_HEAP_SIZE (10 * 1024)

    ◦   Đơn vị tính: byte

    ◦   Heap này được kernel sử dụng để cấp phát các object nếu bật dynamic allocation
  	
### **II.  Heap Management Schemes**

####  **2.1. Heap_1**

* **Khái niệm:**
	
	*  Heap_1 là scheme đơn giản nhất, thực chất là một **subdivision allocator (bộ cấp phát phân nhỏ dần)**
	
	*  Sử dụng một mảng tĩnh duy nhất (kích thước định nghĩa bới `configTOTAL_HEAP_SIZE`
		
	* Chỉ hỗ trợ **allocate** (cấp phát bộ nhớ), không hỗ trợ **free/deallocation** (giải phóng bộ nhớ)

* **Cơ chế hoạt động:**

	* **Allocation (cấp phát):**
		* Tìm khối trống tiếp theo và phân nhỏ để trả về con trỏ.
		
		*  Mỗi khối có header nhỏ (thường 8 byte) lưu kích thước.

	* **Deallocation (giải phóng):**
		* Không hỗ trợ, bộ nhớ đã cấp phát không bao giờ thu hồi.
		
* **Đặc điểm:**
	
	*  Thời gian cấp phát luôn cố định
	
	*  Không bao giờ phân mảnh bộ nhớ
		 
	*  Không hỗ trợ `vPortFree()`
	
* **VD1:** `configTOTAL_HEAP_SIZE = 10240 byte`
		
	*  Tạo task → cấp phát 1024 byte stack → heap còn 9216 byte liên tục.
	
	*  Tạo task khác → cấp phát 2048 byte → heap còn 7168 byte.
		 
	*  Xóa task đầu → không thu hồi 1024 byte → heap vẫn chỉ còn 7168 byte.

* **VD2:** 
		
			/* Task 1: stack 1024 byte */
			void vTask1(void *pvParameters){
				for(;;)
				{
					vTaskDelay(pdMS_TO_TICKS(1000));
				}
			}
		
		/* Task 2: stack 2048 byte */
		void vTask2(void *pvParameters){
			for(;;)
			{
				vTaskDelay(pdMS_TO_TICKS(1000));
			}
		}
		
		int main(void){
			TaskHandle_t xHandle1;
			TaskHandle_t xHandle2;
			
	    /* Heap ban đầu: 10240 byte */
	    printf("Heap ban dau: %u byte\n", (unsigned)xPortGetFreeHeapSize());
		
		/* Bước 1: Tạo task 1 -> cấp phát stack ~1024 byte */
		xTaskCreate(vTask1, "Task1", 256, NULL, 1, &xHandle1);
	    printf("Sau khi tao Task1: %u byte\n",
           (unsigned)xPortGetFreeHeapSize());

		/* Bước 2: Tạo task 2 -> cấp phát stack ~2048 byte */
		xTaskCreate(vTask2, "Task2", 512, NULL, 1, &xHandle2);
	    printf("Sau khi tao Task2: %u byte\n",
           (unsigned)xPortGetFreeHeapSize());
       
       /* Bước 3: Xóa task 1 */
       vTaskDelete(xHandle1);
       printf("Sau khi xoa Task1: %u byte\n", (unsigned)xPortGetFreeHeapSize());
       vTaskStartScheduler();
       while(1);
       }
####  **2.2. Heap_2**

* **Khái niệm:**
	
	*  Heap_2 được xây dựng nhằm mở rộng `heap_1` bằng cách bổ sung:

		*  Cấp phát bộ nhớ động (`pvPortMalloc`)
		
		* Giải phóng bộ nhớ (`vPortFree`)	
	
	* Khác với `heap_1` chỉ allocate tuyến tính, `heap_2` duy trì một danh sách các vùng nhớ trống để tái sử dụng các block đã được free
		
* **Cơ chế hoạt động:**
	
	*  **Allocation (cấp phát):** 
	
		* Khi gọi:
				
				pvPortMalloc(size)		
	 
		* Kernel sẽ:
	
			* Duyệt danh sách các block trống 
			* Tìm một block đủ lớn để cấp phát
			* Có thể split block nếu block lớn hơn yêu cầu
		
	*  **Deallocation (giải phóng):** 
	
		* Khi gọi:
				
				vPortFree(ptr)
	 
		* Kernel sẽ:
	
			* Đưa block vừa free trở lại free list 
			* Block được đánh dấu là trống

* **Đặc điểm:**
	
	*  Không có cơ chế gộp các khối trống liền kề, vì vậy bộ nhớ rất dễ bị chia nhỏ thành nhiều mảnh rời rạc theo thời gian
	
	*  Thời gian cấp phát không cố định, do hệ thống phải duyệt qua danh sách các khối trống để tìm vùng phù hợp
		 
	*  Bộ nhớ bị phân mảnh tăng nhanh nếu việc cấp phát và giải phóng diễn ra với nhiều kích thước khác nhau
	
	*  Không đảm bảo được tính ổn định về thời gian thực, vì thời gian cấp phát phụ thuộc vào số lượng khối trống trong heap
	
* **VD1:**
	* Giả sử heap ban đầu có 10000 byte liên tục:
			
				[ 10000 bytes free ]

		*  **Bước 1: Allocate 2000 byte**
		
				[ 2000 used ][ 8000 free ]

		*  **Bước 2: Allocate thêm 3000 byte**
		
				[ 2000 used ][ 3000 used ][ 5000 free ]

		*  **Bước 3: Free block đầu tiên (2000 byte)**
		
				[ 2000 free ][ 3000 used ][ 5000 free]

				// Tổng free là 2000 + 5000 = 7000 byte nhưng heap bị chia thành 2 block rời rạc

		
		*  **Bước 4: Allocate 6000 byte**
		
				// heap_2 không thể gộp 2 block free lại

				// không block nào đủ lớn hơn 6000 byte dẫn tới allocation thất bại

* **VD2:**
	
			
				void vHeap2FragmentationDemo(void){
					void *p1;
					void *p2;
					void *p3;
					
					/* Allocation 2000 bytes */
					p1 = pvPortMalloc(2000);

					/* Allocation 3000 bytes */
					p2 = pvPortMalloc(3000);

					/* Free the first block */
					vPortFree(p1);

					/* Now heap has two free blocks: 2000 and 5000 */
					
					/* Try allocating 6000 bytes */
					
					p3 = pvPortMalloc(6000);
					if (p3 == NULL){
						printf("Heap_2 allocation failed: fragmentation occurred!\n");
					}
				}

####  **2.3. Heap_3**

* **Khái niệm:**
	
	*  Heap_3 đóng vai trò như một lớp bao bọc (wrapper) để sử dụng trực tiếp các hàm

		*  `malloc()`
		
		* `free()`	
		
* **Cơ chế hoạt động:**
	
	*  Trong `heap_3`, các hàm cấp phát của FreeRTOS thực chất chỉ gọi lại hàm chuẩn của C:
	
		* `pvPortMalloc()` sẽ gọi `malloc()`
		* `vPortFree()` sẽ gọi `free()`

	 
	* Trước khi gọi, kernel sẽ tạm thời khóa scheduler hoặc vào vùng critical section để tránh 2 task gọi malloc cùng lúc
	
			void *pvPortMalloc(size_t xSize)
			{
				taskENTER_CRITICAL();
				void *ptr = malloc(xSize);
				taskEXIT_CRITICAL();
				
				return ptr;
			}
			
			void vPortFree(void *pv){
				taskENTER_CRITICAL();
				free(pv);
				taskEXIT_CRITICAL();
			}

* **Đặc điểm:**
	
	*  Không dùng `configTOTAL_HEAP_SIZE` , dung lượng heap sẽ phụ thuộc vào: linker script, C library
	
	* Phụ thuộc hoàn toàn vào malloc/free của compiler
		 
	*  Các hàm `malloc()` thường có thời gian chạy không cố định dẫn tới không phù hợp với hệ thống thời gian thực nghiêm ngặt
	
	*  Chỉ phù hợp với các môi trường giả lập hoặc các dự án thử nghiệm
	
* **VD:**
	
			
				void vExampleTask(void *pvParameters){
				
					char *buffer;
					
					/* Cấp phát 256 byte từ malloc() của hệ thống */
					buffer = (char *)pvPortMalloc(256);
					if (buffer == NULL){
						printf("Khong du bo nho!\n");
					}
					else
					{
						strcpy(buffer, "FreeRTOS heap_3 example");
						printf("%s\n", buffer);
						
						vPortFree(buffer);
					}
					vTaskDelete(NULL);
				}	
					
####  **2.4. Heap_4**

* **Khái niệm:**
	
	*  Heap_4 là sơ đồ quản lý bộ nhớ được sử dụng phổ biến nhất trong FreeRTOS vì:

		*  Hiệu quả trong sử dụng RAM
		
		* Khả năng cấp phát giải phóng linh hoạt

		* Giảm phân mảnh bộ nhớ

	*  Hỗ trợ đầy đủ hai cơ chế quan trọng:
	
		* Tách một khối trống lớn thành khối nhỏ vừa đủ khi cấp phát 
		
		* Hợp nhất các khối trống liền kề khi giải phóng
		 		
* **Cơ chế hoạt động:**
	
	*  **Khi cấp phát (Allocation):**
	
		* Khi gọi `pvPortMalloc(size)`, kernel sẽ thực hiện:
				
				1. Duyệt danh sách các khối trống 
				2. Tìm khối đầu tiên có kích thước đủ lớn
				3. Nếu khối đó lớn hơn kích thước cần thiết, tách ra 
				4. Cấp phát phần vừa đủ, phần dư vẫn giữ lại làm block trống 
				
		* VD:
				
				block trống 5000 byte
				yêu cầu 2000 byte
				heap_4 sẽ tách thành:
					[2000 used][3000 free]

	 
	*  **Khi giải phóng (Deallocation):**
	
		* Khi gọi `vPortFree(ptr)`, kernel sẽ thực hiện:
				
				1. Đánh dấu block đó là trống 
				2. Kiểm tra block ngay trước và ngay sau
				3. Nếu các block liền kề cũng trống, tự động kết hợp 
				4. Tạo thành một block lớn liên tục 
				
		* VD:
				
				[2000 free][3000 free]
				// Sau hợp nhất trở thành 
				[5000 free]

* **Đặc điểm:**
	
	*  Hỗ trợ cấp phát và giải phóng động đầy đủ (`pvPortMalloc`/`vPortFree`)
	
	* Thời gian cấp phát không hoàn toàn cố định, vì phải duyệt danh sách block trống
		 
	*  Cấp phát hiệu quả ngay cả khi kích thước object thay đổi liên tục
	
* **VD1:**
	
	* Heap ban đầu:
			
			[10000 free]
	
	* Cấp phát 2000 byte

			[2000 used][8000 free]
		 
	*  Cấp phát thêm 3000 byte

			[2000 used][3000 used][ 5000 free]
			
	* Giải phóng block đầu tiên (2000 byte)

			[2000 free][3000 used][5000 free]
		 
	*  Giải phóng tiếp block 3000 byte

			[2000 free][3000 free][ 5000 free]
			
			// heap_4 ngay lập tức thực hiện hợp nhất
			
			[10000 free]

	*  Cấp phát 6000 byte

			// Vì heap đã được hợp nhất thành một block liên tục lớn:
			
			pvPortMalloc(6000);
			
			// Thành công
			[6000 used][4000 free]

* **VD2:**
			
			void Heap4Example(void){
				void *p1 = pvPortMalloc(2000);
				void *p2 = pvPortMalloc(3000);
				vPortFree(p1);
				vPortFree(p2);
				void *p3 = pvPortMalloc(6000);
				if (p3 != NULL)
				{
					printf("Cap phat 6000 byte thanh cong!\n");
				}
				else
				{
					printf("Cap phat that bai!\n");
				}
			}

####  **2.5. Heap_5**

* **Khái niệm:**
	
	*  Heap_5 giúp cho phép heap được trải trên nhiều vùng RAM rời rạc, không cần liên tục trong không gian địa chỉ
		 		
* **Cơ chế hoạt động:**
	
	*  **Heap gồm nhiều vùng nhớ**
	
		* Thay vì chỉ có một vùng heap liên tục như `heap_4`, heap_5 cho phép khai báo nhiều vùng:
			*	vùng RAM trong (nhanh, nhỏ)
			*	vùng RAM ngoài (lớn, chậm)
				
	*  **Bắt buộc phải khởi tạo trước khi cấp phát**
	
		* Trước khi gọi bất kỳ hàm nào, người dùng phải khai báo các vùng heap và gọi
	
				vPortDefineHeapRegions()

	*  **Khai báo các vùng bằng mảng HeapRegion_t**
	
		* Người dùng cần cung cấp một mảng các vùng heap, mỗi vùng gồm: 

				Địa chỉ bắt đầu 
				Kích thước vùng
				
		* Các vùng phải được sắp xếp theo thứ tự địa chỉ tăng dần

* **Đặc điểm:**
	
	*  Cho phép heap nằm trên nhiều vùng RAM khác nhau, kể cả không liền kề
	
	* Sau khi khởi tạo, hành vì cấp phát/giải phóng giống heap_4
		 
* **VD:**
	
	*  Giả sử MCU có 2 vùng RAM:
		* SRAM nội: 64 KB tại `0x20000000`
		* SDRAM ngoài: 512 KB tại `0x90000000`
	
			    HeapRegion_t xHeapRegions[] = 
			    {
				   { (uint8_t *)0x20000000UL, 64 * 1024 },			// SRAM nội (nhanh)
				   { (uint8_t *)0x90000000UL, 512 * 1024},			// SDRAM ngoài (lớn)		
				   { NULL, 0 }		// Kết thúc danh sách
				};	
				
				int main(void){
					SystemInit();
					vPortDefineHeapRegions(xHeapRegions);
					xTaskCreate(vTaskApp, "APP", 512, NULL, 2, NULL)L
					vTaskStartScheduler();
					while(1);
				}											
     </details> 

<details>
    <summary><strong>BÀI 2: STATIC ALLOCATION</strong></summary>

## **BÀI 2: STATIC ALLOCATION**

### **I.  Tổng quan**

#### **1.1. Khái niệm**

*   Static Allocation (cấp phát tĩnh) là cơ chế mà toàn bộ bộ nhớ cho các kernel objects được chuẩn bị trước từ các giai đoạn như :
	* **Compile-time:** Giai đoạn biên dịch
	* **Startup:**	 Quá trình khởi động hệ thống
 
*	Trong cơ chế này, kernel không sử dụng heap và không gọi hàm `pvPortMalloc()`
*	Hệ thống chỉ sử dụng các vùng nhớ đã được cung cấp sẵn bởi người dùng


 
#### **1.2. Thành phần**

##### **1.2.1. Giới thiệu**
 
*  Mỗi đối tượng kernel trong FreeRTOS khi được tạo ra đều cần hai nhóm bộ nhớ chính để hoạt động:

	*  **Vùng quản lý nội bộ (Control Block)**
	
	*  **Vùng dữ liệu thực tế (Data Buffer)**
	

##### **1.2.2. Control Block**

* Control Block là cấu trúc dữ liệu nội bộ mà kernel sử dụng để quản lý đối tượng

* Mỗi loại kernel object sẽ có một control block tương ứng:

	*  Task sử dụng **Task Control Block (TCB)**
	
	*  Queue sử dụng **Queue Control Block (QCB)**
	 
	*  Software Timer sử dụng **Timer Control Block** 

* Các cấu trúc này chứa các thông tin quan trọng phục vụ cho scheduling và quản lý runtime, bao gồm:

	*  Trạng thái hoạt động của đối tượng
	
	*  Priority scheduling của task 
	
	*  Con trỏ stack và thông tin ngữ cảnh 
	
	*  Các tham số đồng bộ (semaphore, mutex) 
	
	*  Dữ liệu liên quan đến quản lý hệ thống thời gian thực   

##### **1.2.3. Data Buffer**

* Ngoài vùng quản lý, kernel còn cần một vùng bộ nhớ dữ liệu thực tế để đối tượng có thể hoạt động đúng chức năng

* Data Buffer là vùng làm việc trực tiếp của đối tượng trong runtime 

* VD:

	*  Task cần một vùng stack riêng để lưu biến cục bộ, context và lời gọi hàm
	
	*  Queue cần vùng lưu trữ các item được gửi/nhận 
	
	*  Stream buffer cần bộ như để chứa dữ liệu truyền liên tục 
	
	*  Message buffer cần vùng lưu các gói message  

	*  Task sử dụng **Task Control Block (TCB)**
	
	*  Queue sử dụng **Queue Control Block (QCB)**
 
#### **1.3. Cấu hình**

*  Trong `FreeRTOSConfig.h`:

			#define  configSUPPORT_STATIC_ALLOCATION		1

*  Khi bật macro này, FreeRTOS sẽ cung cấp thêm các hàm có hậu tố Static cho hầu hết các đối tượng:
	* `xTaskCreateStatic()`
	* `xQueueCreateStatic()`
	* `xSemaphoreCreateBinaryStatic()`, `xSemaphoreCreateMutexStatic()`,...
	* `xTimeCreateStatic()`
	* `xEventGroupCreateStatic()`
	*  `xStreamBufferCreateStatic()`, `xMessageBufferCreateStatic()`, ...
	
*   Nếu chỉ dùng static allocation, có thể đặt:

			#define configTOTAL_HEAP_SIZE			0 		// Không cần heap nữa

	
### **II.  Cơ chế cấp phát bộ nhớ**

#### **2.1. Các tiến trình nền trong FreeRTOS Kernel**

*   Trong FreeRTOS, ngay cả khi hệ thống được cấu hình sử dụng hoàn toàn cơ chế cấp phát tĩnh, kernel vẫn phải tạo ra một số task hệ thống nội bộ để scheduler có thể hoạt động đúng

*    Khi bật static allocation, kernel vẫn tự động tạo

	    ◦   **Idle Task** (luôn tồn tại trong mọi hệ thống FreeRTOS)
	     
	    ◦   **Timer Service Task** (được tạo khi `configUSE_TIMERS = 1`)

#### **2.2. Idle Task và cơ chế duy trì scheduler**

*   Idle Task là task có priority thấp nhất và luôn được kernel tạo ra khi scheduler bắt đầu hoạt động

*   Task này đảm bảo rằng hệ thống luôn có ít nhất một task ở trạng thái ready để CPU có thể thực thi khi không còn task ứng dụng nào sẵn sàng chạy.

*   Idle Task còn đảm nhiệm một số chức năng quan trọng:

	*   Thực thi khi toàn bộ các task khác đang bị block hoặc suspended
	
	*   Thu hồi tài nguyên của các task đã bị xóa (task cleanup)
	
	*    Gọi Idle Hook nếu hệ thống bật tùy chọn `configUSE_IDLE_HOOK`
	
#### **2.3. Timer Task và quản lý software timer**

*   Khi hệ thống bật software timer thông qua cấu hình:
			
			#define configUSE_TIMERS		1
			
*   FreeRTOS sẽ tự động tạo thêm Timer Service Task

*   Task này chịu trách nhiệm quản lý toàn bộ software timer trong hệ thống, bao gồm:

	*  Theo dõi thời điểm hết hạn của timer
	
	*  Thực thi các hàm callback tương ứng 
	
	*  Xử lý các lệnh điều khiển timer được gửi từ các task khác
	
####  **2.4.Application Hook**

#####  **2.4.1. Giới thiệu**

*  Trong cấp phát động, kernel tạo task bằng cách cấp phát bộ nhớ từ heap thông qua `pvPortMalloc()`

* Khi hệ thống sử dụng static allocation hoàn toàn, heap không được sử dụng do đó kernel không thể tự cấp phát bộ nhớ cho các task nền.

* FreeRTOS giải quyết vấn đề này bằng cách cung cấp cơ chế **Application Hook**

	* Người dùng phải định nghĩa các hàm đặc biệt để kernel lấy bộ nhớ cần thiết
  
* Hai hook bắt buộc bao gồm:
	* `vApplicationGetIdleTaskMemory()`
	* `vApplicationGetTimerTaskMemory()`
		

#####  **2.4.2. vApplicationGetIdleTaskMemory()**

* Hàm `vApplicationGetIdleTaskMemory()` được kernel gọi tự động khi FreeRTOS khởi tạo scheduler.
	
*  **Cú pháp**

			void vApplicationGetIdleTaskMemory(
					StaticTask_t **ppxIdleTaskTCBBuffer,
					StaticType_t  **ppxIdleTaskStackBuffer,
					uint32_t *pulIdleTaskStackSize
			);

	* **`StaticTask_t **ppxIdleTaskTCBBuffer`**: 
	
		* Đây là con trỏ kép trỏ đến vùng nhớ chứa **Task Control Block (TCB)** của Idle Task.
		
		* Kernel cần TCB để lưu thông tin quản lý task như: 
				
				Trạng thái task
				Priority
				Con trỏ stack
				Thông tin scheduling
	 
		* Kernel cần TCB để lưu thông tin quản lý task như: 
	
				static StaticTask_t xIdleTaskTCB;

		* Sau đó trả địa chỉ cho kernel:
	
				*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

						
	*  **`StackType_t **ppxIdleTaskStackBuffer:`** 
	
		* Đây là con trỏ kép trỏ đến vùng nhớ stack được cấp phát tĩnh cho Idle Task.
		
		* Stack này lưu:
				
				Biến cục bộ trong task
				Địa chỉ return 
				Context CPU khi context switch
	 
		* Người dùng khai báo stack dưới dạng mảng:
	
				static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

		* Trả địa chỉ stack cho kernel:
	
				*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	*  **`uint32_t *pulIdleTaskStackSize:`** 
	
		* Đây là con trỏ trỏ đến biến lưu kích thước stack của Idle Task.
		
		* Giá trị được tính theo **số phần tử StackType_t**, không phải byte. 

				*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;

#####  **2.4.3. vApplicationGetTimerTaskMemory()**

* Khi hệ thống bật software timer (`configUSE_TIMERS = 1`), kernel sẽ tạo thêm **Timer Service Task**.

* Vì không dùng heap, kernel yêu cầu người dùng cung cấp bộ nhớ tĩnh cho task này thông qua hook:

			vApplicationGetTimerTaskMemory()

*  **Cú pháp**

		void vApplicationGetTimerTaskMemory(
		    StaticTask_t **ppxTimerTaskTCBBuffer,
		    StackType_t **ppxTimerTaskStackBuffer,
		    uint32_t *pulTimerTaskStackSize
		);


	* **`StaticTask_t **ppxTimerTaskTCBBuffer`**: 
	
		* Con trỏ kép đến vùng nhớ TCB của Timer Task.
		
		* Timer Task cần TCB riêng để kernel quản lý task này như một task độc lập.
	 
		* Người dùng khai báo: 
	
				static StaticTask_t xTimerTaskTCB;

		* Cung cấp cho kernel:
	
				*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;


						
	*  **`StackType_t **ppxTimerTaskStackBuffer`**: 
	
		* Con trỏ kép đến stack buffer của Timer Task.
		
		* Timer Task thường cần stack lớn hơn Idle Task vì:
				
				Xử lý command queue
				Chạy callback function

		* Khai báo:
	
				static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

		* Cung cấp:
	
				*ppxTimerTaskStackBuffer = uxTimerTaskStack;


	*  **`uint32_t *pulTimerTaskStackSize`** :
	
		* Con trỏ đến biến lưu kích thước stack Timer Task.
		
		* Giá trị lấy từ:
		
				configTIMER_TASK_STACK_DEPTH

#####  **2.4.4. VD**

		void vApplicationGetTimerTaskMemory(
		    StaticTask_t **ppxTimerTaskTCBBuffer,
		    StackType_t **ppxTimerTaskStackBuffer,
		    uint32_t *pulTimerTaskStackSize )
		{
		    static StaticTask_t xTimerTaskTCB;
		    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

		    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
		    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
		    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
		}
			
   </details> 

# CHƯƠNG 4: TASKS MANAGEMENT 
<details>
    <summary><strong>BÀI 1: TASK FUNCTION, TASK CREATION AND STACK DEPTH</strong></summary>

## **BÀI 1: TASK FUNCTION, TASK CREATION AND STACK DEPTH**

### **I.  Task function**
#### **1.1. Khái niệm**

*   Hàm task là hàm C thực thi công việc độc lập của một tác vụ.
 
* Mỗi task được xem như một luồng thực thi (thread) riêng biệt, được kernel quản lý và lập lịch (scheduling)

* Khi một task  được tạo ra, hệ điều hành sẽ cấp phát cho nó:

    ◦   **Stack riêng** để lưu biến cục bộ và context khi chạy
   
    ◦   **Task Control Block (TCB)** để lưu thông tin quản lý (priority, state, stack pointer, ID, …)
  
    ◦   **Context riêng**, cho phép task có thể bị tạm dừng và tiếp tục mà không mất trạng thái

#### **1.2. Đặc điểm**

*   **Task thường phải có vòng lặp vô hạn**

    ◦   Một task trong FreeRTOS **không được kết thúc tự nhiên bằng cách `return`**, bởi vì kernel giả định task sẽ tồn tại trong suốt vòng đời hệ thống.
    
    ◦   Do đó, cấu trúc chuẩn của một task luôn là:
		 
			 void TaskFunction(void *pvParameters)
			 {
			 	 for(;;)
				 {
				 // Do tasks
				 }
			  }
				 
*   **Tham số truyền vào có kiểu `void*`**

    ◦   Task function luôn nhận tham số đầu vào dạng:
				
				void *pvParameters
			
    ◦   VD: Task có thể nhận struct thông qua con trỏ `void*` 
				
				typedef struct {
					int id;
					int period_ms;
					} TaskConfig;
					      
*   **Task return sẽ bị kernel tự xóa**

    ◦   Nếu task kết thúc bằng `return`, FreeRTOS có thể tự động gọi:
				
				vTaskDelete(NULL);

    ◦   Điều này chỉ xảy ra khi:

				#define configUSE_TASK_DELETE 	1

#### **1.3.VD**

				void vSensorTask(void *pvParameters)
				{
					SensorConfig_t *config = (SensorConfig_t *)pvParameters;
					for(;;)
					{
						uint16_t value = ReadSensor(config->pin);
						ProcessData(value);
						vTaskDelay(pdMS_TO_TICKS(100));
					}
				 }

### **II. Task creation**

#### **2.1. Dynamic allocation**

##### **2.1.1. Khái niệm**

*   Trong FreeRTOS, **Dynamic Allocation** là cơ chế tạo task mà hệ điều hành sẽ tự động cấp phát bộ nhớ từ **heap** tại thời điểm runtime.

*   Khi gọi `xTaskCreate()`, kernel sẽ cấp phát:

    ◦   **TCB (Task Control Block)**: cấu trúc quản lý task

    ◦   **Stack riêng**: vùng bộ nhớ lưu biến cục bộ và context của task.

##### **2.1.2. Cú pháp**

	CBaseType_t xTaskCreate(
	    TaskFunction_t pvTaskCode,
	    const char * const pcName,
	    uint16_t usStackDepth,
	    void *pvParameters,
	    UBaseType_t uxPriority,
	    TaskHandle_t *pxCreatedTask
	);


*   `pvTaskCode` – Con trỏ tới hàm task

    ◦   void vTaskFunction(void *pvParameters);

			void vSensorTask(void *pvParameters)
			{
			    for(;;)
			    {
			        // Code xử lý cảm biến
			    }
			}

*   ``pcName`` – Tên task

    ◦   const char * const pcName

    ◦   Tên này dùng để hiển thị trong debugger, trace task
    
			"Sensor"
			"Communication"
			"MotorCtrl"

*   ``usStackDepth`` – Kích thước stack của task

    ◦   uint16_t usStackDepth

    ◦   Đây là số lượng **word**, không phải byte.

    ◦   Stack = `usStackDepth * 4`
    
			256 word → 1024 bytes stack

*   ``pvParameters`` – Tham số truyền vào task

    ◦   void *pvParameters

    ◦   Cho phép truyền dữ liệu cấu hình vào task, ví dụ: Struct config, ID task, Handle peripheral

    ◦   Ví dụ:
        
			xTaskCreate(vSensorTask, "Sensor", 256, &sensorConfig, 3, NULL);

    ◦   Trong task:
        
			SensorConfig_t *cfg = (SensorConfig_t*) pvParameters;

    ◦   Nếu không cần tham số → truyền `NULL`.

*   ``uxPriority`` – Độ ưu tiên của task

    ◦   UBaseType_t uxPriority

    ◦   FreeRTOS scheduling dựa trên priority:
			
			Priority càng cao → task càng được ưu tiên chạy
			
			Task priority thấp sẽ chạy khi CPU rảnh
			
*   ``pxCreatedTask`` – Task Handle (định danh task)

    ◦   TaskHandle_t *pxCreatedTask

    ◦   Handle là ID để quản lý task sau này:
			
			Suspend/Resume
			
			Delete
			
			Notify

			Change priority
			
     ◦   Ví dụ:

			TaskHandle_t xHandle;
			xTaskCreate(..., &xHandle);
 
 ##### **2.1.3. Giá trị trả về**
 
* 	`pdPASS` → tạo task thành công

* 	`errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY` → không đủ heap để cấp phát stack + TCB

 ##### **2.1.4. VD**

			TaskHandle_t xHandle;
			if (xTaskCreate(
					vSensorTask,				// Hàm task
					"Sensor",						// Tên debug
					256,								// Stack size (word)
					&sensorConfig,			// Tham số truyền vào 
					3,									// Priority
					&xHandle						// Handle task
				) != pdPASS)
			{
					// Xử lý lỗi: heap không đủ
			}
			
#### **2.2. Static allocation**

##### **2.2.1. Khái niệm**

*   Trong FreeRTOS, **Static Allocation** là cơ chế tạo task mà **người lập trình phải cung cấp trước bộ nhớ** cho task, thay vì để kernel tự cấp phát từ heap.

*   Khi tạo task theo kiểu static, người dùng phải chuẩn bị sẵn:

    ◦   **Stack buffer** (mảng stack riêng của task)

    ◦   **TCB buffer** (`StaticTask_t`) để kernel lưu thông tin quản lý task

##### **2.2.2. Cú pháp**

		TaskHandle_t xTaskCreateStatic(
		    TaskFunction_t pvTaskCode,
		    const char * const pcName,
		    uint32_t ulStackDepth,
		    void *pvParameters,
		    UBaseType_t uxPriority,
		    StackType_t *puxStackBuffer,
		    StaticTask_t *pxTaskBuffer
		);



*   `pvTaskCode` – Hàm thực thi task

    ◦   `TaskFunction_t pvTaskCode`


			void vTaskFunction(void *pvParameters);


*   ``pcName`` – Tên task

    ◦   `const char * const pcName`

    ◦   Tên này dùng để hiển thị trong debugger, trace task
    
			"Sensor"
			"Communication"
			"MotorCtrl"

*   ``usStackDepth`` – Kích thước stack của task

    ◦   `uint32_t ulStackDepth`

    ◦   Đây là số lượng **word**, không phải byte.

    ◦   Stack = `usStackDepth * 4`
    
			256 word → 1024 bytes stack

*   ``pvParameters`` – Tham số truyền vào task

    ◦   `void *pvParameters`

    ◦   Cho phép truyền dữ liệu cấu hình vào task, ví dụ: Struct config, ID task, Handle peripheral

    ◦   Ví dụ:
        
			xTaskCreate(vSensorTask, "Sensor", 256, &sensorConfig, 3, NULL);

    ◦   Trong task:
        
			SensorConfig_t *cfg = (SensorConfig_t*) pvParameters;

    ◦   Nếu không cần tham số → truyền `NULL`.

*   ``uxPriority`` – Độ ưu tiên của task

    ◦   `UBaseType_t uxPriority`

    ◦   FreeRTOS scheduling dựa trên priority:
			
			Priority càng cao → task càng được ưu tiên chạy
			
			Task priority thấp sẽ chạy khi CPU rảnh
			
*   ``puxStackBuffer`` – Stack buffer do người dùng cấp phát

    ◦   `StackType_t *puxStackBuffer`
			
			static StackType_t xSensorStack[256];

*   ``pxTaskBuffer`` – Bộ nhớ TCB do người dùng cung cấp

    ◦   `StaticTask_t *pxTaskBuffer
`
			
			static StaticTask_t xSensorTCB;
   
 ##### **2.2.3. Giá trị trả về**
 
* 	`TaskHandle_t` → tạo task thành công

* 	`NULL` → lỗi tạo task (buffer không hợp lệ hoặc thiếu cấu hình)

 ##### **2.2.4. VD**

		static StackType_t xSensorStack[256];
		static StaticTask_t xSensorTCB;

		TaskHandle_t xHandle = xTaskCreateStatic(
		    vSensorTask,            // Hàm task
		    "Sensor",               // Tên debug
		    256,                    // Stack depth (word)
		    &sensorConfig,          // Tham số truyền vào
		    3,                      // Priority
		    xSensorStack,           // Stack buffer
		    &xSensorTCB             // TCB buffer
		);

			
### **IV. Stack depth**

#### **4.1. Khái niệm**

*  Trong FreeRTOS, **stack depth** là tham số xác định lượng bộ nhớ RAM được cấp phát riêng cho mỗi task nhằm phục vụ cho quá trình thực thi.

*  Stack của task được sử dụng để lưu trữ:

    ◦   Biến cục bộ (local variables)

    ◦   Địa chỉ trả về của hàm (return address)

    ◦   Call stack khi gọi hàm lồng nhau

    ◦  Context CPU khi xảy ra context switch (register save/restore)

    ◦  Một phần dữ liệu tạm thời khi chạy thư viện (printf, malloc…)

#### **4.2. Đặc điểm**

*    **Stack depth được tính theo đơn vị word:**

        ◦   Trên Cortex-M (32-bit): 1 word = 4 bytes

				usStackDepth = 128
				→ 128 × 4 = 512 bytes RAM

*    **Mỗi task đều có stack riêng biệt:**

        ◦   FreeRTOS là hệ điều hành đa nhiệm, nên mỗi task có vùng stack độc lập.

        ◦   Nếu tạo nhiều task với stack lớn, RAM sẽ nhanh chóng bị tiêu tốn.


			5 task × 512 words  
			→ 5 × 2 KB = 10 KB RAM chỉ cho stack
				

     </details> 


<details>
    <summary><strong>BÀI 2: PRIORITY AND DELAY</strong></summary>

## **BÀI 2: TASK PRIORITY AND DELAY**

### **I.  Task Priority**

#### **1.1. Khái niệm**

*   Trong FreeRTOS, **priority** (độ ưu tiên) là tham số quan trọng nhất quyết định task nào sẽ được scheduler chọn để chạy.
 
* Kernel luôn ưu tiên thực thi:

    ◦   Task ở trạng thái Ready có priority cao nhất.


#### **1.2. Đặc điểm**

*   **Scheduler dựa trên priority**

    ◦   FreeRTOS sử dụng mô hình: Priority-based preemptive scheduling
			
			nếu bật `configUSE_PREEMPTION = 1`	
    
    ◦   Nếu một task priority cao hơn trở thành Ready → task đang chạy sẽ bị ngắt ngay lập tức
		 

				 
*   **Priority có thể thay đổi trong runtime**

    ◦   FreeRTOS cho phép thay đổi priority động, giúp hệ thống linh hoạt trong các tình huống như:
				
				Xử lý sự kiện khẩn cấp

				Tăng ưu tiên tạm thời để hoàn thành deadline

				Điều chỉnh theo chế độ hoạt động
			
    ◦   API sử dụng:
				
				vTaskPrioritySet()

					      
*   **`configMAX_PRIORITIES` ảnh hưởng overhead hệ thống**

			#define configMAX_PRIORITIES   8


    ◦   Giá trị này xác định số mức priority mà kernel hỗ trợ.
				
				Giá trị càng lớn → kernel cần nhiều RAM hơn cho ready list

				Giá trị quá nhỏ → khó phân cấp task hợp lý

    ◦   8 – 16 mức priority là hợp lý

#### **1.3.Thay đổi priority – `vTaskPrioritySet()`**

		void vTaskPrioritySet(
		    TaskHandle_t xTask,
		    UBaseType_t uxNewPriority
		);

   ◦   `xTask`: handle của task cần thay đổi priority

   ◦   `uxNewPriority`: priority mới

   ◦  Nếu truyền `NULL` → thay đổi priority của task hiện tại.
 
 #### **1.4. Lấy priority hiện tại – `uxTaskPriorityGet()`**

		UBaseType_t uxTaskPriorityGet(TaskHandle_t xTask);


   ◦   Trả về priority hiện tại của task được chỉ định

   ◦   Nếu truyền `NULL` → lấy priority của task đang chạy

   ◦  Ví dụ:
	
		UBaseType_t prio = uxTaskPriorityGet(NULL);
        
### **II. Delay**

#### **2.1. vTaskDelay() – Relative delay**

##### **2.1.1. Khái niệm**

*   `vTaskDelay()` là hàm dùng để **block (tạm dừng) task trong một khoảng thời gian tương đối**, tính từ thời điểm hàm được gọi.

*   Khi task gọi `vTaskDelay()`, nó sẽ chuyển sang trạng thái:

    ◦   **Blocked** và chỉ quay trở lại trạng thái Ready sau khi đủ số tick delay.


##### **2.1.2. Đặc điểm**

*   **Delay tương đối (relative)**

    ◦  Thời gian delay bắt đầu được tính ngay tại thời điểm gọi hàm:

		vTaskDelay(200 ticks);
		
    ◦  Task sẽ ngủ 200 tick kể từ lúc gọi.

*   **Chu kỳ thực tế bị lệch (drift)**

    ◦  Do task còn phải thực thi code trước khi delay, nên chu kỳ thực tế sẽ là:

		T_period​=T_execution​ + T_delay​
		
    ◦  Ví dụ: 
    
			Xử lý mất 20 ms
			Delay 200 ms
			Chu kỳ thực tế = 220 ms

    ◦  Qua nhiều vòng lặp, task sẽ bị **drift (trôi thời gian)** so với chu kỳ mong muốn.

*   **Phù hợp với task không yêu cầu chu kỳ chính xác**


 ##### **2.1.3. Cú pháp**
 
		 void vTaskDelay(TickType_t xTicksToDelay);

* 	`xTicksToDelay` → số tick mà task sẽ bị block

* 	Nên dùng `pdMS_TO_TICKS(ms)`
 → để đổi từ mili-giây sang tick.

 ##### **2.1.4. VD**

		for(;;)
		{
		    ToggleLED();
		    vTaskDelay(pdMS_TO_TICKS(200));  // Nhấp nháy mỗi 200 ms
		}

			
#### **2.2. `vTaskDelayUntil()` – Absolute Delay**

##### **2.2.1. Khái niệm**

*   `vTaskDelayUntil()` là hàm delay theo cơ chế **absolute timing**, tức là task sẽ được block cho đến đúng một **mốc thời gian tuyệt đối tiếp theo**, thay vì delay tương đối từ thời điểm gọi.

*   Nó giúp đảm bảo task chạy theo chu kỳ cố định:

    ◦   T_period​=constant

##### **2.2.2. Đặc điểm**

*   **Chu kỳ chính xác, không bị drift**

    ◦   Kernel tự động tính toán phần thời gian đã trễ và bù lại.

*   **Phù hợp cho periodic task real-time**

    ◦   `vTaskDelayUntil()` được dùng trong các tác vụ yêu cầu chu kỳ chính xác như: 

			Sensor sampling định kỳ
			Control loop (PID motor)
			ADC acquisition

*   **Cần lưu lại thời điểm wake-up trước đó**

    ◦   Task phải lưu biến: `TickType_t xLastWakeTime;` để FreeRTOS biết mốc thời gian chuẩn
         
##### **2.2.3. Cú pháp**

			void vTaskDelayUntil(
				TickType_t *pxPreviousWakeTime,
				TickType_t xTimeIncrement
			);



*   `pxPreviousWakeTime` – Con trỏ tới biến tick wakeup trước đó

*   `xTimeIncrement` - Chu kỳ delay mong muốn (theo tick)

*    VD:


			uint16_t ADC_ReadChannel(void)
			{
				ADC_SoftwareStartConvCmd(ADC1, ENABLE);
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
				return ADC_GetConversionValue(ADC1);
			}
			
			void vADCTask(void *pvParameters){
					TickType_t xLastWakeTime;
					const TickType_t xPeriod = pdMS_TO_TICKS(100);
					xLastWakeTime = xTaskGetTickCount();
					for(;;)
					{
						uint16_t adcVal = ADC_ReadChannel();
						ProcessADC(adcVal);
						vTaskDelayUntil(&xLastWakeTime, xPeriod);
					}
			}



     </details> 



<details>
    <summary><strong>BÀI 3: DELETE TASK AND THREAD LOCAL STORAGE</strong></summary>

## **BÀI 3: DELETE TASK AND THREAD LOCAL STORAGE**

### **I.  Delete Task**

#### **1.1. Khái niệm**

*   Trong FreeRTOS, **xóa task (task deletion)** là thao tác loại bỏ một task ra khỏi hệ thống khi task đó không còn cần thiết.
 
* Việc xóa task giúp:

    ◦   Giải phóng tài nguyên RAM đã cấp phát cho task

    ◦   Loại bỏ task khỏi scheduler

    ◦   Tránh chiếm CPU không cần thiết   

* Một task có thể bị xóa theo hai cách:

    ◦   Task khác xóa nó

    ◦   Task tự xóa chính nó (self-delete)
    
#### **1.2. Đặc điểm**

*   **Task bị xóa sẽ không còn được scheduler chạy**

    ◦   Sau khi gọi `vTaskDelete()`, task sẽ bị chuyển sang trạng thái: `Deleted state`
			
*   **Dynamic Allocation Task**

    ◦   Nếu task được tạo bằng: `xTaskCreate()`  thì stack và TCB được cấp phát từ heap.
    
    ◦   Khi task bị xóa:

			FreeRTOS không giải phóng ngay lập tức

			Task sẽ được đưa vào danh sách “waiting termination”

			Idle Task sẽ dọn dẹp và thu hồi heap sau đó		 

				 
*   **Static Allocation Task**

    ◦   Nếu task được tạo bằng: `xTaskCreateStatic()`  thì stack và TCB do người dùng cung cấp.
    
    ◦   Khi task bị xóa:

			Kernel chỉ loại bỏ task khỏi scheduler

			Bộ nhớ stack và TCB không được free, vì đó là static buffer

*   **Không nên xóa task thường xuyên trong hệ real-time**

    ◦   Trong thực tế, task thường tồn tại suốt vòng đời hệ thống.
    
    ◦   Nếu cần bật/tắt chức năng, nên dùng:

			Suspend/Resume

			Event/Notification

			State machine
			
#### **1.3.Cú pháp `vTaskDelete()`**

		void vTaskDelete(TaskHandle_t xTaskToDelete);

   ◦   `xTaskToDelete`: handle của task cần xóa

   ◦   `vTaskDelete(NULL):` Task hiện tại tự xóa chính nó

 
 #### **VD**

		TaskHandle_t xWorkerHandle;
		void vManagerTask(void *pvParameters)
		{
			// Sau một điều kiện nào đó
			vTaskDelete(xWorkerHandle);
		}

*  **Lưu ý:**

   ◦   Không được xóa task đang giữ mutex

   ◦   Không nên xóa task nếu còn tài nguyên liên quan (cần cleanup trước khi delete)

   ◦   Idle Task phải luôn tồn tại
	
        
### **II. Thread-Local Storage (TLS)**

#### **2.1. Khái niệm**

*   Trong FreeRTOS, **Thread-Local Storage (TLS)** là cơ chế cho phép mỗi task có thể lưu trữ một tập các con trỏ dữ liệu riêng, gắn trực tiếp với task đó.

*   Mỗi task sẽ có một mảng con trỏ nội bộ:

    ◦   TLS[0],TLS[1],...,TLS[n−1]

    ◦   Các con trỏ này được truy cập thông qua **index chung**, nhưng giá trị lại **riêng biệt theo từng task**.

#### **2.2. Đặc điểm**

##### **2.2.1. Dữ liệu riêng theo từng task**

*   Nếu hai task cùng truy cập:

		pvTaskGetThreadLocalStoragePointer(NULL, 0);
		
    ◦  Task A sẽ nhận giá trị của Task A

    ◦  Task B sẽ nhận giá trị của Task B
    
##### **2.2.2. Hữu ích trong các ứng dụng task-specific**

*  TLS thường được dùng cho:

    ◦  Logging theo tên task

    ◦  Task-specific configuration
		
    ◦  Context pointer cho driver 
    
    ◦  Lưu handle UART/I2C riêng
		
    ◦  Debugging và tracing

*   **VD:**
	* Task UART dùng TLS[0] = "UART_Task"
	* Task Sensor dùng TLS[0] = "Sensor_Task"


 ##### **2.2.3. Không cấp phát bộ nhớ mới**

*  TLS chỉ lưu **con trỏ**, không tự động cấp phát hay copy dữ liệu.
*  Người dùng phải đảm bảo vùng nhớ mà con trỏ trỏ tới vẫn tồn tại hợp lệ.

 ##### **2.2.4. Index có giới hạn bởi cấu hình hệ thống**

* Số lượng TLS pointer mỗi task được quyết định bởi macro:
			
			configNUM_THREAD_LOCAL_STORAGE_POINTERS

* Nếu đặt = 4, mỗi task có:

		TLS[0..3]
	
#### **2.3. Cấu hình TLS**

*   Trong file `FreeRTOSConfig.h`:

		#define configNUM_THREAD_LOCAL_STORAGE_POINTERS		4

    ◦  Mỗi task sẽ có tối đa 4 slot TLS pointer

    ◦  Tăng giá trị này sẽ làm tăng overhead RAM trên mỗi task
    
#### **2.4. API sử dụng TLS**

*   **Gán giá trị TLS**

			void vTaskSetThreadLocalStoragePointer(
				TaskHandle_t xTask, 
				BaseType_t  xIndex,
				void *pvValue
				);
				
    ◦   `xTask:` task cần gán TLS
    ◦   `NULL:` task hiện tại
    ◦   `xIndex:` vị trí TLS slot (0 -> N-1)
    ◦   `pvValue:` con trỏ dữ liệu muốn lưu        

*   **Lấy giá trị TLS**

			void *pvTaskGetThreadLocalStoragePointer(
				TaskHandle_t xTask,
				BaseType_t xIndex
				);
				
    ◦   Trả về con trỏ đã lưu trong TLS slot tương ứng

			Sensor sampling định kỳ
			Control loop (PID motor)
			ADC acquisition

*   **VD: Logging theo tên task**

    ◦   Khởi tạo TLS trong task
		
		void vTaskInit(void *pvParameters){
			vTaskSetThreadLocalStoragePointer(NULL, 0, (void*)"UART_Task");
			for(;;)
			{
				// Task code
			}
		}

    ◦   Hàm log dùng TLS
		
		void LogData(int value){
			const char *name = 
				(const char *) pvTaskGetThreadLocalStoragePointer(NULL, 0);
			printf("[%s] Value: %d\n", name, value);
		}	

    ◦   Output
		
		// Nếu task UART gọi
		LogData(123)
		
		// Kết quả:
		[UART_Task] Value: 123
		
		// Nếu task sensor gọi:
		[Sensor_Task] Value: 123
		
		-> Logging tự động phân biệt theo task
		
*   **Lưu ý**

    ◦   Không lưu con trỏ tới biến local

		// Sai vì buffer sẽ mất khi ra khỏi scope
		void vTask(void *p){
			char buffer[20];
			vTaskSetThreadLocalStoragePointer(NULL, 0, buffer);
			}

    ◦   Nếu TLS trỏ tới vùng nhớ malloc, người dùng phải tự free trước khi xóa task.

    ◦   TLS slot nên có quy ước rõ ràng
		
		TLS[0] = Task name
		TLS[1] = Config struct
		TLS[2] = Driver handle
		
    
     </details> 



<details>
    <summary><strong>BÀI 4: TASK RUNTIME CHECK</strong></summary>

## **BÀI 4: TASK RUNTIME CHECK**

### **I.  Delete Task**

#### **1.1. vTaskList()**

##### **1.1.1. Khái niệm**

*   Trong FreeRTOS, hàm **`vTaskList()`** dùng để xuất ra danh sách toàn bộ task hiện có trong hệ thống dưới dạng một chuỗi (string).
 
*  Danh sách này thường bao gồm:

    ◦   Tên task

    ◦   Trạng thái task (Running/Ready/Blocked/…)

    ◦   Priority   

    ◦   Stack còn dư

    ◦   Task number (ID)   
 
##### **1.1.2. Cấu hình**
 
* Để sử dụng `vTaskList()`, cần bật các macro trong `FreeRTOSConfig.h`

	    #define  configUSE_TRACE_FACILITY				1
        #define  configUSE_STATS_FORMATTING_FUNCTIONS	1

    ◦   `configUSE_TRACE_FACILITY`: bật cơ chế trace nội bộ để kernel lưu thông tin task

    ◦   `configUSE_STATS_FORMATTING_FUNCTIONS`: cho phép FreeRTOS format thông tin thành chuỗi  
 
 ##### **1.1.3. Cú pháp**
 
		 void vTaskList(char *pcWriteBuffer);
		 
* `pcWriteBuffer`: buffer do người dùng cung cấp để kernel ghi output string vào
*  Buffer phải đủ lớn để chứa danh sách tất cả task.

* **VD:**
				
				char buf[512];
				vTaskList(buf);	 
				printf("Name\tState\tPri\tStack\tNum\n%s", buf);					 
 
     ◦   Output:

		Name          State  Pri  Stack  Num
		-------------------------------------
		IDLE          R      0    120    1
		SensorTask    B      3    200    2
		UARTTask      R      2    180    3
		MotorCtrl     R      5    90     4
				
     
#### **1.2. vTaskGetRunTimeStats()**

##### **1.2.1. Khái niệm**

*   Trong FreeRTOS, hàm `vTaskGetRunTimeStats()`  dùng để thống kê thời gian CPU đã được sử dụng bởi từng task, từ đó tính ra:

    ◦   % CPU usage của mỗi task
 
##### **1.2.2. Cấu hình**
 	
*   **Để sử dụng runtime stats, cần bật các macro trong `FreeRTOSConfig.h`:**
			
			#define configGENERATE_RUN_TIME_STATS			1
			#define configUSE_TRACE_FACILITY					1
			#define configUSE_STATS_FORMATTING_FUNCTIONS		1
    ◦   Nếu task được tạo bằng: `xTaskCreate()`  thì stack và TCB được cấp phát từ heap.
    
*   **Ngoài ra, hệ thống phải cung cấp timer đo thời gian runtime:**


			#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
			#define portGET_RUN_TIME_COUNTER_VALUE()
	 

				 
##### **1.2.3. Cú pháp**

			void vTaskGetRunTimeStats(char *pcWriteBuffer);
 	
*   `pcWriteBuffer`: buffer để ghi bảng thống kê output

*   VD :
		
			char buf[512];
			vTaskGetRunTimeStats(buf);
			printf("Task Runtime Stats:\n%s\n", buf)

*   Output:

		Task            Time        %
		--------------------------------
		IDLE            5000        50%
		SensorTask      2000        20%
		UARTTask        1500        15%
		MotorCtrl       1500        15%


			

#### **1.3. uxTaskGetStackHighWaterMark()**

##### **1.3.1. Khái niệm**

*   Trong FreeRTOS, mỗi task có stack riêng.

*   Hàm `uxTaskGetStackHighWaterMark()` trả về số lượng word stack còn trống nhỏ nhất từng đạt được kể từ khi task được tạo.

*  VD:
	* Nếu task được cấp: 256 words stack và hàm trả về 40 words
	* Task đã từng dùng tối đa: 216 words
	* Nếu số dư quá nhỏ → nguy cơ crash.
 
##### **1.3.2. Cú pháp**
 	
		UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t xTask);
    
*   `xTask`: handle task cần kiểm tra
*   Nếu truyền `NULL`:

			uxTaskGetStackHighWaterMark(NULL);
*   VD: Kiểm tra stack low

		UBaseType_t hw = uxTaskGetStackHighWaterMark(xHandle);

		if (hw < 50)
		{
		    printf("Warning: Stack low!\n");
		}
	 
### **II.  Stack Measure**

#### **2.1. Ước lượng stack ban đầu**

*   Khi thiết kế task, việc chọn stack depth thường bắt đầu bằng ước lượng sơ bộ.
 
			Stack≈Base + LocalVars + CallDepth

    ◦   **Base overhead**: phần tối thiểu cho context + scheduler

    ◦   **Call depth**: độ sâu hàm gọi lồng nhau

    ◦   **Local variables**: biến cục bộ trong task
  
#### **2.2. Phương pháp đo stack thực tế**
 
* High-Water Mark
	* FreeRTOS cung cấp API chuẩn:
		* `uxTaskGetStackHighWaterMark()`


				UBaseType_t freeWords;

				freeWords = uxTaskGetStackHighWaterMark(xHandle);

				printf("Stack remaining: %lu words\n", freeWords);


* Debugger Memory View
	* Ngoài HighWaterMark, có thể kiểm tra stack bằng debugger:
		* Keil uVision
		* STM32CubeIDE
		* IAR Embedded Workbench


* Bật Stack Overflow Detection
	* Trong `FreeRTOSConfig.h`:
	
			#define configCHECK_FOR_STACK_OVERFLOW 2

	* Kernel sẽ gọi hook khi overflow:
	
			void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
			{
			    printf("Stack overflow in task: %s\n", pcTaskName);

			    taskDISABLE_INTERRUPTS();
			    for(;;);
			}

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
