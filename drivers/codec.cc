// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Codec driver

#include "codec.hh"

#define DELAY_MS(x)                             \
do {							\
  register unsigned int i;				\
  for (i = 0; i < (25000*x); ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

#define ASSERT(x)                               \
  { if (!(x)) return false; }                   \

#define WAIT(x)                                 \
  { int timeout = CODEC_TIMEOUT;                \
    while (x)                                   \
      if (timeout-- == 0) return false;         \
  }                                             \

#define WAIT_LONG(x)                            \
  { int timeout = CODEC_LONG_TIMEOUT;           \
    while (x)                                   \
      if (timeout-- == 0) return false;         \
  }                                             \

#define CODEC_IS_SLAVE 0
#define CODEC_IS_MASTER 1

#define CS4271_ADDR_0 0b0010000
#define CODEC_ADDRESS           (CS4271_ADDR_0<<1)

#define CS4271_NUM_REGS 6	/* we only initialize the first 6 registers, the 7th is for pre/post-init and the 8th is read-only */

#define CS4271_REG_MODECTRL1	1
#define CS4271_REG_DACCTRL		2
#define CS4271_REG_DACMIX		3
#define CS4271_REG_DACAVOL		4
#define CS4271_REG_DACBVOL		5
#define CS4271_REG_ADCCTRL		6
#define CS4271_REG_MODECTRL2	7
#define CS4271_REG_CHIPID		8	/*Read-only*/

//Reg 1 (MODECTRL1):
#define SINGLE_SPEED		(0b00<<6)		/* 4-50kHz */
#define DOUBLE_SPEED		(0b10<<6)		/* 50-100kHz */
#define QUAD_SPEED			(0b11<<6)		/* 100-200kHz */
#define	RATIO0				(0b00<<4)		/* See table page 28 and 29 of datasheet */
#define	RATIO1				(0b01<<4)
#define	RATIO2				(0b10<<4)
#define	RATIO3				(0b11<<4)
#define	MASTER				(1<<3)
#define	SLAVE				(0<<3)
#define	DIF_LEFTJUST_24b	(0b000)
#define	DIF_I2S_24b			(0b001)
#define	DIF_RIGHTJUST_16b	(0b010)
#define	DIF_RIGHTJUST_24b	(0b011)
#define	DIF_RIGHTJUST_20b	(0b100)
#define	DIF_RIGHTJUST_18b	(0b101)

//Reg 2 (DACCTRL)
#define AUTOMUTE 		(1<<7)
#define SLOW_FILT_SEL	(1<<6)
#define FAST_FILT_SEL	(0<<6)
#define DEEMPH_OFF		(0<<4)
#define DEEMPH_44		(1<<4)
#define DEEMPH_48		(2<<4)
#define DEEMPH_32		(3<<4)
#define	SOFT_RAMPUP		(1<<3) /*An un-mute will be performed after executing a filter mode change, after a MCLK/LRCK ratio change or error, and after changing the Functional Mode.*/
#define	SOFT_RAMPDOWN	(1<<2) /*A mute will be performed prior to executing a filter mode change.*/
#define INVERT_SIGA_POL	(1<<1) /*When set, this bit activates an inversion of the signal polarity for the appropriate channel*/
#define INVERT_SIGB_POL	(1<<0)

//Reg 3 (DACMIX)
#define BEQA			(1<<6) /*If set, ignore AOUTB volume setting, and instead make channel B's volume equal channel A's volume as set by AOUTA */
#define SOFTRAMP		(1<<5) /*Allows level changes, both muting and attenuation, to be implemented by incrementally ramping, in 1/8 dB steps, from the current level to the new level at a rate of 1 dB per 8 left/right clock periods */
#define	ZEROCROSS		(1<<4) /*Dictates that signal level changes, either by attenuation changes or muting, will occur on a signal zero crossing to minimize audible artifacts*/
#define ATAPI_aLbR		(0b1001) /*channel A==>Left, channel B==>Right*/

//Reg 4: DACAVOL
//Reg 5: DACBVOL

//Reg 6 (ADCCTRL)
#define DITHER16		(1<<5) /*activates the Dither for 16-Bit Data feature*/
#define ADC_DIF_I2S		(1<<4) /*I2S, up to 24-bit data*/
#define ADC_DIF_LJUST	(0<<4) /*Left Justified, up to 24-bit data (default)*/
#define MUTEA			(1<<3)
#define MUTEB			(1<<2)
#define HPFDisableA		(1<<1)
#define HPFDisableB		(1<<0)

//Reg 7 (MODECTRL2)
#define PDN		(1<<0)		/* Power Down Enable */
#define CPEN	(1<<1)		/* Control Port Enable */
#define FREEZE	(1<<2)		/* Freezes effects of register changes */
#define MUTECAB	(1<<3)		/* Internal AND gate on AMUTEC and BMUTEC */
#define LOOP	(1<<4)		/* Digital loopback (ADC->DAC) */

Codec* Codec::instance_;

void Codec::InitGPIO(void)
{
	/* Enable I2S and I2C GPIO clocks */
	RCC_AHB1PeriphClockCmd(CODEC_I2C_GPIO_CLOCK | CODEC_I2S_GPIO_CLOCK, ENABLE);

  GPIO_InitTypeDef gpio;

	/* I2C SCL and SDA pins configuration */
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_OD;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN;
  GPIO_Init(CODEC_I2C_GPIO, &gpio);

	/* Connect pins to I2C peripheral */
	GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SCL_PINSRC, CODEC_I2C_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SDA_PINSRC, CODEC_I2C_GPIO_AF);

	/* I2S output pins configuration: WS, SCK SD0 SDI MCK pins */
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_I2S_WS_PIN;	GPIO_Init(CODEC_I2S_GPIO_WS, &gpio);
	gpio.GPIO_Pin = CODEC_I2S_SDI_PIN;	GPIO_Init(CODEC_I2S_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODEC_I2S_SCK_PIN;	GPIO_Init(CODEC_I2S_GPIO_SCK, &gpio);
	gpio.GPIO_Pin = CODEC_I2S_SDO_PIN;	GPIO_Init(CODEC_I2S_GPIO_SDO, &gpio);

	GPIO_PinAFConfig(CODEC_I2S_GPIO_WS, CODEC_I2S_WS_PINSRC, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_GPIO_SCK, CODEC_I2S_SCK_PINSRC, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_GPIO_SDO, CODEC_I2S_SDO_PINSRC, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_GPIO_SDI, CODEC_I2S_SDI_PINSRC, CODEC_I2Sext_GPIO_AF);

  gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_Speed = GPIO_Speed_100MHz;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

  gpio.GPIO_Pin = CODEC_I2S_MCK_PIN; GPIO_Init(CODEC_I2S_MCK_GPIO, &gpio);
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF);

  // Reset pin
	RCC_AHB1PeriphClockCmd(CODEC_RESET_RCC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_RESET_pin; GPIO_Init(CODEC_RESET_GPIO, &gpio);
	CODEC_RESET_LOW;
}

bool Codec::WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue)
{
	uint8_t Byte1 = RegisterAddr;
	uint8_t Byte2 = RegisterValue;
	
	/*!< While the bus is busy */
	WAIT_LONG(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY));

	/* Start the config sequence */
	I2C_GenerateSTART(CODEC_I2C, ENABLE);

	/* Test on EV5 and clear it */
	WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT));

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Transmit the first address for write operation */
	I2C_SendData(CODEC_I2C, Byte1);

	/* Test on EV8 and clear it */
	WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

	/* Prepare the register value to be sent */
	I2C_SendData(CODEC_I2C, Byte2);

	/*!< Wait till all data have been physically transferred on the bus */
	WAIT_LONG(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF));

	/* End the configuration sequence */
	I2C_GenerateSTOP(CODEC_I2C, ENABLE);

	return true;
}

