
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

<details>
    <summary><strong>BÀI 3: HEAP MONITORING</strong></summary>

## **BÀI 3: HEAP MONITORING**

### **I.  Free Heap Monitoring**

#### **1.1. xPortGetFreeHeapSize()**

*	 Hàm trả về tổng dung lượng heap còn trống tại thời điểm gọi

*	Cú pháp:

			size_t xPortGetFreeHeapSize(void);

*	Giá trị trả về:

	   ◦   Số byte heap còn lại trong hệ thống

 *	Đặc điểm:

	   ◦   Có mặt trong tất cả heap schemes (`heap_1` đến `heap_5`)

	   ◦   Chỉ phản ánh heap free tại thời điểm hiện tại

	   ◦   Không đánh giá được fragmentation

	   ◦   Thường dùng để log runtime hoặc debug khi tạo object mới

*	VD:

			void vHeapMonitor(void){
				printf("Free Heap: %u bytes\r\n", (unsigned)xPortGetFreeHeapSize());
			}
				    
#### **1.2. xPortGetMinimumEverFreeHeapSize()**

*  Trả về dung lượng heap nhỏ nhất từng được ghi nhận kể từ khi hệ thống bắt đầu chạy

* Cú pháp:

		size_t xPortGetMinimumEverFreeHeapSize(void);

* Đặc điểm:

	*  Chỉ hỗ trợ trong `heap_4` và `heap_5`
	
	*  Là chỉ số quan trọng để đánh giá an toàn bộ nhớ 
	
	* Không bị thay đổi theo thời gian (luôn giữ mức thấp nhất)  
	
* VD:

		void vHeapWatermarkCheck(void){
			printf("Minimum Ever Free Heap: %u bytes\r\n",  (unsigned)xPortGetMinimumEverFreeHeapSize());
		}
	
### **II.  Heap Statistics**

#### **2.1. Fragmentation**

*	 Phân mảnh heap xảy ra khi:
		
		*  Nhiều khối bộ nhớ nhỏ được cấp phát và giải phóng không đồng đều 
		
		* Heap bị chia thành nhiều block trống rời rạc 
		
		* Không còn block đủ lớn để cấp phát đối tượng mới   	

*	VD:

	* Heap còn trống tổng cộng: 5000 bytes
	* Nhưng block lớn nhất chỉ: 200 bytes
	* Khi hệ thống cần cấp phát 400 bytes -> thất bại 	

#### **2.2. vPortGetHeapStats()**

* Hàm này thu thập trạng thái heap hiện tại và ghi vào struct `HeapStats_t`

* Cú pháp:

			void vPortGetHeapStats(HeapStats_t *pxHeapStats);

* Cấu trúc HeapStats_t:

		typedef struct xHeapStats {
			size_t xAvailableHeapSpaceInBytes;
			size_t xSizeOfLargestFreeBlockInBytes;
			size_t xSizeOfSmallestFreeBlockInBytes;
			size_t xNumberOfFreeBlocks;
			size_t xMinimumEverFreeBytesRemaining;
		} HeapStats_t;

* Tham số:

	*  `xAvailableHeapSpaceInBytes:`
	
		* Đây là tổng dung lượng heap còn trống tại thời điểm gọi 
		* Cho biết heap còn bao nhiêu RAM

	
	*  `xSizeOfLargestFreeBlockInBytes`
	
		* Nếu block lớn nhất quá nhỏ -> malloc lớn sẽ fail
		* Chỉ số quan trọng nhất để phát hiện fragmentation
		  
	
	*  `xNumberOfFreeBlocks`
	
		* Cho biết heap đang có bao nhiêu block trống rời rạc.
		* FreeBlocks ít , heap còn liền mạch 
		* FreeBlocks nhiều, heap bị chia nhỏ
	
	*  `xMinimumEverFreeBytesRemaining`
	
		* Mức heap thấp nhất từng đạt được
		* Đánh giá mức độ an toàn dài hạn của hệ thống

* VD:

			void vPrintHeapStats(void){
				HeapStats_t stats;
				vPortGetHeapStats(&stats);
	
				printf("Free Heap: %u bytes\r\n", (unsigned)stats.xAvailableHeapSpaceInBytes);
				printf("Largest Free Block: %u bytes\r\n", (unsigned)stats.xSizeOfLargestFreeBlockInBytes);
				printf("Free Blocks: %u\r\n", (unsigned)stats.xNumberOfFreeBlocks);
			}

	* Giả sử log:
		* Free Heap = 5000 bytes
		* Largest Free Block = 200 bytes
		* Free Blocks = 35
		   
### **III.  Allocation Failure Hook**

#### **3.1. Khái niệm**

*	 FreeRTOS cung cấp cơ chế callback hook , đây là một hàm do người dùng định nghĩa, được kernel gọi ngay khi malloc thất bại
		
			vApplicationMallocFailedHook()
			
*   Hàm này đóng vai trò như một cơ chế cảnh báo runtime, giúp phát hiện lỗi heap ngay lập tức	
		
#### **3.2. Điều kiện kích hoạt Hook**

*  Hook chỉ hoạt động nếu bật cấu hình trong `FreeRTOSConfig.h`

		#define configUSE_MALLOC_FAILED_HOOK 		1


*  Nếu giá trị bằng 0, kernel sẽ không gọi hook 


#### **3.3. Cơ chế hoạt động**

*  Trong `pvPortMalloc()`, FreeRTOS có kiểm tra:

		if (pvReturn == NULL){
			vApplicationMallocFailedHook();
		}

	* Ngay khi allocation fail
	* Hook sẽ được gọi tức thời 
	* Hệ thống được đưa vào trạng thái debug

