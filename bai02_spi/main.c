#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"

// MAX7219 LED Matrix 8x8
// PA4 = CS
// PA5 = CLK
// PA7 = DIN

#define MAX_DECODE       0x09
#define MAX_INTENSITY    0x0A
#define MAX_SCAN_LIMIT   0x0B
#define MAX_SHUTDOWN     0x0C
#define MAX_TEST         0x0F

volatile uint32_t tick_ms = 0;


// Ma tran diem cho cac so tu 0 den 9.
// Moi so gom 8 hang, moi hang 8 bit.
// Bit 1 tuong ung mot LED sang.

const uint8_t numbers[10][8] =
{
    // So 0
    {
        0x3C,
        0x66,
        0x6E,
        0x76,
        0x66,
        0x66,
        0x3C,
        0x00
    },

    // So 1
    {
        0x18,
        0x38,
        0x18,
        0x18,
        0x18,
        0x18,
        0x3C,
        0x00
    },

    // So 2
    {
        0x3C,
        0x66,
        0x06,
        0x0C,
        0x30,
        0x60,
        0x7E,
        0x00
    },

    // So 3
    {
        0x3C,
        0x66,
        0x06,
        0x1C,
        0x06,
        0x66,
        0x3C,
        0x00
    },

    // So 4
    {
        0x0C,
        0x1C,
        0x3C,
        0x6C,
        0x7E,
        0x0C,
        0x0C,
        0x00
    },

    // So 5
    {
        0x7E,
        0x60,
        0x7C,
        0x06,
        0x06,
        0x66,
        0x3C,
        0x00
    },

    // So 6
    {
        0x1C,
        0x30,
        0x60,
        0x7C,
        0x66,
        0x66,
        0x3C,
        0x00
    },

    // So 7
    {
        0x7E,
        0x06,
        0x0C,
        0x18,
        0x30,
        0x30,
        0x30,
        0x00
    },

    // So 8
    {
        0x3C,
        0x66,
        0x66,
        0x3C,
        0x66,
        0x66,
        0x3C,
        0x00
    },

    // So 9
    {
        0x3C,
        0x66,
        0x66,
        0x3E,
        0x06,
        0x0C,
        0x38,
        0x00
    }
};


// Ham ngat SysTick moi 1 ms.
void SysTick_Handler(void)
{
    tick_ms++;
}


// Tao delay theo SysTick.
void DelayMs(uint32_t ms)
{
    uint32_t start = tick_ms;

    while ((uint32_t)(tick_ms - start) < ms)
    {
    }
}


// Cau hinh GPIO.
void GPIO_Config(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOC,
        ENABLE
    );

    // PA4 dieu khien CS.
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);

    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    // PA5 va PA7 su dung SPI1.
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);

    // LED PC13 tren board.
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOC, &gpio);

    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}


// Cau hinh SPI1 Master.
void SPI1_Config(void)
{
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_SPI1,
        ENABLE
    );

    SPI_I2S_DeInit(SPI1);

    SPI_StructInit(&spi);

    spi.SPI_Direction = SPI_Direction_1Line_Tx;

    spi.SPI_Mode = SPI_Mode_Master;

    spi.SPI_DataSize = SPI_DataSize_8b;

    spi.SPI_CPOL = SPI_CPOL_Low;

    spi.SPI_CPHA = SPI_CPHA_1Edge;

    spi.SPI_NSS = SPI_NSS_Soft;

    spi.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_64;

    spi.SPI_FirstBit = SPI_FirstBit_MSB;

    SPI_Init(SPI1, &spi);

    SPI_NSSInternalSoftwareConfig(
        SPI1,
        SPI_NSSInternalSoft_Set
    );

    SPI_Cmd(SPI1, ENABLE);
}


// Gui mot byte qua SPI.
void SPI1_SendByte(uint8_t data)
{
    while (
        SPI_I2S_GetFlagStatus(
            SPI1,
            SPI_I2S_FLAG_TXE
        ) == RESET
    )
    {
    }

    SPI_I2S_SendData(SPI1, data);
}


// Ghi dia chi va du lieu cho MAX7219.
void MAX7219_Write(uint8_t address, uint8_t data)
{
    // Bat dau truyen.
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);

    SPI1_SendByte(address);

    SPI1_SendByte(data);

    // Cho thanh ghi truyen trong.
    while (
        SPI_I2S_GetFlagStatus(
            SPI1,
            SPI_I2S_FLAG_TXE
        ) == RESET
    )
    {
    }

    // Cho SPI truyen xong.
    while (
        SPI_I2S_GetFlagStatus(
            SPI1,
            SPI_I2S_FLAG_BSY
        ) == SET
    )
    {
    }

    // Ket thuc va chot du lieu.
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


// Xoa tat ca 64 LED.
void MAX7219_Clear(void)
{
    uint8_t row;

    for (row = 1; row <= 8; row++)
    {
        MAX7219_Write(row, 0x00);
    }
}


// Khoi tao MAX7219.
void MAX7219_Config(void)
{
    // Dua chip vao shutdown trong luc cau hinh.
    MAX7219_Write(MAX_SHUTDOWN, 0x00);

    // Tat che do kiem tra.
    MAX7219_Write(MAX_TEST, 0x00);

    // Quan trong:
    // LED Matrix khong su dung decode.
    MAX7219_Write(MAX_DECODE, 0x00);

    // Dieu khien du 8 hang.
    MAX7219_Write(MAX_SCAN_LIMIT, 0x07);

    // Do sang muc 3.
    MAX7219_Write(MAX_INTENSITY, 0x03);

    MAX7219_Clear();

    // Bat che do hoat dong binh thuong.
    MAX7219_Write(MAX_SHUTDOWN, 0x01);
}


// Hien thi mot chu so tu 0 den 9.
void MAX7219_ShowDigit(uint8_t digit)
{
    uint8_t row;

    if (digit > 9)
    {
        return;
    }

    // Truyen lan luot 8 hang.
    for (row = 0; row < 8; row++)
    {
        MAX7219_Write(
            row + 1,
            numbers[digit][row]
        );
    }
}


// Chuong trinh chinh.
int main(void)
{
    uint8_t digit = 0;

    uint32_t last_count;
    uint32_t last_led;

    SystemCoreClockUpdate();

    // Tao ngat SysTick moi 1 ms.
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    GPIO_Config();

    SPI1_Config();

    MAX7219_Config();

    // Test sang tat ca 64 LED.
    MAX7219_Write(MAX_TEST, 0x01);

    DelayMs(350);

    MAX7219_Write(MAX_TEST, 0x00);

    MAX7219_Clear();

    // Bat dau hien thi so 0.
    MAX7219_ShowDigit(digit);

    last_count = tick_ms;
    last_led = tick_ms;

    while (1)
    {
        uint32_t now = tick_ms;

        // Dao LED PC13 moi 500 ms.
        if ((uint32_t)(now - last_led) >= 500U)
        {
            last_led += 500U;

            GPIOC->ODR ^= GPIO_Pin_13;
        }

        // Chuyen sang so moi moi 1 giay.
        if ((uint32_t)(now - last_count) >= 1000U)
        {
            last_count += 1000U;

            digit++;

            if (digit > 9)
            {
                digit = 0;
            }

            MAX7219_ShowDigit(digit);
        }
    }
}