bool Codec::InitControlInterface(void)
{
  // Enable I2C
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(CODEC_I2C_CLK, ENABLE);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x33;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  
	I2C_DeInit(CODEC_I2C);
	I2C_Init(CODEC_I2C, &I2C_InitStructure);
	I2C_Cmd(CODEC_I2C, ENABLE);

  // Set codec pin
	CODEC_RESET_HIGH;
	DELAY_MS(2);

  // Set codec registers
  //Control Port Enable and Power Down Enable  
	ASSERT(WriteRegister(CS4271_REG_MODECTRL2, CPEN | PDN));

  ASSERT(WriteRegister(CS4271_REG_MODECTRL1, SINGLE_SPEED | RATIO0 | SLAVE | DIF_I2S_24b));
	ASSERT(WriteRegister(CS4271_REG_DACCTRL, FAST_FILT_SEL | DEEMPH_OFF));
	ASSERT(WriteRegister(CS4271_REG_DACMIX, ATAPI_aLbR));
	ASSERT(WriteRegister(CS4271_REG_DACAVOL, 0));
	ASSERT(WriteRegister(CS4271_REG_DACBVOL, 0));
  ASSERT(WriteRegister(CS4271_REG_ADCCTRL, ADC_DIF_I2S));

  //Power Down disable
	ASSERT(WriteRegister(CS4271_REG_MODECTRL2, CPEN))

	return true;
}


