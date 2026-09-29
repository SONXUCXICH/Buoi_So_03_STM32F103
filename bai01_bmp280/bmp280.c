#include "bmp280.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"


/* =========================
   Calibration data
   ========================= */

static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;

static uint16_t dig_P1;
static int16_t  dig_P2;
static int16_t  dig_P3;
static int16_t  dig_P4;
static int16_t  dig_P5;
static int16_t  dig_P6;
static int16_t  dig_P7;
static int16_t  dig_P8;
static int16_t  dig_P9;

static int32_t t_fine;


/* =========================
   Delay
   ========================= */

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
   I2C1
   PB6 = SCL
   PB7 = SDA
   ========================= */

void I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* Clock GPIOB */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB,
        ENABLE
    );

    /* Clock I2C1 */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_I2C1,
        ENABLE
    );

    /* PB6 = SCL
       PB7 = SDA */

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_6 | GPIO_Pin_7;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_AF_OD;

    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /* I2C configuration */

    I2C_InitStructure.I2C_ClockSpeed =
        100000;

    I2C_InitStructure.I2C_Mode =
        I2C_Mode_I2C;

    I2C_InitStructure.I2C_DutyCycle =
        I2C_DutyCycle_2;

    I2C_InitStructure.I2C_OwnAddress1 =
        0x00;

    I2C_InitStructure.I2C_Ack =
        I2C_Ack_Enable;

    I2C_InitStructure.I2C_AcknowledgedAddress =
        I2C_AcknowledgedAddress_7bit;

    I2C_Init(
        I2C1,
        &I2C_InitStructure
    );

    I2C_Cmd(
        I2C1,
        ENABLE
    );
}


/* =========================
   I2C WRITE
   ========================= */

static void I2C_WriteByte(
    uint8_t device,
    uint8_t reg,
    uint8_t data)
{
    while (
        I2C_GetFlagStatus(
            I2C1,
            I2C_FLAG_BUSY
        )
    )
    {
    }


    /* START */

    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_MODE_SELECT
        )
    )
    {
    }


    /* Address + WRITE */

    I2C_Send7bitAddress(
        I2C1,
        device << 1,
        I2C_Direction_Transmitter
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED
        )
    )
    {
    }


    /* Register */

    I2C_SendData(
        I2C1,
        reg
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_BYTE_TRANSMITTED
        )
    )
    {
    }


    /* Data */

    I2C_SendData(
        I2C1,
        data
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_BYTE_TRANSMITTED
        )
    )
    {
    }


    /* STOP */

    I2C_GenerateSTOP(
        I2C1,
        ENABLE
    );
}


/* =========================
   I2C READ 1 BYTE
   ========================= */

static uint8_t I2C_ReadByte(
    uint8_t device,
    uint8_t reg)
{
    uint8_t data;


    while (
        I2C_GetFlagStatus(
            I2C1,
            I2C_FLAG_BUSY
        )
    )
    {
    }


    /* START */

    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_MODE_SELECT
        )
    )
    {
    }


    /* Address + WRITE */

    I2C_Send7bitAddress(
        I2C1,
        device << 1,
        I2C_Direction_Transmitter
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED
        )
    )
    {
    }


    /* Register */

    I2C_SendData(
        I2C1,
        reg
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_BYTE_TRANSMITTED
        )
    )
    {
    }


    /* REPEATED START */

    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_MODE_SELECT
        )
    )
    {
    }


    /* Address + READ */

    I2C_Send7bitAddress(
        I2C1,
        device << 1,
        I2C_Direction_Receiver
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED
        )
    )
    {
    }


    /* NACK */

    I2C_AcknowledgeConfig(
        I2C1,
        DISABLE
    );

    /* STOP */

    I2C_GenerateSTOP(
        I2C1,
        ENABLE
    );


    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_BYTE_RECEIVED
        )
    )
    {
    }


    data = I2C_ReceiveData(I2C1);


    /* Enable ACK again */

    I2C_AcknowledgeConfig(
        I2C1,
        ENABLE
    );

    return data;
}


/* =========================
   I2C READ MULTIPLE BYTES
   ========================= */

static void I2C_ReadBytes(
    uint8_t device,
    uint8_t reg,
    uint8_t *buffer,
    uint8_t length)
{
    uint8_t i;


    while (
        I2C_GetFlagStatus(
            I2C1,
            I2C_FLAG_BUSY
        )
    )
    {
    }


    /* START */

    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_MODE_SELECT
        )
    )
    {
    }


    /* Address + WRITE */

    I2C_Send7bitAddress(
        I2C1,
        device << 1,
        I2C_Direction_Transmitter
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED
        )
    )
    {
    }


    /* Register */

    I2C_SendData(
        I2C1,
        reg
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_BYTE_TRANSMITTED
        )
    )
    {
    }


    /* REPEATED START */

    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_MODE_SELECT
        )
    )
    {
    }


    /* Address + READ */

    I2C_Send7bitAddress(
        I2C1,
        device << 1,
        I2C_Direction_Receiver
    );

    while (
        !I2C_CheckEvent(
            I2C1,
            I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED
        )
    )
    {
    }


    /* Read data */

    for (i = 0; i < length; i++)
    {
        if (i == length - 1)
        {
            I2C_AcknowledgeConfig(
                I2C1,
                DISABLE
            );

            I2C_GenerateSTOP(
                I2C1,
                ENABLE
            );
        }

        while (
            !I2C_CheckEvent(
                I2C1,
                I2C_EVENT_MASTER_BYTE_RECEIVED
            )
        )
        {
        }

        buffer[i] =
            I2C_ReceiveData(I2C1);
    }


    I2C_AcknowledgeConfig(
        I2C1,
        ENABLE
    );
}


