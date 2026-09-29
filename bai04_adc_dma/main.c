#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include <stdint.h>

/* =========================================================
 * CẤU HÌNH
 * ========================================================= */

#define ADC_BUFFER_SIZE    100

/* Bộ đệm ADC */
volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Cờ báo từ ngắt DMA */
volatile uint8_t adc_half_ready = 0;
volatile uint8_t adc_full_ready = 0;


/* =========================================================
 * UART1
 *
 * PA9  -> TX
 * PA10 -> RX
 * Baudrate = 115200
 * ========================================================= */

static void USART1_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Bật clock GPIOA và USART1 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );

    /* -------------------------
     * PA9 - USART1 TX
     * Alternate Function Push-Pull
     * ------------------------- */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* -------------------------
     * PA10 - USART1 RX
     * Floating input
     * ------------------------- */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* -------------------------
     * Cấu hình USART1
     * ------------------------- */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode =
        USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);

    /* Bật USART1 */
    USART_Cmd(USART1, ENABLE);
}


/* =========================================================
 * Gửi 1 ký tự UART
 * ========================================================= */

static void USART1_SendChar_Custom(char c)
{
    /* Chờ thanh ghi truyền rỗng */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }

    USART_SendData(USART1, (uint16_t)c);
}


/* =========================================================
 * Gửi chuỗi UART
 * ========================================================= */

static void USART1_SendString_Custom(const char *str)
{
    while (*str)
    {
        USART1_SendChar_Custom(*str);
        str++;
    }
}


/* =========================================================
 * Gửi số nguyên unsigned 16-bit
 *
 * Không dùng printf để chương trình nhẹ hơn.
 * ========================================================= */

static void USART1_SendUInt16(uint16_t value)
{
    char buffer[5];
    int i = 0;

    /* Trường hợp value = 0 */
    if (value == 0)
    {
        USART1_SendChar_Custom('0');
        return;
    }

    /* Tách từng chữ số */
    while (value > 0)
    {
        buffer[i] = (char)('0' + (value % 10));
        value = value / 10;
        i++;
    }

    /* In ngược lại */
    while (i > 0)
    {
        i--;
        USART1_SendChar_Custom(buffer[i]);
    }
}


/* =========================================================
 * GPIO ADC
 *
 * PA0 = ADC1_IN0
 * ========================================================= */

static void ADC_GPIO_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Bật clock GPIOA */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA,
        ENABLE
    );

    /* PA0 làm ngõ vào analog */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/* =========================================================
 * ADC1
 *
 * PA0 = ADC1 Channel 0
 *
 * ADC được kích bởi TIM3 TRGO
 * ========================================================= */

static void ADC1_Init_Custom(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    /* Bật clock ADC1 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_ADC1,
        ENABLE
    );

    /* -----------------------------------------------------
     * ADC1 cấu hình
     * ----------------------------------------------------- */

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;

    /* Chỉ dùng 1 channel */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;

    /* Không chạy liên tục.
     * Mỗi lần TIM3 TRGO xuất hiện -> ADC chạy 1 lần.
     */
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;

    /* Trigger từ TIM3 TRGO */
    ADC_InitStructure.ADC_ExternalTrigConv =
        ADC_ExternalTrigConv_T3_TRGO;

    /* Kết quả ADC nằm bên phải */
    ADC_InitStructure.ADC_DataAlign =
        ADC_DataAlign_Right;

    /* Chỉ có 1 conversion */
    ADC_InitStructure.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    /* -----------------------------------------------------
     * Channel 0 = PA0
     *
     * Sample time = 55.5 cycles
     * ----------------------------------------------------- */

    ADC_RegularChannelConfig(
        ADC1,
        ADC_Channel_0,
        1,
        ADC_SampleTime_55Cycles5
    );

    /* -----------------------------------------------------
     * Cho phép ADC sử dụng DMA
     * ----------------------------------------------------- */

    ADC_DMACmd(ADC1, ENABLE);

    /* Cho phép external trigger */
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    /* Bật ADC */
    ADC_Cmd(ADC1, ENABLE);

    /* -----------------------------------------------------
     * Reset calibration
     * ----------------------------------------------------- */

    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1))
    {
    }

    /* Bắt đầu calibration */
    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1))
    {
    }
}


