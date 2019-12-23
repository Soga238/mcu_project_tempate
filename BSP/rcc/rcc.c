/*********************************************************************
*                      Hangzhou Lingzhi Lzlinks                      *
*                        Internet of Things                          *
**********************************************************************
*                                                                    *
*            (c) 2018 - 8102 Hangzhou Lingzhi Lzlinks                *
*                                                                    *
*       www.lzlinks.com     Support: embedzjh@gmail.com              *
*                                                                    *
**********************************************************************
*                                                                    *
*       RCC * SYS CLK                                                *
*                                                                    *
**********************************************************************
*/
#include "./rcc.h"
#include "stm32f10x_flash.h"

/**
 * \brief   初始化系统时钟
 * \retval
 */
void rcc_init(void)
{    
    RCC_DeInit();
    
#ifdef USE_HSE
    RCC_HSEConfig(RCC_HSE_ON);
    while (SUCCESS != RCC_WaitForHSEStartUp()) {
        ;
    }
    
    FLASH_SetLatency(FLASH_Latency_2);

    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);    // PLLCLK = 72M
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                        // HCLK = 72M
    RCC_PCLK1Config(RCC_HCLK_Div2);                         // APB1 = 36M
    RCC_PCLK2Config(RCC_HCLK_Div1);                         // APB2 = 72M

    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == 0) {
        ;
    }
    
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);              // SYSCLK = 72M
    while(RCC_GetSYSCLKSource() != 0x08) {
        ;
    }
    
#else
    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == 0) {
        ;
    }
    // 使能 FLASH 预存取缓冲区
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
 
    // SYSCLK 周期与闪存访问时间的比例设置，这里统一设置成 2
    // 设置成 2 的时候，SYSCLK 低于 48M 也可以工作，如果设置成 0 或者 1 的时候，
    // 如果配置的 SYSCLK 超出了范围的话，则会进入硬件错误，程序就死了
    // 0：0 < SYSCLK <= 24M
    // 1：24< SYSCLK <= 48M
    // 2：48< SYSCLK <= 72M
    FLASH_SetLatency(FLASH_Latency_2);

    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);        // 8M / 2 * 9 = 64M
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                            // HCLK = 64M
    RCC_PCLK1Config(RCC_HCLK_Div2);                             // PCLK = APB1 = 32M
    RCC_PCLK2Config(RCC_HCLK_Div1);                             // PCLK = APB2 = 64M

    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == 0) {
        ;
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);                  // SYSCLK = 48M
    while(RCC_GetSYSCLKSource() != 0x08) {
        ;
    }
#endif
    
    // 更新系统时钟变量，会被第三方用户程序调用
    SystemCoreClockUpdate();
}

/**
 * \brief 1ms systick interrupt.
 * \param 
 * \return
 */
void systick_init_1ms(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    RCC_GetClocksFreq(&RCC_Clocks);
    if(SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000)) {
        while(1);
    }
}

/*************************** End of file ****************************/