#### **3.4. VD**

			void vApplicationMallocFailedHook(void){
			taskDISABLE_INTERRUPTS();
			printf("ERROR: Heap exhausted or fragmented!\r\n");
			while(1);
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


# CHƯƠNG 5: TASK NOTIFICATIONS
<details>
    <summary><strong>BÀI 1: ARCHITECTURE OVERVIEW</strong></summary>

## **BÀI 1: ARCHITECTURE OVERVIEW**

### **I.  UNMEDIATED - DIRECT TO TCB**

#### **1.1. Khái niệm**

*	 **Task Notification** là cơ chế đồng bộ và truyền tín hiệu trong đó nhân hệ điều hành ghi trực tiếp vào khối điều khiển tác vụ (**Task Control Block – TCB**) của tác vụ nhận.

*  Cơ chế này không thông qua một đối tượng trung gian như **Queue** hay **Semaphore**.


#### **1.2. Task Control Block (TCB)**

##### **1.2.1. Thành phần quản lý tác vụ**

*  TCB là cấu trúc dữ liệu mà nhân hệ điều hành sử dụng để quản lý toàn bộ vòng đời của một tác vụ.

*  Trong TCB chứa các thông tin cơ bản:

	* Con trỏ ngăn xếp (Stack pointer)
		
	* Mức ưu tiên (Priority)
		
	* Trạng thái thực thi (Task state)
		
	* Danh sách phục vụ bộ lập lịch (Scheduler lists)
		
	* Vùng Notification tích hợp sẵn
	
* Mỗi Task trong FreeRTOS có một cấu trúc:
	
		TCB_t

*  Trong đó:

		typedef struct tskTaskControlBlock
		{
			...
			volatile uint32_t ulNotifiedValue;
			volatile uint8_t ucNotifyState;
			...
		} TCB_t; 

##### **1.2.2. Giá trị thông báo (ulNotifiedValue)**

*  `ulNotifiedValue` là biến 32-bit nằm trong TCB

*   Có thể hoạt động theo nhiều chế độ:
	
	*  Giá trị đơn (single value)
	
	*  Bộ đếm sự kiện (event counter)
	
	*  Tập bit sự kiện (bitwise event flags)
	
*   Vì biến này tồn tại cố định trong TCB nên:

	*   Không cần cấp phát heap
	
	*   Không cần buffer riêng
	
	*   Không cần copy dữ liệu qua object trung gian 

##### **1.2.3. Trạng thái thông báo (ucNotifyState)**

*  `ucNotifyState` biểu diễn trạng thái điều khiển logic chờ/nhận:

	*  `taskNOT_WAITING_NOTIFICATION`
	
	*  `taskWAITING_NOTIFICATION`
	
	*   `taskNOTIFICATION_RECEIVED`
	
* Kernel sử dụng trường này để:

	*  Quyết định có chuyển Task sang trạng thái Ready hay không
	
	*  Kiểm soát việc ghi đè dữ liệu

#### **1.3.Mô hình kiến trúc**

##### **1.3.1.Mô hình Queue/Semaphore**

*  Task Sender -> IPC Object -> Task Receiver

*  Luồng xử lý:

	* Ghi dữ liệu vào object.
	
	* Cập nhật danh sách chờ (Wait List).
	
	* Sao chép dữ liệu.
	
	* Đánh thức Task nhận.
	
* IPC object có cấu trúc riêng:

	* Danh sách task chờ gửi
	
	* Danh sách task chờ nhận
	
	* Con trỏ buffer
	
	* Biến chỉ số đọc/ghi

##### **1.3.2. Mô hình Task Notification**

*  Task A → TCB của Task B

*  Luồng xử lý:

	* Kernel truy cập trực tiếp TCB.
	
	* Ghi vào `ulNotifiedValue`.
	
	* Cập nhật `ucNotifyState`.
	
	* Nếu cần → chuyển Task B sang Ready List.
	
* Không tồn tại object trung gian.

* Không tồn tại hàng đợi.

* Không có cấu trúc dữ liệu bổ sung.

### **II.  CƠ CHẾ THỰC THI TRONG KERNEL**

#### **2.1. Khi gửi Notification**

		xTaskNotify(targetHandle,
		            1,
		            eIncrement);

* Các bước kernel thực hiện:

	* Vào vùng bảo vệ tới hạn (critical section).
	
	* Truy cập TCB qua `targetHandle`.
	
	* Cập nhật `ulNotifiedValue`.
	
	* Cập nhật `ucNotifyState`.    
	
	* Nếu Task đang Blocked:
	
		*  Xóa khỏi danh sách chờ
		
		*  Thêm vào Ready List
		
	*  Thoát critical section.
	
	*  Có thể kích hoạt chuyển ngữ cảnh (context switch).
	
*  Không có thao tác:

	*  enqueue
	
	*  dequeue
	
	*  sao chép buffer

#### **2.2. Khi Task chờ Notification**

		xTaskNotifyWait(0,
		                0xFFFFFFFF,
		                &receivedValue,
		                portMAX_DELAY);

* Kernel:

	* Kiểm tra `ucNotifyState`.
	
	* Nếu chưa có thông báo:
	
		*  Đặt trạng thái WAITING
		
		*  Chuyển Task sang Blocked List

	*  Khi có thông báo:
	
		*  Sao chép `ulNotifiedValue`
		
		*   Cập nhật lại trạng thái
		
		*   Xóa bit theo cấu hình 

### **III.  HIỆU SUẤT VÀ TÍNH TỐI ƯU**

#### **3.1. Độ trễ (Latency)**

* Notification thường nhanh hơn khoảng 40–50% so với Binary Semaphore vì:

	*  Không quản lý hai danh sách chờ
	
	*  Không thao tác trên buffer
	
	*  Không copy dữ liệu
	
	*  Số bước kernel ít hơn
	
*  Chi phí thời gian của Notification gần tương đương:

	*  Ghi một biến 32-bit + cập nhật trạng thái Task

#### **3.2. Tính xác định thời gian**

* Queue:

	*  Phụ thuộc kích thước buffer
	
	*  Phụ thuộc trạng thái full/empty
	
	*   Có thể lặp lại thao tác kiểm tra

* Notification:

	* Không phụ thuộc độ dài hàng đợi
	
	* Không có vòng lặp xử lý
	
	* Số bước cố định  

#### **3.3. Tối ưu bộ nhớ (RAM Efficiency)**

* Queue cần:

	*  Cấu trúc Queue object (~70–80 byte)
	
	*  Buffer lưu dữ liệu
	
	* 	Danh sách chờ
	
*  Notification:
	
	*  Không cấp phát thêm bộ nhớ
	
	*  Tận dụng trường có sẵn trong TCB
	
*  Do đó:

	*  Không phát sinh lỗi `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`
	
	*  Phù hợp hệ thống RAM hạn chế

### **IV.  GIỚI HẠN KIẾN TRÚC**

#### **4.1. One-to-One Binding**

* Mỗi Notification gắn với một Task cụ thể.

* Không thể broadcast tới nhiều Task.

#### **4.2. No True Buffering**

* Chỉ tồn tại:

	*  Một biến 32-bit

* Nếu Task nhận xử lý chậm:

	*  Có thể bị ghi đè (overwrite mode)
	
	*  Hoặc bỏ lỡ sự kiện
	
* Không thể lưu nhiều payload như Queue.

#### **4.3. Giới hạn kích thước dữ liệu**

* Chỉ truyền được 32-bit.

*  Không phù hợp truyền struct lớn.

#### **4.4. Ràng buộc hướng truyền**

*  ISR không có TCB.

*  Không thể gửi Notification tới ISR.

*  Chỉ hỗ trợ:

	*   Task → Task
	
	*   ISR → Task

### **V. VÍ DỤ**

* **ISR đánh thức Task xử lý dữ liệu**

		static TaskHandle_t processingTaskHandle;

		void vProcessingTask(void *pvParameters)
		{
		    uint32_t value;

		    for (;;)
		    {
		        xTaskNotifyWait(0,
		                        0xFFFFFFFF,
		                        &value,
		                        portMAX_DELAY);

		        processData();
		    }
		}

		void DMA_IRQHandler(void)
		{
		    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		    xTaskNotifyFromISR(processingTaskHandle,
		                       0,
		                       eNoAction,
		                       &xHigherPriorityTaskWoken);

		    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

   </details> 


<details>
    <summary><strong>BÀI 2: MODE 1 – LIGHTWEIGHT SEMAPHORE</strong></summary>

## **BÀI 2: MODE 1 – LIGHTWEIGHT SEMAPHORE**

### **I.  SIGNALING (BÁO HIỆU)**

#### **1.1. Nguyên lý thiết kế**

*	Ở chế độ này, Task Notification được sử dụng như:

	*	Một **cờ sự kiện (flag)**
	
	*  Hoặc một **biến đếm sự kiện (counter)**	

*  Được sử dụng để báo 

	*  Đã hoàn thành
	
	*  Có dữ liệu mới
	
	*  Ngắt đã xảy ra
	
	*  Tài nguyên đã sẵn sàng    

*  Đây chính là hành vi của:

	*  Binary Semaphore
	
	*  Counting Semaphore

#### **1.2. Thiết kế**

##### **1.2.1. Semaphore**

		xSemaphoreGive(binarySem);
		xSemaphoreTake(binarySem, portMAX_DELAY);

##### **1.2.2. Notification**

		xTaskNotifyGive(taskHandle);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

### **II.  API CẤP NHIỆM VỤ (TASK-LEVEL APIs)**

#### **2.1. Gửi tín hiệu: `xTaskNotifyGive()`**

* `xTaskNotifyGive()` thực chất tương đương:

		xTaskNotify(taskHandle, 0, eIncrement);

* Kernel sẽ:

	* Vào critical section.
	
	* Tăng `ulNotifiedValue` lên 1.
	
	* Cập nhật `ucNotifyState`.
	
	* Nếu Task đang Blocked → chuyển sang Ready.

* **VD: Task A báo hoàn thành cho Task B**

		static TaskHandle_t consumerHandle;

		void vProducerTask(void *pvParameters)
		{
		    for (;;)
		    {
		        performAcquisition();

		        xTaskNotifyGive(consumerHandle);
		    }
		}

		void vConsumerTask(void *pvParameters)
		{
		    for (;;)
		    {
		        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		        processData();
		    }
		}

#### **2.2. Chờ tín hiệu: `ulTaskNotifyTake()`**

		uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit,
		                          TickType_t xTicksToWait);

* Tham số `xClearCountOnExit`

	* Trường hợp 1: `pdTRUE` – Binary Mode
	
		*  Sau khi nhận, giá trị bị xóa về 0.
	
		*  Hoạt động giống Binary Semaphore.
		
				ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		* Phù hợp khi:
		
			*   Mỗi lần chỉ cần một tín hiệu.
			
			*   Không cần biết có bao nhiêu lần sự kiện xảy ra. 

	* Trường hợp 2: `pdFALSE` – Counting Mode
	
		*  Sau khi nhận, giá trị giảm đi 1.
	
		*  Nếu trước đó giá trị là 3 → trả về 3.
		
		*  Sau khi thoát → còn 2. 
		
				uint32_t pendingEvents;

				pendingEvents = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

		* Phù hợp khi:
		
			*   Có thể xảy ra nhiều sự kiện liên tiếp.
			
			*   Cần xử lý đủ số lần sự kiện.

* Tham số `xTicksToWait`

	* Cho phép:
	
		*  0 → không chờ (non-blocking)
	
		*  `portMAX_DELAY` → chờ vô hạn
		
		*   Giá trị cụ thể → timeout
		
	* Kernel sẽ:
	
		* Nếu `ulNotifiedValue == 0`
		
			*  Đưa Task vào Blocked List
			
		*  Khi có Notification:
		
			*  Đưa Task về Ready List 

							
### **III.  KỸ THUẬT XỬ LÝ NGẮT (ISR HANDLING)**

#### **3.1. API chuyên dụng: `vTaskNotifyGiveFromISR()`**

* Dùng trong ISR:

		void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify,
		                            BaseType_t *pxHigherPriorityTaskWoken);

	*  Tăng `ulNotifiedValue`.
	
	*  Nếu Task đích có ưu tiên cao hơn:
	
		*  `*pxHigherPriorityTaskWoken = pdTRUE`.
	
	*  Sau đó cần:
	
			portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

