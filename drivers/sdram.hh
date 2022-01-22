// Copyright 2013 Radoslaw Kwiecien.
// Copyright 2015 Dan Green.
//
// Author: Radoslaw Kwiecien (radek@dxp.pl)
// Source: http://en.radzio.dxp.pl/stm32f429idiscovery/
// Modified by: Dan Green (matthias.puech@gmail.com)
// Modified by: Matthias Puech (matthias.puech@gmail.com)
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
// SDRAM driver

#ifndef SDRAM_H_
#define SDRAM_H_

#include <stm32f4xx.h>

#define SDRAM_BASE 0xD0000000
#define SDRAM_SIZE 0x02000000

class SDRAM {
 public:
  void Clear() {
    volatile uint32_t ptr = 0;
    for(ptr = SDRAM_BASE; ptr < (SDRAM_BASE + SDRAM_SIZE - 1); ptr += 4)
      *((uint32_t *)ptr) = 0xFFFFFFFF;
  }

  bool Test() {

    uint32_t addr;
    uint32_t i;

    addr=SDRAM_BASE;
    for (i=0;i<SDRAM_SIZE/2;i++){
      while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
      *((uint16_t *)addr) = (uint16_t)i;
      addr += 2;
    }

    addr=SDRAM_BASE;
    for (i=0;i<SDRAM_SIZE/2;i++){
      while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
      uint16_t t = *((uint16_t*)addr);
      if (t != (uint16_t)i)
        return false;
      addr += 2;
    }

    return true;
  }

  /* Wait until the SDRAM controller is ready */
  void Wait() {
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);
  }


  void Init() {
    GPIO_InitTypeDef            GPIO_InitStructure;
    FMC_SDRAMTimingInitTypeDef  FMCT;
    FMC_SDRAMInitTypeDef        FMCI;
    FMC_SDRAMCommandTypeDef     FMCC;

    /* GPIO configuration ------------------------------------------------------*/
    /* Enable GPIOs clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB
                           | RCC_AHB1Periph_GPIOC
                           | RCC_AHB1Periph_GPIOD
                           | RCC_AHB1Periph_GPIOE
                           | RCC_AHB1Periph_GPIOF
                           | RCC_AHB1Periph_GPIOG,
                           ENABLE);

    /* Common GPIO configuration */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    /* GPIOD configuration */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  |GPIO_Pin_1  |GPIO_Pin_8 |GPIO_Pin_9 |
      GPIO_Pin_10 |GPIO_Pin_14 |GPIO_Pin_15;

    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOE configuration */
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
      GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
      GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* GPIOF configuration */
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
      GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
      GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* GPIOG configuration */
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_2
      | GPIO_Pin_4 | GPIO_Pin_5
      | GPIO_Pin_8 | GPIO_Pin_15;

    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* GPIOH configuration */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6 , GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* GPIOI configuration */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource0 , GPIO_AF_FMC);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Enable FMC clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

    int32_t timeout;

    FMCT.FMC_LoadToActiveDelay      = 2;                         //  2   1 clock cycle = 1 / 90MHz = 11ns
    FMCT.FMC_ExitSelfRefreshDelay   = 6;                         //  6   TXSR: min=66ns  (  6x11ns)
    FMCT.FMC_SelfRefreshTime        = 4;                         //  4   TRAS: min=44ns  (  4x11ns) max = 120k (ns)
    FMCT.FMC_RowCycleDelay          = 6;                         //  6   TRC:  min=66ns  (  7x11ns)
    FMCT.FMC_WriteRecoveryTime      = 2;                         //  2   TWR:  min=1+7ns (1+1x11ns)
    FMCT.FMC_RPDelay                = 2;                         //  2   TRP:  20ns      (  2x11ns)
    FMCT.FMC_RCDDelay               = 2;                         //  2   TRCD: 20ns      (  2x11ns)

    FMCI.FMC_Bank                   = FMC_Bank2_SDRAM;              //  FMC SDRAM control configuration
    FMCI.FMC_ColumnBitsNumber       = FMC_ColumnBits_Number_9b;     //  Row addressing   : [ 8:0]
    FMCI.FMC_RowBitsNumber          = FMC_RowBits_Number_13b;       //  Column addressing: [11:0]
    FMCI.FMC_SDMemoryDataWidth      = FMC_SDMemory_Width_16b;
    FMCI.FMC_InternalBankNumber     = FMC_InternalBank_Number_4;
    FMCI.FMC_CASLatency             = FMC_CAS_Latency_3;            //  CL: Cas Latency = 3 clock cycles
    FMCI.FMC_WriteProtection        = FMC_Write_Protection_Disable;
    FMCI.FMC_SDClockPeriod          = FMC_SDClock_Period_2;
    FMCI.FMC_ReadBurst              = FMC_Read_Burst_Disable;
    FMCI.FMC_ReadPipeDelay          = FMC_ReadPipe_Delay_1;
    FMCI.FMC_SDRAMTimingStruct      = &FMCT;
    FMC_SDRAMInit(&FMCI);                                           //  FMC SDRAM bank initialization

    //  Configure a clock configuration enable command
    FMCC.FMC_CommandMode            = FMC_Command_Mode_CLK_Enabled;
    FMCC.FMC_CommandTarget          = FMC_Command_Target_bank2;
    FMCC.FMC_AutoRefreshNumber      = 1;
    FMCC.FMC_ModeRegisterDefinition = 0;
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);
    FMC_SDRAMCmdConfig(&FMCC);

    for(timeout = 0x00; timeout < 0xD0000; timeout++) {}

    //  Configure a PALL (precharge all) command
    FMCC.FMC_CommandMode            = FMC_Command_Mode_PALL;
    FMCC.FMC_CommandTarget          = FMC_Command_Target_bank2;
    FMCC.FMC_AutoRefreshNumber      = 1;
    FMCC.FMC_ModeRegisterDefinition = 0;
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy));
    FMC_SDRAMCmdConfig(&FMCC);

    //  Configure a Auto-Refresh command
    FMCC.FMC_CommandMode            = FMC_Command_Mode_AutoRefresh;
    FMCC.FMC_CommandTarget          = FMC_Command_Target_bank2;
    FMCC.FMC_AutoRefreshNumber      = 1;
    FMCC.FMC_ModeRegisterDefinition = 0;
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy));
    FMC_SDRAMCmdConfig(&FMCC);

    //  Configure a load Mode register command
