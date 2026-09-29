#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

#include "bmp280.h"

#include <stdio.h>


static void delay_ms(uint32_t ms)
{
    uint32_t i;
    uint32_t j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 8000; j++)
        {
            __NOP();
        }
    }
}


/* =========================
   USART1
   PA9  -> TX
   PA10 -> RX
   ========================= */

static void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );

    /* PA9 - TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 - RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;

    USART_InitStructure.USART_Mode =
        USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}


/* =========================
   Gửi 1 ký tự
   ========================= */

static void USART1_SendChar(char c)
{
    USART_SendData(USART1, c);

    while (USART_GetFlagStatus(
               USART1,
               USART_FLAG_TXE) == RESET)
    {
    }
}


/* =========================
   Gửi chuỗi
   ========================= */

static void USART1_SendString(const char *str)
{
    while (*str)
    {
        USART1_SendChar(*str);
        str++;
    }
}


/* =========================
   MAIN
   ========================= */

int main(void)
{
    uint8_t id;

    float temperature;
    float pressure;

    USART1_Init();

    USART1_SendString(
        "\r\n========================\r\n"
    );

    USART1_SendString(
        " STM32F103 + BMP280\r\n"
    );

    USART1_SendString(
        " I2C + UART\r\n"
    );

    USART1_SendString(
        "========================\r\n"
    );


    /* Khoi tao I2C1 */
    I2C1_Init();


    /* Doc ID BMP280 */
    id = BMP280_ReadID();

    USART1_SendString("BMP280 ID = ");

    if (id == 0x58)
    {
        USART1_SendString("0x58\r\n");
    }
    else
    {
        USART1_SendString("ERROR\r\n");
    }


    /* Khoi tao BMP280 */
    if (BMP280_Init() == 0)
    {
        USART1_SendString(
            "BMP280 ERROR!\r\n"
        );

        while (1)
        {
        }
    }

    USART1_SendString(
        "BMP280 OK!\r\n"
    );


    while (1)
    {
        /* Đọc nhiệt độ */
        BMP280_ReadTemperature(
            &temperature
        );

        /* Đọc áp suất */
        BMP280_ReadPressure(
            &pressure
        );


        USART1_SendString(
            "Temperature: "
        );

        /*
         * Tạm thời gửi phần nguyên
         */
        {
            int t = (int)temperature;

            if (t < 0)
            {
                USART1_SendChar('-');
                t = -t;
            }

            USART1_SendChar(
                '0' + (t / 10)
            );

            USART1_SendChar(
                '0' + (t % 10)
            );
        }

        USART1_SendString(
            " C\r\n"
        );


        USART1_SendString(
            "Pressure: "
        );

        {
            int p =
                (int)(pressure / 100.0f);

            char buf[10];
            int i = 0;
            int j;

            if (p == 0)
            {
                USART1_SendChar('0');
            }
            else
            {
                while (p > 0)
                {
                    buf[i++] =
                        '0' + (p % 10);

                    p /= 10;
                }

                for (j = i - 1; j >= 0; j--)
                {
                    USART1_SendChar(buf[j]);
                }
            }
        }

        USART1_SendString(
            " hPa\r\n"
        );

        USART1_SendString(
            "--------------------\r\n"
        );

        delay_ms(1000);
    }
}