#### **3.2. Mô hình Deferred Interrupt Processing**

* Bước 1 – ISR

	*   Xóa cờ ngắt phần cứng.
	
	*   Gửi Notification.
	
	*   Thoát nhanh.
	
			void DMA_IRQHandler(void)
			{
			    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

			    clearHardwareFlag();

			    vTaskNotifyGiveFromISR(processTaskHandle,
			                           &xHigherPriorityTaskWoken);

			    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}



* Bước 2 – Task xử lý
	
			void vProcessingTask(void *pvParameters)
			{
			    for (;;)
			    {
			        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			        heavyComputation();
			    }
			}

	*   Toàn bộ logic nặng được chuyển ra ngoài ISR.
	
   </details> 


<details>
    <summary><strong>BÀI 3: MODE 2 – VALUED NOTIFICATIONS</strong></summary>

## **BÀI 3: MODE 2 – VALUED NOTIFICATIONS**

### **I.  VALUED NOTIFICATION**

#### **1.1. Giới thiệu**

*	Khác với chế độ Signaling (chỉ tăng biến đếm), chế độ này khai thác đầy đủ biến:

			ulNotifiedValue (32-bit)

#### **1.2. Truyền giá trị 32-bit**

* Dùng Notification để truyền:

	*	Mã lệnh điều khiển (Command ID)
	
	*  Mã lỗi (Error Code)	

	*  Giá trị cảm biến (ADC sample)

	*  Trạng thái hệ thống (System State)
	
	*  Con trỏ tới buffer (trong thiết kế nâng cao)

			xTaskNotify(controlTaskHandle,
			            CMD_START,
			            eSetValueWithOverwrite);