#define SDRAM_BURST_LENGTH_1                ((uint16_t)0x0000)
#define SDRAM_BURST_LENGTH_2                ((uint16_t)0x0001)
#define SDRAM_BURST_LENGTH_4                ((uint16_t)0x0002)
#define SDRAM_BURST_LENGTH_8                ((uint16_t)0x0004)
#define SDRAM_BURST_TYPE_SEQUENTIAL         ((uint16_t)0x0000)
#define SDRAM_BURST_TYPE_INTERLEAVED        ((uint16_t)0x0008)
#define SDRAM_CAS_LATENCY_2                 ((uint16_t)0x0020)
#define SDRAM_CAS_LATENCY_3                 ((uint16_t)0x0030)
#define SDRAM_OPERATING_MODE_STANDARD       ((uint16_t)0x0000)
#define SDRAM_WRITEBURST_MODE_PROGRAMMED    ((uint16_t)0x0000)
#define SDRAM_WRITEBURST_MODE_SINGLE        ((uint16_t)0x0200)

#define SDRAM_MODEREG (SDRAM_BURST_LENGTH_2|SDRAM_BURST_TYPE_SEQUENTIAL|SDRAM_CAS_LATENCY_3|SDRAM_OPERATING_MODE_STANDARD|SDRAM_WRITEBURST_MODE_SINGLE)

    FMCC.FMC_CommandMode            = FMC_Command_Mode_LoadMode;
    FMCC.FMC_CommandTarget          = FMC_Command_Target_bank2;
    FMCC.FMC_AutoRefreshNumber      = 1;
    FMCC.FMC_ModeRegisterDefinition = SDRAM_MODEREG;
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy));
    FMC_SDRAMCmdConfig(&FMCC);

    FMC_SetRefreshCount(683);
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);
  }    
};

#endif