/* =========================================================
 * DMA1 Channel 1
 *
 * ADC1 -> DMA1 Channel 1 -> RAM
 *
 * 100 mẫu
 *
 * 0   -> 49  : Half Transfer
 * 50  -> 99  : Transfer Complete
 * ========================================================= */

static void DMA1_ADC_Init_Custom(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* -----------------------------------------------------
     * Bật clock DMA1
     * DMA nằm trên AHB
     * ----------------------------------------------------- */

    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_DMA1,
        ENABLE
    );

    /* Reset DMA Channel 1 */
    DMA_DeInit(DMA1_Channel1);

    /* -----------------------------------------------------
     * Địa chỉ ngoại vi
     *
     * ADC1->DR
     * ----------------------------------------------------- */

    DMA_InitStructure.DMA_PeripheralBaseAddr =
        (uint32_t)&ADC1->DR;

    /* -----------------------------------------------------
     * Địa chỉ RAM
     *
     * adc_buffer
     * ----------------------------------------------------- */

    DMA_InitStructure.DMA_MemoryBaseAddr =
        (uint32_t)adc_buffer;

    /* ADC -> RAM */
    DMA_InitStructure.DMA_DIR =
        DMA_DIR_PeripheralSRC;

    /* 100 phần tử */
    DMA_InitStructure.DMA_BufferSize =
        ADC_BUFFER_SIZE;

    /* ADC Data Register không tăng địa chỉ */
    DMA_InitStructure.DMA_PeripheralInc =
        DMA_PeripheralInc_Disable;

    /* RAM tự tăng địa chỉ */
    DMA_InitStructure.DMA_MemoryInc =
        DMA_MemoryInc_Enable;

    /* ADC = 16 bit */
    DMA_InitStructure.DMA_PeripheralDataSize =
        DMA_PeripheralDataSize_HalfWord;

    /* RAM = 16 bit */
    DMA_InitStructure.DMA_MemoryDataSize =
        DMA_MemoryDataSize_HalfWord;

    /* DMA chạy vòng */
    DMA_InitStructure.DMA_Mode =
        DMA_Mode_Circular;

    /* Ưu tiên cao */
    DMA_InitStructure.DMA_Priority =
        DMA_Priority_High;

    /* Không dùng Memory-to-Memory */
    DMA_InitStructure.DMA_M2M =
        DMA_M2M_Disable;

    DMA_Init(
        DMA1_Channel1,
        &DMA_InitStructure
    );

    /* -----------------------------------------------------
     * Cho phép 2 loại ngắt:
     *
     * HT = Half Transfer
     * TC = Transfer Complete
     * ----------------------------------------------------- */

    DMA_ITConfig(
        DMA1_Channel1,
        DMA_IT_HT | DMA_IT_TC,
        ENABLE
    );

    /* -----------------------------------------------------
     * Cấu hình NVIC cho DMA1 Channel 1
     * ----------------------------------------------------- */

    NVIC_InitStructure.NVIC_IRQChannel =
        DMA1_Channel1_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
        0;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority =
        0;

    NVIC_InitStructure.NVIC_IRQChannelCmd =
        ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /* Bật DMA */
    DMA_Cmd(
        DMA1_Channel1,
        ENABLE
    );
}


/* =========================================================
 * TIM3
 *
 * TIM3 tạo TRGO với tần số 100 Hz
 *
 * Clock TIM3 = 72 MHz
 *
 * Prescaler = 7199
 * ARR       = 99
 *
 * 72 MHz / (7199 + 1) / (99 + 1)
 * = 100 Hz
 *
 * => 1 trigger mỗi 10 ms
 * ========================================================= */