#### **1.3. Bitmasking – Thay thế Event Group**

* Biến 32-bit có thể chia thành 32 cờ sự kiện:

			Bit 0 → ADC Done
			Bit 1 → UART RX
			Bit 2 → Timeout
			Bit 3 → Sensor Error
			...

* Sử dụng phép OR để gộp nhiều sự kiện.

* VD:

		#define EVT_ADC_DONE     (1 << 0)
		#define EVT_UART_RX      (1 << 1)
		#define EVT_TIMEOUT      (1 << 2)

* Gửi:

		xTaskNotify(taskHandle,
		            EVT_ADC_DONE,
		            eSetBits);

* Nhận:

		xTaskNotifyWait(0,
		                0xFFFFFFFF,
		                &eventFlags,
		                portMAX_DELAY);
		            
### **II.  HỆ THỐNG API TRUYỀN DỮ LIỆU**

#### **2.1. Gửi dữ liệu: `xTaskNotify()`**

		BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify,
		                       uint32_t ulValue,
		                       eNotifyAction eAction);

* Tham số:

	* `xTaskToNotify` → Task nhận
	
	* `ulValue` → Giá trị truyền
	
	* `eAction` → Cách xử lý giá trị

* Luồng thực thi kernel

	* Vào critical section.
	
	* Truy cập TCB.
	
	* Thực hiện hành động theo `eAction`.

	* Cập nhật `ucNotifyState`.
	
	* Nếu Task đang Blocked → đưa vào Ready List.
	
	* Thoát critical section.
	


#### **2.2. Nhận dữ liệu: `xTaskNotifyWait()`**

		BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
		                           uint32_t ulBitsToClearOnExit,
		                           uint32_t *pulNotificationValue,
		                           TickType_t xTicksToWait);

* Tham số `ulBitsToClearOnEntry`

	* Xóa các bit trước khi chờ.
	
		*  Làm sạch trạng thái cũ.
	
		*  Bảo đảm không xử lý sự kiện tồn dư.
		
* Tham số `ulBitsToClearOnExit`

	* Xóa các bit sau khi nhận.
	
	*  Thường dùng: 0xFFFFFFFF
	
	*  Để xóa toàn bộ sau khi đọc.