/* =========================
   READ BMP280 ID
   ========================= */

uint8_t BMP280_ReadID(void)
{
    return I2C_ReadByte(
        BMP280_ADDR,
        BMP280_REG_ID
    );
}


/* =========================
   READ CALIBRATION
   ========================= */

static void BMP280_ReadCalibration(void)
{
    uint8_t data[24];


    I2C_ReadBytes(
        BMP280_ADDR,
        0x88,
        data,
        24
    );


    dig_T1 =
        ((uint16_t)data[1] << 8) |
        data[0];

    dig_T2 =
        (int16_t)(
            ((uint16_t)data[3] << 8) |
            data[2]
        );

    dig_T3 =
        (int16_t)(
            ((uint16_t)data[5] << 8) |
            data[4]
        );


    dig_P1 =
        ((uint16_t)data[7] << 8) |
        data[6];

    dig_P2 =
        (int16_t)(
            ((uint16_t)data[9] << 8) |
            data[8]
        );

    dig_P3 =
        (int16_t)(
            ((uint16_t)data[11] << 8) |
            data[10]
        );

    dig_P4 =
        (int16_t)(
            ((uint16_t)data[13] << 8) |
            data[12]
        );

    dig_P5 =
        (int16_t)(
            ((uint16_t)data[15] << 8) |
            data[14]
        );

    dig_P6 =
        (int16_t)(
            ((uint16_t)data[17] << 8) |
            data[16]
        );

    dig_P7 =
        (int16_t)(
            ((uint16_t)data[19] << 8) |
            data[18]
        );

    dig_P8 =
        (int16_t)(
            ((uint16_t)data[21] << 8) |
            data[20]
        );

    dig_P9 =
        (int16_t)(
            ((uint16_t)data[23] << 8) |
            data[22]
        );
}


/* =========================
   BMP280 INIT
   ========================= */

uint8_t BMP280_Init(void)
{
    uint8_t id;


    delay_ms(100);


    id = BMP280_ReadID();


    if (id != BMP280_CHIP_ID)
    {
        return 0;
    }


    /* Calibration */

    BMP280_ReadCalibration();


    /*
     * ctrl_meas = 0x27
     *
     * Temperature oversampling = x1
     * Pressure oversampling    = x1
     * Normal mode
     */

    I2C_WriteByte(
        BMP280_ADDR,
        BMP280_REG_CTRL,
        0x27
    );


    /*
     * Config
     */

    I2C_WriteByte(
        BMP280_ADDR,
        BMP280_REG_CONFIG,
        0x00
    );


    return 1;
}


/* =========================
   TEMPERATURE COMPENSATION
   ========================= */

static float BMP280_CompensateTemperature(
    int32_t adc_T)
{
    float var1;
    float var2;
    float temperature;


    var1 =
        ((float)adc_T / 16384.0f
        - (float)dig_T1 / 1024.0f)
        * (float)dig_T2;


    var2 =
        (((float)adc_T / 131072.0f
        - (float)dig_T1 / 8192.0f)
        *
        ((float)adc_T / 131072.0f
        - (float)dig_T1 / 8192.0f))
        * (float)dig_T3;


    t_fine =
        (int32_t)(var1 + var2);


    temperature =
        (var1 + var2) / 5120.0f;


    return temperature;
}


/* =========================
   READ TEMPERATURE
   ========================= */

void BMP280_ReadTemperature(
    float *temperature)
{
    uint8_t data[3];

    int32_t adc_T;


    I2C_ReadBytes(
        BMP280_ADDR,
        BMP280_REG_TEMP,
        data,
        3
    );


    adc_T =
        ((int32_t)data[0] << 12) |
        ((int32_t)data[1] << 4) |
        ((int32_t)data[2] >> 4);


    *temperature =
        BMP280_CompensateTemperature(
            adc_T
        );
}


/* =========================
   PRESSURE COMPENSATION
   ========================= */

static float BMP280_CompensatePressure(
    int32_t adc_P)
{
    float var1;
    float var2;
    float pressure;


    var1 =
        ((float)t_fine / 2.0f)
        - 64000.0f;


    var2 =
        var1 * var1
        * (float)dig_P6
        / 32768.0f;


    var2 =
        var2
        + var1 * (float)dig_P5 * 2.0f;


    var2 =
        (var2 / 4.0f)
        + ((float)dig_P4 * 65536.0f);


    var1 =
        ((float)dig_P3
        * var1 * var1
        / 524288.0f
        + (float)dig_P2
        * var1)
        / 524288.0f;


    var1 =
        (1.0f + var1 / 32768.0f)
        * (float)dig_P1;


    if (var1 == 0.0f)
    {
        return 0.0f;
    }


    pressure =
        1048576.0f
        - (float)adc_P;


    pressure =
        (pressure
        - (var2 / 4096.0f))
        * 6250.0f
        / var1;


    var1 =
        (float)dig_P9
        * pressure * pressure
        / 2147483648.0f;


    var2 =
        pressure
        * (float)dig_P8
        / 32768.0f;


    pressure =
        pressure
        + (var1 + var2
        + ((float)dig_P7))
        / 16.0f;


    return pressure;
}


/* =========================
   READ PRESSURE
   ========================= */

void BMP280_ReadPressure(
    float *pressure)
{
    uint8_t data[3];

    int32_t adc_P;


    I2C_ReadBytes(
        BMP280_ADDR,
        BMP280_REG_PRESS,
        data,
        3
    );


    adc_P =
        ((int32_t)data[0] << 12) |
        ((int32_t)data[1] << 4) |
        ((int32_t)data[2] >> 4);


    /*
     * Phải đọc nhiệt độ trước
     * để cập nhật t_fine
     */

    *pressure =
        BMP280_CompensatePressure(
            adc_P
        );
}