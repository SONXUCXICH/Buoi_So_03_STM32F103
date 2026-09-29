#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"

// Thay ma lop va ma nhom neu can.
#define MESSAGE_PREFIX "D23DTM01<11>:BTN:"

#define DEBOUNCE_MS 30U
#define TX_BUFFER_SIZE 64U

// Bien thoi gian do SysTick cap nhat.
volatile uint32_t tick_ms = 0;

// Bo dem gui du lieu qua DMA.
static char tx_buffer[TX_BUFFER_SIZE];

// Trang thai truyen DMA.
static uint8_t dma_busy = 0;


// SysTick tao ngat moi 1 ms.
void SysTick_Handler(void)
{
    tick_ms++;
}


// Cau hinh nut nhan PB0.
void Button_Config(void)
{
    GPIO_InitTypeDef gpio;

    // Bat clock GPIOB.
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB,
        ENABLE
    );

    // PB0 su dung dien tro keo len noi bo.
    GPIO_StructInit(&gpio);

    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOB, &gpio);
}


// Cau hinh USART1.
// PA9 la TX, PA10 la RX.
void USART1_Config(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );

    GPIO_StructInit(&gpio);

    // PA9 truyen UART.
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);

    // PA10 nhan UART.
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &gpio);

    // Thiet lap UART 115200, 8N1.
    USART_StructInit(&uart);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;

    uart.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;

    uart.USART_Mode =
        USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &uart);

    // Kich hoat USART1.
    USART_Cmd(USART1, ENABLE);
}


// Cau hinh DMA1 Channel 4 cho USART1 TX.
void DMA_TX_Config(void)
{
    DMA_InitTypeDef dma;

    // Bat clock DMA1.
    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_DMA1,
        ENABLE
    );

    DMA_DeInit(DMA1_Channel4);

    DMA_StructInit(&dma);

    // Dia chi thanh ghi du lieu UART.
    dma.DMA_PeripheralBaseAddr =
        (uint32_t)&USART1->DR;

    // Dia chi bo dem trong RAM.
    dma.DMA_MemoryBaseAddr =
        (uint32_t)tx_buffer;

    // DMA chuyen du lieu tu RAM sang UART.
    dma.DMA_DIR = DMA_DIR_PeripheralDST;

    dma.DMA_BufferSize = 1;

    dma.DMA_PeripheralInc =
        DMA_PeripheralInc_Disable;

    dma.DMA_MemoryInc =
        DMA_MemoryInc_Enable;

    // Moi lan truyen mot byte.
    dma.DMA_PeripheralDataSize =
        DMA_PeripheralDataSize_Byte;

    dma.DMA_MemoryDataSize =
        DMA_MemoryDataSize_Byte;

    // Truyen mot ban tin roi dung.
    dma.DMA_Mode = DMA_Mode_Normal;

    dma.DMA_Priority = DMA_Priority_High;
    dma.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel4, &dma);

    // Cho phep USART1 su dung DMA de truyen.
    USART_DMACmd(
        USART1,
        USART_DMAReq_Tx,
        ENABLE
    );
}


// Chuyen so nguyen thanh chuoi.
// Tao ban tin dung dinh dang cua de bai.
uint16_t Build_Message(uint32_t number)
{
    const char *prefix = MESSAGE_PREFIX;

    char digits[10];

    uint8_t count = 0;
    uint16_t length = 0;

    // Them ma lop va ma nhom.
    while (*prefix != '\0')
    {
        tx_buffer[length++] = *prefix;
        prefix++;
    }

    // Tach tung chu so.
    do
    {
        digits[count++] =
            (char)('0' + number % 10U);

        number /= 10U;

    } while (number > 0U);

    // Dao nguoc de gui dung thu tu.
    while (count > 0U)
    {
        count--;

        tx_buffer[length++] = digits[count];
    }

    // De bai yeu cau ky tu LF roi CR.
    tx_buffer[length++] = '\n';
    tx_buffer[length++] = '\r';

    return length;
}


// Bat dau truyen du lieu bang DMA.
void DMA_Send(uint16_t length)
{
    // Tat kenh truoc khi thiet lap lan truyen moi.
    DMA_Cmd(DMA1_Channel4, DISABLE);

    // Xoa co DMA cua lan truyen truoc.
    DMA_ClearFlag(DMA1_FLAG_GL4);

    // Cai dat so byte can truyen.
    DMA_SetCurrDataCounter(
        DMA1_Channel4,
        length
    );

    dma_busy = 1;

    // Bat dau DMA.
    DMA_Cmd(DMA1_Channel4, ENABLE);
}


// Kiem tra trang thai DMA.
// 0: chua xong hoac dang truyen.
// 1: truyen thanh cong.
// 2: truyen gap loi.
uint8_t DMA_Service(void)
{
    if (dma_busy == 0U)
    {
        return 0;
    }

    // Kiem tra loi truyen DMA.
    if (DMA_GetFlagStatus(DMA1_FLAG_TE4) != RESET)
    {
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_GL4);

        dma_busy = 0;

        return 2;
    }

    // Kiem tra DMA da truyen het buffer hay chua.
    if (DMA_GetFlagStatus(DMA1_FLAG_TC4) != RESET)
    {
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_GL4);

        dma_busy = 0;

        return 1;
    }

    return 0;
}


// Chuong trinh chinh.
int main(void)
{
    uint8_t raw;
    uint8_t last_raw;
    uint8_t stable;

    uint32_t change_ms;

    // Tong so lan nhan nut hop le.
    uint32_t press_count = 0;

    // So thu tu ban tin tiep theo.
    uint32_t next_to_send = 1;

    // Cap nhat clock.
    SystemCoreClockUpdate();

    // Khoi tao cac ngoai vi.
    Button_Config();
    USART1_Config();
    DMA_TX_Config();

    // Tao ngat SysTick moi 1 ms.
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    // Doc trang thai nut ban dau.
    stable =
        (GPIO_ReadInputDataBit(
            GPIOB,
            GPIO_Pin_0
        ) != Bit_RESET) ? 1U : 0U;

    last_raw = stable;

    change_ms = tick_ms;

    while (1)
    {
        uint32_t now = tick_ms;

        // Doc muc logic PB0.
        raw =
            (GPIO_ReadInputDataBit(
                GPIOB,
                GPIO_Pin_0
            ) != Bit_RESET) ? 1U : 0U;

        // Phat hien tin hieu nut thay doi.
        if (raw != last_raw)
        {
            last_raw = raw;
            change_ms = now;
        }

        // Chi chap nhan neu on dinh du 30 ms.
        if (
            (uint32_t)(now - change_ms) >= DEBOUNCE_MS &&
            raw != stable
        )
        {
            stable = raw;

            // Nut duoc nhan khi PB0 = LOW.
            if (stable == 0U)
            {
                press_count++;
            }
        }

        // Kiem tra trang thai lan truyen DMA.
        uint8_t result = DMA_Service();

        // Neu truyen thanh cong thi chuyen ban tin.
        if (result == 1U)
        {
            next_to_send++;
        }

        // Neu gap loi, thu gui lai ban tin cu.
        // Chi tao ban tin moi khi DMA dang ranh.
        if (
            dma_busy == 0U &&
            next_to_send <= press_count
        )
        {
            uint16_t length;

            length = Build_Message(next_to_send);

            DMA_Send(length);
        }
    }
}