

IIC_type IIC = {0};
volatile uint32_t Temp = 0;

#define SCL_1                  \
    {                          \
        MFP->GPIO_DATA |= SCL; \
        Delay_Thrid_HZ();      \
    }
#define SCL_0                   \
    {                           \
        MFP->GPIO_DATA &= ~SCL; \
        Delay_Thrid_HZ();       \
    }
#define SDA_1                  \
    {                          \
        MFP->GPIO_DATA |= SDA; \
        Delay_Thrid_HZ();      \
    }
#define SDA_0                   \
    {                           \
        MFP->GPIO_DATA &= ~SDA; \
        Delay_Thrid_HZ();       \
    }

#if 0
void Delay_Thrid_HZ()
{
    for (uint32_t i = 0; i <= 1000; i++)
    {
        Temp = i;
    }
}
void DelaySend()
{
    for (uint32_t i = 0; i <= 5000; i++)
    {
        Temp = i;
    }
}
#else

/* (CPU 14Mhz) -> (5us) ,(5us) * 3 ~= 17us -> (IIC 58.5Khz) */

/* SCL波形随意,只要是稳定的周期波形即可(这里为了方便用了占空比1:2的) */

void Delay_Thrid_HZ()
{
    /* 1/3 Hz : 5us */
    for (uint32_t i = 0; i <= 10; i++)
    {
        Temp = i;
    }
}

/* 建议大于200us */
void DelaySend()
{
    for (uint32_t i = 0; i <= 300; i++)
    {
        Temp = i;
    }
}
#endif

#if 1
/* 建议I2C频率,周期要高于10倍的 (I2C_Read_Data()等 通信PACKET中的代码) 的执行时间 */
/* -> (CPU 14Mhz) -> (IIC 57Khz) */
#else
/* 建议 分析 (I2C_Read_Data()等 通信PACKET中的代码) 的执行时间,合并时间到Delay()中 */
#endif

void inline I2C_Read_Data(uint8_t i)
{
    if (MFP->GPIO_DATA & SDA)
    {
        IIC.ReceiveData |= (BIT7 >> i);
    }
}

void IIC_SEND_START()
{
    SDA_1;
    SCL_1;

    SDA_0;
    SCL_1;

    SCL_0;
}

void IIC_SEND_DATA(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((data << i) & BIT7)
        {
            SDA_1;
        }
        else
        {
            SDA_0;
        }

        SCL_1;
        SCL_0;
    }

    SDA_1;

    MFP->GPO_EN &= ~SDA;
    MFP->GPI_EN |= SDA;

    SCL_1;

    if (MFP->GPIO_DATA & SDA)
        IIC.Crc = FALSE;
    else
        IIC.Crc = TRUE;

    SCL_0;

    MFP->GPI_EN &= ~SDA;
    MFP->GPO_EN |= SDA;

    SDA_1;

    SCL_0;
}

void IIC_RECEIVE_DATA()
{
    IIC.ReceiveData = 0;

    MFP->GPO_EN &= ~SDA;
    MFP->GPI_EN |= SDA;

    SDA_0;

#if 1
    /* 没有去支持 I2C时钟拉伸 -> 加延时(建议大于50us) */
    for (uint32_t i = 0; i <= 100; i++)
    {
        Temp = i;
    }
#else
    /* 时钟拉伸的标准处理 */
#endif

    for (uint8_t i = 0; i < 8; i++)
    {
        SCL_1;

        I2C_Read_Data(i);

        SCL_0;

        Delay_Thrid_HZ();
    }

    MFP->GPI_EN &= ~SDA;
    MFP->GPO_EN |= SDA;

    SDA_1;

    SCL_1;

    SCL_0;

    SDA_1;
}

void IIC_SEND_STOP()
{
    SDA_0;
    SCL_0;

    SCL_1;

    SDA_1;
    SCL_1;
}

void IIC_Read(uint8_t Reg)
{
    IIC_SEND_START();
    IIC_SEND_DATA(IIC.Addr << 1);
    IIC_SEND_DATA(Reg);
    DelaySend();

    IIC_SEND_START();
    IIC_SEND_DATA(IIC.Addr << 1 | BIT0);
    IIC_RECEIVE_DATA();
    IIC_SEND_STOP();
    DelaySend();
}

void IIC_Write(uint8_t Reg, uint8_t Data)
{
    IIC_SEND_START();
    IIC_SEND_DATA(IIC.Addr << 1);
    IIC_SEND_DATA(Reg);
    IIC_SEND_DATA(Data);
    IIC_SEND_STOP();
    DelaySend();
}

void IIC_Process()
{
    /* TEST */
    while (1)
    {
        DelayMs(1000);

        IIC_Read(0x00);
        IIC_Write(TEST_REG, IIC.ReceiveData);
        IIC_Read(0x01);
        IIC_Write(TEST_REG, IIC.ReceiveData);
        IIC_Read(0x02);
        IIC_Write(TEST_REG, IIC.ReceiveData);

        IIC_Read(0x31);
        IIC_Write(TEST_REG, IIC.ReceiveData);
        IIC_Read(0x32);
        IIC_Write(TEST_REG, IIC.ReceiveData);
        IIC_Read(0x33);
        IIC_Write(TEST_REG, IIC.ReceiveData);
    }
}