* Tham số `pulNotificationValue`

	* Con trỏ nhận giá trị 32-bit.
	
	*  Kernel sẽ:
	
		*  Sao chép `ulNotifiedValue` vào biến này.
		*  Thực hiện xóa bit theo cấu hình.

							
### **III.  PHÂN TÍCH CÁC HÀNH ĐỘNG THÔNG BÁO (eNotifyAction)**

#### **3.1.`eSetBits` – Gộp sự kiện**

			ulNotifiedValue |= ulValue;

*  Đặc điểm:
	
	*  Không mất sự kiện nếu nhiều nguồn gửi khác nhau.
	
	* Có thể tích lũy nhiều bit trước khi Task xử lý.
	
*  VD:
	
			xTaskNotify(sensorTaskHandle,
            EVT_ADC_DONE | EVT_UART_RX,
            eSetBits);

#### **3.2. `eIncrement` – Tăng giá trị**

			ulNotifiedValue++;

* Giống:

	*   xTaskNotifyGive()
	
*   Khác biệt:
	
	*   Có thể dùng chung với `xTaskNotify()` API.
	
#### **3.3. `eSetValueWithOverwrite` – Ghi đè**

			ulNotifiedValue = ulValue;

* Không kiểm tra giá trị cũ.
	
*   Ví dụ:
	
	*   Trạng thái hệ thống
	
	*   Giá trị nhiệt độ tức thời
	
	*   Tốc độ motor hiện tại

					    xTaskNotify(displayTaskHandle,
		            currentTemperature,
		            eSetValueWithOverwrite);

#### **3.4. ``eSetValueWithoutOverwrite` – Không ghi đè**

* Chỉ ghi nếu `ucNotifyState` chưa ở trạng thái “đã nhận”.
	
*   Nếu Task chưa đọc giá trị cũ:
	
	*   Hàm trả về `pdFAIL`.
	
* Ứng dụng:

	*  Mailbox yêu cầu không mất dữ liệu.
	
	*  Cơ chế giao tiếp cần xác nhận đọc.  

			if (xTaskNotify(taskHandle,
			                newValue,
			                eSetValueWithoutOverwrite) == pdFAIL)
			{
			    // xử lý khi mailbox đang bận
			}
	
  
   </details> 

# CHƯƠNG 6: **SYNC & RESOURCE PROTECTION**
<details>
    <summary><strong>BÀI 1: CRITICAL SECTIONS</strong></summary>

## **BÀI 1: CRITICAL SECTIONS**

### **I.  RACE CONDITION**

#### **1.1. Khái niệm**

*	**Race Condition** (tình trạng tranh chấp tài nguyên) là một hiện tượng phổ biến trong các hệ thống xử lý song song.

*  Nó phát sinh khi nhiều **tiến trình (process)** hoặc **luồng (thread)** cùng truy cập vào một **tài nguyên dùng chung (shared resource)**.

*  Các truy cập này không được kiểm soát bởi **cơ chế đồng bộ**

	*  **Shared resource** có thể là biến, vùng nhớ, tệp tin hoặc thiết bị phần cứng

	*  **Process** là đơn vị thực thi chương trình có không gian địa chỉ riêng 

	*  **Thread** là đơn  vi thực thi nhỏ hơn trong một tiến trình, dùng chung không gian địa chỉ với các luồng khác
	
*  Khi Race Condition xảy ra, kết quả của chương trình phụ thuộc vào thứ tự thực thi của các luồng

	* Thứ tự này là **không xác định** và do **scheduler** của hệ điều hành quyết định

#### **1.2. Cơ chế hình thành**

* **Race condition** xuất hiện khi thỏa mãn:

	*	Có tài nguyên dùng chung giữa các thực thể thực thi
	
		*	VD: **Biến đếm truy cập website**
		
			*  Một trang web có biến counter dùng để đếm số lượng truy cập 
			
			*  Biến này được lưu trong bộ nhớ và nhiều phiên xử lý request khác nhau sẽ cùng đọc và ghi vào biến này  	
	
	*	Có nhiều thực thể thực thi đồng thời
	
		*	VD: **Ứng dụng đa luồng xử lý ảnh****
		
			*  Một ứng dụng chỉnh sửa ảnh tạo ra 4 luồng riêng biệt để xử lý 4 góc của một bức ảnh lớn.
			
			*  Các luồng này cùng chạy song song để tăng tốc xử lý. 	

	*	Không tồn tại cơ chế loại trừ tương hỗ
	
		*	VD: **Tăng biến đếm**
		
			*  Xét một thao tác tăng biến đếm toàn cục: `global_counter++`
			
			*  Thao tác này được chia làm 3 bước riêng biệt:
			
				*  Read: Giá trị hiện tại của `global_counter` được đọc từ bộ nhớ vào thanh ghi
				
				*  Modify: Giá trị trong thanh ghi được tăng lên 1 đơn vị
				
				*  Write: Giá trị mới được ghi từ thanh ghi trở lại bộ nhớ
				
### **II.  ATOMICITY**

#### **2.1. Định nghĩa**

*	Tính nguyên tử là tính chất của một thao tác được bảo đảm thực hiện trọn vẹn hoặc không thực hiện gì cả

*   Thao tác đó không bị gián đoạn hay xen kẽ bởi tiến trình khác

#### **2.2.Đặc điểm**

* Không bị gián đoạn trong quá trình thực thi

* Không xảy ra xen kẽ với thao tác tương tự từ luồng khác

* Thao tác hoàn tất toàn bộ, hoặc trạng thái hệ thống không thay đổi, không tồn tại trạng thái trung gian quan sát được

#### **2.3.Ví dụ**