//Reference Manual 00090 recommends these values for 48kHz:
// I2SDIV = 3
// ODD = 1
// PLL_N = 258
// PLL_R= 3
#define PLLI2S_N   258
#define PLLI2S_R   3

// These values work decently, setting the sampling rate to 48014Hz:
// #define PLLI2S_N   295
// #define PLLI2S_R   6

//This is only used for SAI audio, but must be >2 and <15
#define PLLI2S_Q   4

void InitAudioInterface(uint32_t sample_rate)
{
	I2S_InitTypeDef I2S_InitStructure;

	RCC_PLLI2SConfig(PLLI2S_N, PLLI2S_Q, PLLI2S_R);

	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

	// CODEC_I2S peripheral configuration for master TX
	SPI_I2S_DeInit(CODEC_I2S);
	I2S_InitStructure.I2S_AudioFreq = sample_rate;
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
  	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;

	// Initialize the I2S main channel for TX
	I2S_Init(CODEC_I2S, &I2S_InitStructure);

	// Initialize the I2S extended channel for RX
	I2S_FullDuplexConfig(CODEC_I2S_EXT, &I2S_InitStructure);

  	I2S_Cmd(CODEC_I2S, ENABLE);
	I2S_Cmd(CODEC_I2S_EXT, ENABLE);

  	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	RCC_PLLI2SCmd(ENABLE);
}

volatile int16_t tx_buffer[CODEC_BUFFER_SIZE * 2 * 2];
volatile int16_t rx_buffer[CODEC_BUFFER_SIZE * 2 * 2];

void InitAudioDMA(void) {
  RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE);

  // DeInit both DMA
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S_DMA_STREAM);
	DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);

  // Setup RX/TX DMA
  DMA_InitTypeDef dma_tx;
  
	dma_tx.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;
	dma_tx.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
	dma_tx.DMA_Memory0BaseAddr = (uint32_t)&tx_buffer;
	dma_tx.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dma_tx.DMA_BufferSize = CODEC_BUFFER_SIZE * 2 * 2;
	dma_tx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_tx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_tx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_tx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_tx.DMA_Mode = DMA_Mode_Circular;
	dma_tx.DMA_Priority = DMA_Priority_High;
	dma_tx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_tx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_tx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_tx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
	DMA_Init(AUDIO_I2S_DMA_STREAM, &dma_tx);
  
  DMA_InitTypeDef dma_rx;

	dma_rx.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;
	dma_rx.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
	dma_rx.DMA_Memory0BaseAddr = (uint32_t)&rx_buffer;
	dma_rx.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_rx.DMA_BufferSize = CODEC_BUFFER_SIZE * 2 * 2;
	dma_rx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_rx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_rx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_rx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_rx.DMA_Mode = DMA_Mode_Circular;
	dma_rx.DMA_Priority = DMA_Priority_High;
	dma_rx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_rx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_rx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_rx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &dma_rx);

  // Enable the interrupts.
  DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);
  NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);  

  // Start DMA from/to codec.
  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

  DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);
}

bool Codec::Init(int32_t sample_rate, FillBufferCallback cb) {
  callback_ = cb;
  instance_ = this;

  InitGPIO();
  InitAudioInterface(sample_rate);
  InitAudioDMA();
  ASSERT(InitControlInterface());

  return true;
};

void Codec::Stop() {
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

void Codec::Fill(int32_t offset) {
  offset *= CODEC_BUFFER_SIZE * 2;
  volatile short* in = &rx_buffer[offset];
  volatile short* out = &tx_buffer[offset];
  (*callback_)((Frame*)(in), (Frame*)(out));
}

extern "C"
{
  void DMA1_Stream3_IRQHandler(void) {
    if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_TC) {
      AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_TC;
      Codec::instance_->Fill(1);
    }
    if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_HT) {
      AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_HT;
      Codec::instance_->Fill(0);
    }
  }
}
