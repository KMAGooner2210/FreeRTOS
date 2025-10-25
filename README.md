# FreeRTOS
# **CHƯƠNG 2: FreeRTOS Kernel Distribution**

<details>
	<summary><strong>BÀI 1: INTRODUCTION</strong></summary>

## **BÀI 1: INTRODUCTION**

### **I. CẤU TRÚC PHÂN PHỐI FreeRTOS**

#### **1.1. Khái niệm**

 *  **FreeRTOS** là một hệ điều hành thời gian thực (RTOS) mã nguồn mở, được thiết kế để cung cấp một môi trường linh hoạt và hiệu quả cho việc phát triển các ứng dụng nhúng trên nhiều nền tảng phần cứng khác nhau

 *  **Khi tải FreeRTOS từ trang web chính thức**, người dùng nhận được một gói phân phối bao gồm ba phần chính: kernel, port và demo


#### **1.2. Kernel (Nhân hệ thống)**

 *  Kernel là phần lõi của FreeRTOS, chứa các tệp mã nguồn cơ bản chịu trách nhiệm quản lý các tác vụ (tasks), hàng đợi (queues), semaphore, và các cơ chế đồng bộ hóa khác

 *  Các tệp chính trong kernel bao gồm
   
    ◦ **task.c:** Chứa các hàm quản lý việc tạo, xóa, tạm dừng và ưu tiên các tác vụ (task management)

     => Đây là thành phần cốt lõi quyết định cách scheduler (lập lịch) hoạt động 
    
    ◦ **queue.c:** Triển khai các cơ chế hàng đợi và semaphore

     => Cho phép giao tiếp an toàn giữa các tác vụ hoặc giữa tác vụ và ngắt (interrupt)

    ◦ **list.c:** Cung cấp cấu trúc dữ liệu danh sách liên kết (linked list)

     => Tổ chức các tác vụ và tài nguyên trong kernel

    ◦ **heap_x.c:** Các tệp quản lý bộ nhớ động (dynamic memory allocation)

     => Cho phép phân bổ và giải phóng bộ nhớ cho các tác vụ hoặc hàng đợi

     => Số "x" đại diện cho các biến thể khác nhau của cơ chế heap, mỗi biến thể có ưu điểm và hạn chế riêng

 *  Kernel là phần độc lập với phần cứng, được thiết kế để có thể tái sử dụng trên nhiều kiến trúc khác nhau thông qua các port

#### **1.3. Port (Cổng thích nghi)**

 *  **Port** là phần mã nguồn phụ thuộc vào kiến trúc phần cứng và trình biên dịch cụ thểm được sử dụng để thích nghi kernel FreeRTOS với một nền tảng phần cứng nhất định

 *  Mỗi port chứa các tệp mã nguồn và tiêu đề (header files) được tùy chỉnh để hỗ trợ các đặc điểm của bộ vi xử lý hoặc vi điều khiển
   
    ◦ Ví dụ: Đối với kiến trúc Cortex-M3 ,port bao gồm các tệp như:

        port.c: Chứa các hàm xử lý ngắt (interrupt handling), chuyển đổi ngữ cảnh (context switching), và đồng bộ hóa với bộ đếm thời gian hệ thống (Systick timer)

        portmacro.h: Định nghĩa các macro và cấu hình cụ thể cho trình biên dịch và kiến trúc Cortex-M3
    
 *  **Port** đảm bảo rằng kernel có thể tương tác đúng với phần cứng chẳng hạn như quản lý ngắt (interrupts), bộ nhớ stack và bộ đếm thời gian (timer)

#### **1.4. Demo**

 *  Gói FreeRTOS bao gồm một bộ sưu tập các dự án demo, được thiết kế để minh họa cách sử dụng kernel và port trên các nền tảng phần cứng khác nhau

 *  Các demo này thường đi kèm với mã nguồn hoàn chỉnh và có thể được biên dịch trực tiếp trong các môi trường phát triển tích hợp (IDE) như Keil, IAR, hoặc GCC.
   

    
### **II. VAI TRÒ VÀ TƯƠNG TÁC GIỮA CÁC THÀNH PHẦN**

 *  Sự phân chia giữa kernel, port, và demo cho phép FreeRTOS linh hoạt và dễ mở rộng

 *  Kernel cung cấp chức năng cốt lõi, port đảm bảo tương thích phần cứng, và demo giúp người dùng làm quen với hệ thống một cách thực hành.
   
 *  kernel định nghĩa logic vận hành của RTOS (quản lý tác vụ, tài nguyên), trong khi port là cầu nối giữa kernel và phần cứng (xử lý ngắt, timer)

 </details> 
