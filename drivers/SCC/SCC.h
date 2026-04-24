#pragma once
#include "inttypes.h"

//SCC sound
extern uint8_t SCC_ram[];

uint16_t get_SCC_Out();
void set_SCC_reg(uint16_t address, uint8_t reg );