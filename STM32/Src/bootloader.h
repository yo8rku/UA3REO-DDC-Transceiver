#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "stm32h7xx.h"
#include "stm32h7xx_hal_conf.h"
#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_gpio.h"

//Public methods
extern void JumpToBootloader(void); //перехов в DFU-режим булодера

#endif