* Đọc/ghi 1 byte đơn lẻ
	
	*  CPU thực hiện lệnh `MOV` (trên kiến trúc x86) hoặc `LDR`/`STR` (trên ARM) để đọc hoặc ghi một đơn vị dữ liệu từ bộ nhớ.
	
	*   Bus dữ liệu giữa CPU và bộ nhớ đảm bảo rằng một lần truyền dữ liệu cơ bản không thể bị xen ngang bởi CPU khác.

* Test-and-Set (TAS)
	
	*  Đọc giá trị hiện tại của một ô nhớ
	
	*   Sau đó đặt ô nhớ đó thành 1 (hoặc giá trị khác không)	

* Compare-and-Swap (CAS)
	
	*  So sánh giá trị hiện tại của ô nhớ với một giá trị kỳ vọng.
	
	*   Nếu bằng nhau, thay thế nó bằng giá trị mới

			int CompareAndSwap(int *ptr, int expected, int new_value) {
			    int current = *ptr;               // Đọc giá trị hiện tại
			    if (current == expected) {         // So sánh với giá trị kỳ vọng
			        *ptr = new_value;              // Nếu bằng, cập nhật giá trị mới
			    }
			    return current;                    // Trả về giá trị cũ
			}

* Fetch-and-Add
	
	*   Đọc giá trị hiện tại của ô nhớ
	
	*   Sau đó cộng thêm một giá trị xác định vào ô nhớ đó

			int FetchAndAdd(int *ptr, int increment) {
			    int old_value = *ptr;      // Đọc giá trị hiện tại
			    *ptr = old_value + increment; // Cộng thêm giá trị
			    return old_value;          // Trả về giá trị cũ
			}    
  
### **III.  KHÓA NGẮT (taskENTER_CRITICAL)**

#### **3.1. Cơ chế**

* `taskENTER_CRITICAL()` thực hiện:

	* Gọi `portDISABLE_INTERRUPTS()` 
	
	* Tăng biến nesting count để hỗ trợ critical section lồng nhau 

* `taskEXIT_CRITICAL()` thực hiện:
	
	* Giảm biến nesting count
	
	* Chỉ khi nesting count về 0 mới khôi phực ngắt bằng `portENABLE_INTERRUPTS()` hoặc khôi phục mask trước đó

* Trong khoảng giữa `ENTER` và `EXIT`:

	* Không có ISR nào được thực thi.
	
	* Không xảy ra context switch.
	
* **Cú pháp:**

		taskENTER_CRITICAL();
		/* ----- Critical section ----- */
		taskEXIT_CRITICAL();

#### **3.2. Đặc điểm**

* Thực thi rất nhanh (chỉ vài chu kỳ CPU).

*  Không cần đối tượng đồng bộ như mutex hoặc semaphore.

*  Làm tăng độ trễ ngắt (Interrupt Latency).

#### **3.3. taskENTER_CRITICAL_FROM_ISR()**

* Trong ISR không được dùng `taskENTER_CRITICAL()`.

	*   ISR không thuộc ngữ cảnh task.
	
* **Cú pháp:**
 
		 UBaseType_t uxSavedInterruptStatus;

		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
		/* ----- Critical section trong ISR ----- */
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	
#### **3.4. Lưu ý**

* Chỉ sử dụng khi:

	*   Cập nhật biến đếm.
	
	*   Gán cờ trạng thái.
	
	*    Đọc/ghi thanh ghi phần cứng.
	
	*     Sao chép vài byte dữ liệu.

#### **3.5. VD**

* Trong Task:

		void vSharedCounterTask(void *pvParameters){
		for(;;){
			taskENTER_CRITICAL();
			global_counter++;
			local_copy = global_counter;
			taskEXIT_CRITICAL();
			}
		}

* Trong ISR

		void TIM2_IRQHandler(void){
			
			UBaseType_t uxStatus = taskENTER_CRITICAL_FROM_ISR()
			shared_flag = 1;
			taskEXIT_CRITICAL_FROM_ISR(uxStatus);
			}

### **IV.  KHÓA SCHEDULER – vTaskSuspendAll()**

#### **4.1. Cơ chế**

##### **4.1.1. Hàm vTaskSuspendAll()**

* Khi một task gọi `vTaskSuspendAll()`:

	* Hệ thống tăng biến nesting count lên 1 đơn vị
	
		*  Biến đếm này cho biết số lần scheduler bị đình chỉ lồng nhau  
	
	* Scheduler bị tạm dừng (scheduler suspended)
	
	* Không có context switch nào được phép xảy ra
	
		* Task hiện tại sẽ tiếp tục chiếm CPU cho đến khi gọi `xTaskResumeAll()`
		
		* Không một task nào khác, kể cả task có higher priority được phép chạy
		
	* Interrupt vẫn được enable, do đó các ISR vẫn có thể xử lý ngắt và thực hiện công việc của chúng    

##### **4.1.2. Hàm vTaskResumeAll()**

* Khi một task gọi `vTaskResumeAll()`:

* Khi biến đếm trở về 0, scheduler được khôi phục hoạt động 

* Hệ thống sẽ kiểm tra và xử lý các **chuyển đổi ngữ cảnh đang chờ (pending context switch)** có thể đã phát sinh trong thời gian scheduler bị đình chỉ 
	
*  Hàm trả về: 
	
	*  **pdTRUE**: Nếu có một chuyển đổi ngữ cảnh xảy ra ngay sau khi resume.
	
	*   **pdFALSE**: Nếu không có chuyển đổi ngữ cảnh nào xảy ra.