static void TIM3_Init_Custom(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* Bật clock TIM3 */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_TIM3,
        ENABLE
    );

    /* -----------------------------------------------------
     * Timer frequency
     *
     * 72 MHz / 7200 = 10 kHz
     *
     * 10 kHz / 100 = 100 Hz
     * ----------------------------------------------------- */

    TIM_TimeBaseStructure.TIM_Prescaler =
        7199;

    TIM_TimeBaseStructure.TIM_Period =
        99;

    TIM_TimeBaseStructure.TIM_CounterMode =
        TIM_CounterMode_Up;

    TIM_TimeBaseStructure.TIM_ClockDivision =
        TIM_CKD_DIV1;

    TIM_TimeBaseStructure.TIM_RepetitionCounter =
        0;

    TIM_TimeBaseInit(
        TIM3,
        &TIM_TimeBaseStructure
    );

    /* -----------------------------------------------------
     * TIM3 Update Event -> TRGO
     *
     * ADC1 lấy TRGO này làm trigger.
     * ----------------------------------------------------- */

    TIM_SelectOutputTrigger(
        TIM3,
        TIM_TRGOSource_Update
    );

    /* Bật TIM3 */
    TIM_Cmd(
        TIM3,
        ENABLE
    );
}


/* =========================================================
 * Gửi 50 mẫu ADC đầu tiên
 *
 * adc_buffer[0] -> adc_buffer[49]
 * ========================================================= */

static void Send_First_Half(void)
{
    uint16_t i;

    for (i = 0; i < 50; i++)
    {
        USART1_SendUInt16(adc_buffer[i]);

        /*
         * Theo yêu cầu dữ liệu ngắt nhau bằng \n\r
         */
        USART1_SendString_Custom("\n\r");
    }
}


/* =========================================================
 * Gửi 50 mẫu ADC thứ hai
 *
 * adc_buffer[50] -> adc_buffer[99]
 * ========================================================= */

static void Send_Second_Half(void)
{
    uint16_t i;

    for (i = 50; i < 100; i++)
    {
        USART1_SendUInt16(adc_buffer[i]);

        /*
         * Theo yêu cầu dữ liệu ngắt nhau bằng \n\r
         */
        USART1_SendString_Custom("\n\r");
    }
}


/* =========================================================
 * DMA1 Channel 1 INTERRUPT
 *
 * Half Transfer:
 *   DMA đã ghi xong 50 mẫu đầu.
 *
 * Transfer Complete:
 *   DMA đã ghi xong 100 mẫu.
 * ========================================================= */

void DMA1_Channel1_IRQHandler(void)
{
    /* -----------------------------------------------------
     * Half Transfer
     * ----------------------------------------------------- */

    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET)
    {
        /* Xóa cờ ngắt */
        DMA_ClearITPendingBit(DMA1_IT_HT1);

        /* Báo cho main */
        adc_half_ready = 1;
    }

    /* -----------------------------------------------------
     * Transfer Complete
     * ----------------------------------------------------- */

    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        /* Xóa cờ ngắt */
        DMA_ClearITPendingBit(DMA1_IT_TC1);

        /* Báo cho main */
        adc_full_ready = 1;
    }
}


/* =========================================================
 * MAIN
 * ========================================================= */

int main(void)
{
    /* -----------------------------------------------------
     * 1. GPIO PA0 - ADC
     * ----------------------------------------------------- */

    ADC_GPIO_Init_Custom();

    /* -----------------------------------------------------
     * 2. USART1
     * ----------------------------------------------------- */

    USART1_Init_Custom();

    /* -----------------------------------------------------
     * 3. DMA
     * ----------------------------------------------------- */

    DMA1_ADC_Init_Custom();

    /* -----------------------------------------------------
     * 4. ADC1
     * ----------------------------------------------------- */

    ADC1_Init_Custom();

    /* -----------------------------------------------------
     * 5. TIM3 = 100 Hz
     * ----------------------------------------------------- */

    TIM3_Init_Custom();

    /* -----------------------------------------------------
     * Vòng lặp chính
     * ----------------------------------------------------- */

    while (1)
    {
        /* -------------------------------------------------
         * DMA đã ghi xong 50 mẫu đầu
         * ------------------------------------------------- */

        if (adc_half_ready)
        {
            adc_half_ready = 0;

            Send_First_Half();
        }

        /* -------------------------------------------------
         * DMA đã ghi xong 100 mẫu
         * ------------------------------------------------- */

        if (adc_full_ready)
        {
            adc_full_ready = 0;

            Send_Second_Half();
        }
    }
}