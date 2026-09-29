# STM32F103 - Bare Metal Week 3

Thực hành lập trình STM32F103C8T6 trên Ubuntu Linux, sử dụng ngôn ngữ C, thư viện STM32 Standard Peripheral Library, ARM GCC và ST-Link V2.

## 1. Danh sách bài tập

### Bài 01: Giao tiếp I2C

*Yêu cầu:*
- Cấu hình I2C giao tiếp thành công với một ngoại vi.
- Có thể lựa chọn DS1307, SSD1306, BMP280 hoặc MPU6050.
- Thực hiện chức năng tương ứng với ngoại vi được chọn.

*Sơ đồ nối chân đề xuất nếu sử dụng SSD1306:*

| STM32F103 | SSD1306 |
|---|---|
| PB6 - I2C1_SCL | SCL |
| PB7 - I2C1_SDA | SDA |
| 3V3 | VCC (module hỗ trợ 3,3 V) |
| GND | GND |

Trạng thái: Chưa bổ sung chương trình và video demo.

### Bài 02: SPI điều khiển MAX7219

*Yêu cầu:*
- Cấu hình SPI hoạt động ở chế độ Master.
- Giao tiếp với MAX7219 LED Matrix 8×8.
- Điều khiển ma trận LED hiển thị giá trị.

*Chương trình thực hiện:*
- Kiểm tra toàn bộ 64 LED khi khởi động.
- Hiển thị các số từ 0 đến 9.
- Thay đổi số sau mỗi giây.
- Sử dụng SysTick tạo mốc thời gian.

*Sơ đồ nối chân:*

| STM32F103 | MAX7219 |
|---|---|
| PA4 | CS / LOAD |
| PA5 - SPI1_SCK | CLK |
| PA7 - SPI1_MOSI | DIN |
| GND | GND |
| Nguồn 5 V ổn định | VCC |

Lưu ý: Khuyến nghị sử dụng 74AHCT125 để chuyển mức tín hiệu SPI từ 3,3 V lên 5 V và nối chung GND.


### Bài 03: UART + Nút nhấn + DMA

*Yêu cầu:*
- Cấu hình UART, nút nhấn và DMA.
- Mỗi lần nhấn nút hợp lệ, biến đếm tăng một đơn vị.
- Truyền dữ liệu từ STM32 lên PC bằng DMA + UART.
- Không sử dụng hàm chờ UART.

*Định dạng bản tin:*

text
<ID-Lớp><ID-Nhóm>:BTN:<Giá trị>\n\r

*Chương trình:*
- Nút nhấn PB0 sử dụng Input Pull-up.
- Chống dội nút nhấn 30 ms.
- USART1 có tốc độ 115200 baud.
- DMA1 Channel 4 truyền dữ liệu UART.

*Sơ đồ nối chân:*

| STM32F103 | Thiết bị |
|---|---|
| PB0 | Nút nhấn |
| GND | Chân còn lại của nút |
| PA9 - USART1_TX | USB-TTL RXD |
| PA10 - USART1_RX | USB-TTL TXD (nếu cần) |
| GND | USB-TTL GND |

USB-TTL cần tương thích mức logic 3,3 V.

*Mở Minicom:*

sudo minicom -D /dev/ttyUSB0 -b 115200

Thay /dev/ttyUSB0 bằng đúng cổng USB-TTL thực tế.


### Bài 04: ADC + Timer + UART + DMA

*Yêu cầu:*
- Cấu hình ADC, Timer, UART và DMA.
- Timer hoạt động ở tần số 100 Hz.
- ADC được kích hoạt bởi Timer.
- DMA tự động lưu dữ liệu ADC trong một giây vào RAM.
- Sử dụng ngắt Half-transfer và Transfer-complete.
- Truyền từng vùng dữ liệu an toàn lên PC bằng UART.
- Các giá trị gửi lên dưới dạng số, phân cách bằng \n\r.

*Sơ đồ nối chân dự kiến:*

| STM32F103 | Thiết bị |
|---|---|
| PA0 - ADC1_IN0 | Ngõ ra biến trở |
| 3V3 | Chân nguồn biến trở |
| GND | Chân GND biến trở |
| PA9 - USART1_TX | USB-TTL RXD |
| GND | USB-TTL GND |

Timer kích hoạt ADC bằng tín hiệu nội bộ, không cần chân Timer bên ngoài.

Trạng thái: Chưa bổ sung chương trình và video demo.

## 2. Kết nối ST-Link V2

| ST-Link | STM32F103 |
|---|---|
| SWDIO | PA13 |
| SWCLK | PA14 |
| GND | GND |
| 3.3V / Vref | 3V3 (theo loại ST-Link) |

Cấp nguồn đúng cách cho board và nối chung GND.

## 3. Biên dịch và nạp

Mở Terminal tại thư mục của từng bài.

*Biên dịch:*

make clean
make

*Kiểm tra ST-Link:*

st-info --probe

*Nạp chương trình:*

make flash

## 4. Công cụ sử dụng

- Ubuntu Linux
- Visual Studio Code
- ARM GNU Toolchain
- STM32 Standard Peripheral Library
- ST-Link V2
- Minicom

## 5. Video demo

- Bài 01: [Bổ sung]
- Bài 02: [Bổ sung]
- Bài 03: [Bổ sung]
- Bài 04: [Bổ sung]

## 6. Repository

- [Week 1](https://github.com/SONXUCXICH/Buoi_So_1_STM32F103_LED)
- [Week 2](https://github.com/SONXUCXICH/Buoi_So_2_STM32F103)
- [Week 3](https://github.com/SONXUCXICH/Buoi_So_03_STM32F103)