#### **4.2. Cú pháp**

		vTaskSuspendAll();
		/* ----- Critical section ----- */
		xTaskResumeAll();

#### **4.3. Ứng dụng**

* **Cần ISR vẫn hoạt động** để thu thập dữ liệu thời gian thực:

	-   Driver UART nhận dữ liệu vào buffer (ISR điền byte).
	
	-   Task xử lý buffer lớn mà không muốn bị gián đoạn bởi task khác.
	
	-   Driver SPI/I2C với polling dài, nhưng vẫn cần nhận ngắt từ các nguồn khác.
	
	-   Xử lý dữ liệu sensor batch mà ISR tiếp tục cập nhật raw data.

#### **4.4. VD**

		// Task xử lý buffer UART nhận được từ ISR
		void vUartProcessTask(void *pvParameters) {
		    for(;;) {
		        // Chờ có dữ liệu (hoặc polling)
		        if (uxBytesInUartBuffer > MIN_PROCESS) {

		            vTaskSuspendAll();  // Ngăn task khác đọc/ghi buffer

		            // ----- Critical section -----
		            memcpy(localBuf, uartRxBuffer, PROCESS_SIZE);
		            uartRxHead -= PROCESS_SIZE;  // Di chuyển con trỏ
		            // Xử lý localBuf (parse, validate, gửi lên app layer...)
		            // ----- End critical section -----

		            xTaskResumeAll();   // Cho phép scheduler chạy lại
		        }

		        vTaskDelay(pdMS_TO_TICKS(10));
		    }
		}

		// ISR UART vẫn chạy bình thường, điền byte vào uartRxBuffer
		void USART_IRQHandler(void) {
		    if (USART_GetITStatus(USARTx, USART_IT_RXNE)) {
		        uartRxBuffer[uartRxHead++] = USART_ReceiveData(USARTx);
		        // Không bị block bởi task suspend scheduler
		    }
		}

   </details> 

   
<details>
    <summary><strong>BÀI 2: MUTEX (MUTUAL EXCLUSION)</strong></summary>

## **BÀI 2: MUTEX (MUTUAL EXCLUSION)**

### **I.  MUTEX**

#### **1.1. Khái niệm**

*	**Mutex** là cơ chế đồng bộ dùng để bảo vệ tài nguyên chia sẻ giữa các task

*  Tại một thời điểm chỉ có một task duy nhất được truy cập tài nguyên

*  Các task khác phải chờ cho đến khi mutex được giải phóng

#### **1.2. Cơ chế sở hữu (Ownership)**

* Mutex có khái niệm chủ sở hữu (owner)

	*	Task gọi `xSemaphoreTake()` thành công sẽ trở thành owner
	
	*  Chỉ owner mới được phép gọi `xSemaphoreGive()`
	
#### **1.3. Cú pháp**

		xSemaphoreTake(xMutex, portMAX_DELAY);   // Khóa (chờ nếu cần)
		 /* ----- Critical section: truy cập tài nguyên ----- */
		xSemaphoreGive(xMutex);                   // Mở khóa (phải bởi owner)


						
### **II.  PHÂN LOẠI**

#### **2.1. Standard Mutex**

##### **2.1.1. Khởi tạo Mutex**

*	Để tạo một Standard Mutex, sử dụng hàm:

		SemaphoreHandle_t xSemaphoreCreateMutex( void );

##### **2.1.2.Đặc điểm**
		
*   Một task chỉ được **take (lấy)** mutex một lần

	*  Mutex có tính **sở hữu (ownership)**.
	
	*  Task đã lấy mutex được gọi là **task sở hữu (owner task)**.
	
	*  Chỉ task sở hữu mới được phép **give (trả lại)** mutex. 

*   Không cho phép **nested locking** (khóa lồng nhau)

*  Không được dùng trong ISR

##### **2.1.3.Thao tác**

* **Lấy Mutex (Take):**

		BaseType_t xSemaphoreTake( SemaphoreHandle_t xMutex, TickType_t xTicksToWait );
		
	* **Tham số:**

		*   `pdTRUE`: Lấy mutex thành công.

		*   `pdFALSE`: Không lấy được mutex (hết thời gian chờ).
	 
	* **Giá trị trả về:**

		*   `pdTRUE`: Lấy mutex thành công

		*   `pdFALSE`: Không lấy được mutex (hết thời gian chờ).

* **Trả lại Mutex (Give):**

		BaseType_t xSemaphoreGive( SemaphoreHandle_t xMutex );
		
	 
	* **Giá trị trả về:**

		*   `pdTRUE`: Trả mutex thành công.

		*   `pdFALSE`: Trả mutex thất bại (thường do task gọi không phải là task sở hữu).

