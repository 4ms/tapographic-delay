// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
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
// Driver for ADC.

#ifndef ADC_H_
#define ADC_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

enum AdcChannel {
  // pots
  ADC_SCALE_POT,
  ADC_FEEDBACK_POT,
  ADC_MODULATION_POT,
  ADC_DRYWET_POT,
  ADC_MORPH_POT,
  ADC_GAIN_POT,

  // bipolar CV
  ADC_SCALE_CV,
  ADC_FEEDBACK_CV,
  ADC_MODULATION_CV,
  ADC_DRYWET_CV,

  // unipolar CV
  ADC_CLOCK_CV,
  ADC_FSR_CV,
  ADC_VEL_CV,
  ADC_TAPTRIG_CV,
  ADC_CHANNEL_LAST
};

class Adc {
public:
  void Deinit(void) {
    ADC_Cmd(ADC3, DISABLE);
    ADC_DMACmd(ADC3, DISABLE);
    ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);
    DMA_Cmd(DMA2_Stream0, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);

    ADC_Cmd(ADC1, DISABLE);
    ADC_DMACmd(ADC1, DISABLE);
    ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);
    DMA_Cmd(DMA2_Stream4, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);
  }

  void Init() {

    // Initialize peripherals
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 |
                           RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
                           RCC_APB2Periph_ADC3, ENABLE);

    // Configure analog input pins
    GPIO_InitTypeDef gpio_init;

    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;

    // pots
    gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //Channel 6, 7
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; //Channel 11, 12, 13, 14
    GPIO_Init(GPIOC, &gpio_init);

    // cvs
    gpio_init.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3; //Channel 1, 2, 3
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; //Channel 4, 5, 6, 7, 8
    GPIO_Init(GPIOF, &gpio_init);

    // Use DMA to automatically copy ADC data register to values_ buffer.
    DMA_InitTypeDef dma_init;

    dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dma_init.DMA_Mode = DMA_Mode_Circular;
    dma_init.DMA_Priority = DMA_Priority_High;
    dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    // DMA2 Stream 4 Channel 0 for pots
    dma_init.DMA_Channel = DMA_Channel_0;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[0];
    dma_init.DMA_BufferSize = 6;

    DMA_Init(DMA2_Stream4, &dma_init);
    DMA_Cmd(DMA2_Stream4, ENABLE);

    // DMA2 Stream 0 Channel 2 for CVs
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
    dma_init.DMA_Memory0BaseAddr = (uint32_t)&values_[ADC_SCALE_CV];
    dma_init.DMA_BufferSize = 8;
    dma_init.DMA_Channel = DMA_Channel_2;

    DMA_Init(DMA2_Stream0, &dma_init);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    // Common ADC init
    ADC_CommonInitTypeDef adc_common_init;
    adc_common_init.ADC_Mode = ADC_Mode_Independent;
    adc_common_init.ADC_Prescaler = ADC_Prescaler_Div8;
    adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&adc_common_init);

    // ADC init
    ADC_InitTypeDef adc_init;
    adc_init.ADC_Resolution = ADC_Resolution_12b;
    adc_init.ADC_ScanConvMode = ENABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    adc_init.ADC_DataAlign = ADC_DataAlign_Left;

    // ADC1 for pots
    adc_init.ADC_NbrOfConversion = 6;
    ADC_Init(ADC1, &adc_init);

    // ADC3 for CVs
    adc_init.ADC_NbrOfConversion = 8;
    ADC_Init(ADC3, &adc_init);

    // ADC1 channel configuration (pots)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, ADC_MORPH_POT+1, ADC_SampleTime_480Cycles); //PC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, ADC_DRYWET_POT+1, ADC_SampleTime_480Cycles); //PC2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, ADC_MODULATION_POT+1, ADC_SampleTime_480Cycles); //PC3
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, ADC_GAIN_POT+1, ADC_SampleTime_480Cycles); //PC4
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, ADC_FEEDBACK_POT+1, ADC_SampleTime_480Cycles); //PA6
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, ADC_SCALE_POT+1, ADC_SampleTime_480Cycles); //PA7

    // ADC3 channel configuration (CVs)
    ADC_RegularChannelConfig(ADC3, ADC_Channel_8, ADC_SCALE_CV-5, ADC_SampleTime_480Cycles); //PF10
    ADC_RegularChannelConfig(ADC3, ADC_Channel_7, ADC_FEEDBACK_CV-5, ADC_SampleTime_480Cycles); //PF9
    ADC_RegularChannelConfig(ADC3, ADC_Channel_6, ADC_MODULATION_CV-5, ADC_SampleTime_480Cycles); //PF8
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5, ADC_DRYWET_CV-5, ADC_SampleTime_480Cycles); //PF7
    ADC_RegularChannelConfig(ADC3, ADC_Channel_1, ADC_CLOCK_CV-5, ADC_SampleTime_480Cycles); //PA1
    ADC_RegularChannelConfig(ADC3, ADC_Channel_3, ADC_FSR_CV-5, ADC_SampleTime_480Cycles); //PA3
    ADC_RegularChannelConfig(ADC3, ADC_Channel_4, ADC_VEL_CV-5, ADC_SampleTime_480Cycles); //PF6
    ADC_RegularChannelConfig(ADC3, ADC_Channel_2, ADC_TAPTRIG_CV-5, ADC_SampleTime_480Cycles); //PA2

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
    ADC_DMACmd(ADC3, ENABLE);
    ADC_Cmd(ADC3, ENABLE);

    Convert();

    for(int i=0; i<1000000; i++) {
      __asm("nop");

    }
  }

  inline void DeInit() {
    DMA_Cmd(DMA2_Stream0, DISABLE);
    DMA_Cmd(DMA2_Stream4, DISABLE);
    ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);
    ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);
    ADC_Cmd(ADC1, DISABLE);
    ADC_Cmd(ADC3, DISABLE);
    ADC_DMACmd(ADC1, DISABLE);
    ADC_DMACmd(ADC3, DISABLE);
    ADC_DeInit();
  }

  inline void Convert() {
    ADC_SoftwareStartConv(ADC1);
    ADC_SoftwareStartConv(ADC3);
  }

  inline void Wait() {
   while (ADC_GetSoftwareStartConvStatus(ADC1) != RESET);
   while (ADC_GetSoftwareStartConvStatus(ADC3) != RESET);
  }

  inline uint16_t value(uint8_t channel) const {
    return values_[channel];
  }
  inline float float_value(uint8_t channel) const {
    return static_cast<float>(values_[channel]) / 65536.0f;
  }

 private:
  uint16_t values_[ADC_CHANNEL_LAST];
};

#endif
