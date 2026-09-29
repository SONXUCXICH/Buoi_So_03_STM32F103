# STM32F103 - Bare Metal Week 3

Thực hành lập trình STM32F103C8T6 trên Ubuntu Linux bằng ngôn ngữ C, sử dụng STM32 Standard Peripheral Library, ARM GCC và ST-Link V2.

## Bài 01: I2C - BMP280

### Yêu cầu
- Cấu hình I2C giao tiếp với cảm biến BMP280.
- Đọc nhiệt độ và áp suất.
- Truyền dữ liệu đo được lên máy tính thông qua UART.

### Sơ đồ nối chân

| STM32F103 | BMP280 |
|---|---|
| PB6 - I2C1_SCL | SCL |
| PB7 - I2C1_SDA | SDA |
| 3V3 | VCC |
| GND | GND |

Kết nối UART:

| STM32F103 | USB-TTL |
|---|---|
| PA9 - TX | RXD |
| GND | GND |

## Bài 02: SPI - MAX7219 LED Matrix 8x8

### Yêu cầu
- Cấu hình SPI1 ở chế độ Master.
- Giao tiếp với MAX7219.
- Điều khiển ma trận LED 8x8 hiển thị giá trị.

### Chương trình
- Kiểm tra toàn bộ 64 LED.
- Hiển thị các số từ 0 đến 9.
- Chuyển số mỗi giây.
- Sử dụng SysTick tạo mốc thời gian.

### Sơ đồ nối chân

| STM32F103 | MAX7219 |
|---|---|
| PA4 | CS |
| PA5 - SPI1_SCK | CLK |
| PA7 - SPI1_MOSI | DIN |
| GND | GND |
| Nguồn 5V | VCC |


## Bài 03: UART + Nút nhấn + DMA

### Yêu cầu
- Cấu hình UART, nút nhấn và DMA.
- Mỗi lần nhấn hợp lệ, biến đếm tăng 1.
- Truyền bản tin từ STM32 lên PC bằng UART + DMA.
- Không sử dụng hàm chờ UART.

### Chương trình
- Đọc nút nhấn PB0.
- Chống dội nút nhấn 30ms.
- USART1 hoạt động ở 115200 baud.
- DMA1 Channel 4 truyền dữ liệu UART.

### Sơ đồ nối chân

| STM32F103 | Thiết bị |
|---|---|
| PB0 | Nút nhấn |
| GND | Chân còn lại của nút |
| PA9 - USART1_TX | USB-TTL RXD |
| PA10 - USART1_RX | USB-TTL TXD (nếu cần) |
| GND | USB-TTL GND |

## Bài 04: ADC + Timer + UART + DMA

### Yêu cầu
- Sử dụng biến trở xoay làm đầu vào ADC.
- Timer hoạt động với tần số 100Hz.
- Timer kích hoạt quá trình chuyển đổi ADC.
- DMA tự động lưu dữ liệu ADC vào RAM.
- Thu thập 100 mẫu trong một giây.
- Sử dụng ngắt DMA Half-transfer và Transfer-complete.
- Truyền dữ liệu an toàn lên PC bằng UART.

### Sơ đồ nối chân dự kiến

| STM32F103 | Thiết bị |
|---|---|
| PA0 - ADC1_IN0 | Chân giữa biến trở |
| 3V3 | Chân ngoài thứ nhất của biến trở |
| GND | Chân ngoài thứ hai của biến trở |
| PA9 - USART1_TX | USB-TTL RXD |
| GND | USB-TTL GND |

Timer kích hoạt ADC bằng tín hiệu nội bộ.

## Kết nối ST-Link V2

| ST-Link | STM32F103 |
|---|---|
| SWDIO | PA13 |
| SWCLK | PA14 |
| GND | GND |
| 3.3V / Vref | 3V3, tùy loại ST-Link |

## Biên dịch và nạp chương trình

Di chuyển vào thư mục của bài cần thực hiện.

Biên dịch:

    make clean
    make

Kiểm tra ST-Link:

    st-info --probe

Nạp chương trình:

    make flash

Mở Minicom để kiểm tra UART:

    sudo minicom -D /dev/ttyUSB0 -b 115200

Thay ttyUSB0 bằng cổng thiết bị thực tế.

## Công cụ sử dụng

- STM32F103C8T6 Blue Pill
- Ubuntu Linux
- Visual Studio Code
- ARM GNU Toolchain
- STM32 Standard Peripheral Library
- ST-Link V2
- USB-TTL
- Minicom