##### **2.1.4.VD**

		// Khai báo handle mutex
		SemaphoreHandle_t xI2cMutex;

		// Hàm khởi tạo ứng dụng
		void app_init(void) {
		    // Tạo mutex cho bus I2C
		    xI2cMutex = xSemaphoreCreateMutex();
		    
		    if (xI2cMutex == NULL) {
		        // Xử lý lỗi nếu không tạo được mutex
		        // (thường do thiếu heap memory)
		    }
		    
		    // Khởi tạo các task sử dụng I2C
		    xTaskCreate(vSensorTask, "Sensor", 256, NULL, 1, NULL);
		    xTaskCreate(vDisplayTask, "Display", 256, NULL, 1, NULL);
		}

		// Task đọc cảm biến qua I2C
		void vSensorTask(void *pvParameters) {
		    uint8_t sensorData[16];
		    
		    for(;;) {
		        // Cố gắng lấy mutex I2C, chờ tối đa 100ms
		        if (xSemaphoreTake(xI2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
		            
		            // ===== Đã có quyền sử dụng I2C =====
		            // Thực hiện giao tiếp I2C với cảm biến
		            i2c_start(0x50);              // Gửi điều kiện START và địa chỉ slave
		            i2c_write(0x00);               // Ghi địa chỉ thanh ghi
		            i2c_rep_start(0x51);           // Gửi lại START để đọc
		            for(int i = 0; i < 16; i++) {
		                sensorData[i] = i2c_read(i == 15 ? I2C_NACK : I2C_ACK);
		            }
		            i2c_stop();                     // Kết thúc truyền thông
		            
		            // Trả lại mutex cho task khác sử dụng
		            xSemaphoreGive(xI2cMutex);
		            
		            // Xử lý dữ liệu cảm biến
		            processSensorData(sensorData, 16);
		            
		        } else {
		            // Không lấy được mutex trong 100ms
		            // Có thể ghi log hoặc xử lý lỗi
		            vLogError("I2C busy - sensor read timeout");
		        }
		        
		        // Đợi một khoảng thời gian trước khi đọc lại
		        vTaskDelay(pdMS_TO_TICKS(1000));
		    }
		}

		// Task hiển thị thông tin lên LCD (cũng dùng I2C)
		void vDisplayTask(void *pvParameters) {
		    char displayBuffer[32];
		    
		    for(;;) {
		        // Lấy mutex I2C để cập nhật LCD
		        if (xSemaphoreTake(xI2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
		            
		            // Cập nhật thông tin lên LCD qua I2C
		            lcd_clear();
		            lcd_set_cursor(0, 0);
		            lcd_print(displayBuffer);
		            
		            // Trả lại mutex
		            xSemaphoreGive(xI2cMutex);
		        }
		        
		        vTaskDelay(pdMS_TO_TICKS(100));
		    }
		}		

#### **2.2. Recursive Mutex**

##### **2.2.1.Khái niệm**

* **Recursive Mutex** (mutex đệ quy) là một dạng đặc biệt của mutex cho phép **cùng một task** có thể **take (lấy)** mutex nhiều lần liên tiếp mà không bị block.

##### **2.2.2. Khởi tạo Recursive Mutex**

*	Để tạo một Recursive Mutex, sử dụng hàm:

		SemaphoreHandle_t xSemaphoreCreateRecursiveMutex( void );

##### **2.2.3.Đặc điểm**
		
*   Cùng một task có thể take mutex nhiều lần

*  FreeRTOS theo dõi số lần take theo từng **owner (task sở hữu)**
	
	*  Hệ thống duy trì một **biến đếm (counter)** cho mỗi recursive mutex.
	
	*  Mỗi lần task sở hữu take thành công, biến đếm tăng lên 1.

*  Chỉ khi số lần give bằng số lần take, mutex mới thực sự được **unlock (mở khóa)**

*  Không được dùng trong ISR

##### **2.2.4.Thao tác**

* **Lấy Recursive Mutex (Take Recursive):**

		BaseType_t xSemaphoreTakeRecursive( SemaphoreHandle_t xMutex, TickType_t xTicksToWait );
		
	* **Tham số:**

		*   `xMutex`: Handle của recursive mutex cần lấy.

		*   `xTicksToWait`: Thời gian chờ tối đa (tính bằng ticks).
	 
	* **Giá trị trả về:**

		*   `pdTRUE`: Lấy mutex thành công

		*   `pdFALSE`: Không lấy được mutex (hết thời gian chờ).

	* **Cơ chế hoạt động:**

		*  Nếu task gọi **chưa sở hữu mutex**: hoạt động như standard mutex (phải chờ nếu mutex đang bị chiếm).

		*  Nếu task gọi **đã sở hữu mutex**: thành công ngay lập tức, tăng **lock count** lên 1.
		
* **Trả lại Recursive Mutex (Give Recursive):**

		BaseType_t xSemaphoreGiveRecursive( SemaphoreHandle_t xMutex );
		
	 
	* **Giá trị trả về:**

		*   `pdTRUE`: Trả mutex thành công.

		*   `pdFALSE`: Trả mutex thất bại (thường do task gọi không phải là task sở hữu).

	* **Cơ chế hoạt động:**

		*  Giảm **lock count** xuống 1.

		* Khi lock count về **0**, mutex thực sự được giải phóng và task khác có thể lấy được.
		
##### **2.2.5.VD**

		// Tạo recursive mutex
		SemaphoreHandle_t xRecursiveMutex = xSemaphoreCreateRecursiveMutex();

		// Task sử dụng recursive mutex
		void vTaskFunction(void *pvParameters) {
		    // Lần take thứ nhất
		    xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		    
		    // Đang ở trong vùng bảo vệ cấp 1
		    
		    // Lần take thứ hai (lồng nhau)
		    xSemaphoreTakeRecursive(xRecursiveMutex, portMAX_DELAY);
		    
		    // Đang ở trong vùng bảo vệ cấp 2
		    // Thực hiện công việc...
		    
		    // Trả lại lần thứ hai
		    xSemaphoreGiveRecursive(xRecursiveMutex);
		    
		    // Vẫn còn ở vùng bảo vệ cấp 1
		    
		    // Trả lại lần thứ nhất
		    xSemaphoreGiveRecursive(xRecursiveMutex);
		    
		    // Đã thoát hoàn toàn khỏi vùng bảo vệ
		}

   </details> 
   
