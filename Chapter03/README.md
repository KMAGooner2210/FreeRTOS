* **Bài 1: Heap_4 Allocation**

    •	Mô tả: 

        o	Tạo một task trong FreeRTOS để thực hành cấp phát và giải phóng bộ nhớ động bằng heap_4

        o	Task sẽ liên tục cấp phát một vùng nhớ, ghi dữ liệu có chứa tick hiện tại, gửi qua UART

        o	Sau đó giải phóng bộ nhớ

    •	Yêu cầu: 


        o	Chọn file heap_4.c trong cấu hình FreeRTOS.

        o	Tạo một task bằng xTaskCreate().

        o	Trong vòng lặp for(;;):

            Dùng pvPortMalloc() cấp phát 50 bytes cho chuỗi ký tự.

            Dùng sprintf() ghi chuỗi có chứa xTaskGetTickCount().

            Gửi chuỗi qua UART bằng printf().

            Giải phóng bộ nhớ bằng vPortFree().

            Delay 1000ms bằng vTaskDelay() rồi lặp lại.
    


* **Bài 2: Heap Monitoring**

    •	Mô tả:

        o	Tạo hàm theo dõi bộ nhớ heap.

        o	In ra dung lượng còn trống hiện tại.

        o	In ra mức thấp nhất từng đạt được.

        o	Quan sát sự thay đổi khi cấp phát và giải phóng.

    •	Yêu cầu:

        o	Bật configUSE_TRACE_FACILITY (nếu cần).

        o	Viết hàm in:

            xPortGetFreeHeapSize()

            xPortGetMinimumEverFreeHeapSize()

        o	Gọi hàm tại:

            Trước pvPortMalloc()

            Sau pvPortMalloc()

            Sau vPortFree()

        o	Quan sát kết quả qua UART.

* **Bài 3: Heap_1 Limitations**

    •	Mô tả: 

        o	Thay heap_4 bằng heap_1 trong project.

        o	Chạy lại chương trình cấp phát và giải phóng bộ nhớ.

        o	Quan sát sự thay đổi của heap theo thời gian.

        o	Nhận thấy bộ nhớ không được trả lại sau khi vPortFree().

    •	Yêu cầu:

        o	Xóa heap_4.c, thêm heap_1.c vào project.

        o	Chạy lại code ở Bài 1.

        o	Quan sát:    

            Bộ nhớ giảm dần về 0.

            vPortFree() không có tác dụng.

* **Bài 4: Memory Fragmentation**

    •	Mô tả:

        o	Thực nghiệm hiện tượng phân mảnh bộ nhớ với heap_2.

        o	Cấp phát nhiều khối nhớ liên tiếp.

        o	Giải phóng xen kẽ để tạo khoảng trống rời rạc.

        o	Quan sát việc cấp phát lớn bị thất bại dù tổng bộ nhớ vẫn đủ.

    •	Yêu cầu: 

        o	Sử dụng heap_2.c.

        o	Cấp phát 3 khối:

            A: 100 byte

            B: 100 byte

            C: 100 byte

        o	Giải phóng khối B.

        o	Thử cấp phát khối D: 150 byte

        o	Đổi sang heap_4.c.

        o	Giải phóng A và B.       

        o	Thử cấp phát lại khối D

* **Bài 5: Malloc Failed Hook**

    •	Mô tả:

        o	Cấu hình cơ chế bắt lỗi khi hệ thống hết bộ nhớ.

        o	Tạo hàm callback xử lý khi pvPortMalloc() thất bại.

        o	Hiển thị lỗi bằng LED và dừng hệ thống.

        o	Thực nghiệm bằng cách cấp phát liên tục để gây tràn bộ nhớ.

    •	Yêu cầu: 

        o	Mở FreeRTOSConfig.h:

            Set #define configUSE_MALLOC_FAILED_HOOK 1

        o	Viết hàm:

            void vApplicationMallocFailedHook(void)

            Tắt ngắt

            Bật LED báo lỗi (GPIO_WriteBit)

        o	Trong Task:

            Cấp phát liên tục 1000 byte

            Không giải phóng

        o	Quan sát:

            LED sáng khi hết bộ nhớ

* **Bài 6: Task Deletion & Memory Cleanup**

    •	Mô tả:

        o	Tạo Task A sinh ra Task B.

        o	Task B chạy một thời gian rồi tự xóa.

        o	Theo dõi bộ nhớ heap trước và sau khi Task B kết thúc.

        o	Quan sát việc bộ nhớ không được giải phóng ngay lập tức.

    •	Yêu cầu: 

        o	Tạo Task A.

        o	Trong Task A:

            Dùng xTaskCreate() tạo Task B.

        o	Trong Task B:

            Chạy vài giây

            Gọi vTaskDelete(NULL) để tự xóa

        o	Trong Task A:

            Theo dõi xPortGetFreeHeapSize()

        o	Quan sát:

            RAM không tăng lại ngay sau khi Task B bị xóa


* **Bài 7: Heap_5 Memory Regions**

    •	Mô tả:

        o	Sử dụng heap_5 để quản lý nhiều vùng nhớ riêng biệt.

        o	Giả lập hệ thống có nhiều khối RAM độc lập.

        o	Cấu hình các vùng nhớ và kiểm tra cấp phát thực tế.

    •	Yêu cầu: 

        o	Khai báo 2 mảng tĩnh:

            uint8_t RAM_Region1[4096];

            uint8_t RAM_Region2[4096];

        o	Trong main() (trước khi start scheduler):

            Khai báo mảng HeapRegion_t

            Trỏ tới 2 vùng nhớ (địa chỉ tăng dần)

            Kết thúc bằng {NULL, 0}

        o	Gọi vPortDefineHeapRegions()

        o	Thực hiện:

            pvPortMalloc()

            In địa chỉ con trỏ

        o	Quan sát:

            Bộ nhớ được cấp phát nằm trong 2 vùng đã định nghĩa
