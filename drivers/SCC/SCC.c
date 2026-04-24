
#include <stdio.h>
#include "pico/stdlib.h"
#include "stdbool.h"

#include "string.h"
#include "inttypes.h"
#include "SCC.h"

typedef struct SCC_regs_t {
	uint16_t SCC_freq[5];
    uint16_t SCC_counter[5];                       
	uint8_t SCC_volume[5];
    uint8_t SCC_channels_enable;
} SCC_regs_t;

static SCC_regs_t chip;
static uint8_t inx_sample[5];

// out SCC 
uint16_t get_SCC_Out(){
    int16_t outSCC = 0;
// mixer  

    for (size_t i = 0; i < 3; i++)
    {
        if((chip.SCC_counter[i]++) >= chip.SCC_freq[i]){
            chip.SCC_counter[i] = 0;
            // if((inx_sample[i]++) >= 2 ){inx_sample[i] = 0;          
        } 
        inx_sample[i] = (chip.SCC_counter[i]>>5) & 0x001f;

        outSCC += SCC_ram[(0x20*(i)) + inx_sample[i]] * chip.SCC_volume[i] * ((chip.SCC_channels_enable>>i) & 1);
    }
        if((chip.SCC_counter[4]++) >= chip.SCC_freq[4]){
            chip.SCC_counter[4] = 0;
            if((inx_sample[4]++) >= 32 ){inx_sample[4] = 0;}
        } 
        outSCC += SCC_ram[(0x20*(3)) + inx_sample[4]] * chip.SCC_volume[4] * ((chip.SCC_channels_enable>>4) & 1); 

        outSCC = outSCC/2;
        uint16_t out = ((uint16_t)(outSCC + 2000))&0x1B00;
        return out ;
}

// setting SCC register value
void set_SCC_reg(uint16_t address, uint8_t reg ){
        switch (address & 0x007F)
        {
//freq 1
        case 0x00:
            chip.SCC_freq[0] = (chip.SCC_freq[0]&0xF00) + reg;
            break;
        case 0x01:
            chip.SCC_freq[0] = (chip.SCC_freq[0]&0xFF) + (reg<<8);
            break;
//freq 2
        case 0x02:
            chip.SCC_freq[1] = (chip.SCC_freq[1]&0xF00) + reg;
            break;
        case 0x03:
            chip.SCC_freq[1] = (chip.SCC_freq[1]&0xFF) + (reg<<8);
            break;
//freq 3                                    
        case 0x04:
            chip.SCC_freq[2] = (chip.SCC_freq[2]&0xF00) + reg;
            break;
        case 0x05:
            chip.SCC_freq[2] = (chip.SCC_freq[2]&0xFF) + (reg<<8);
            break;
//freq 4
        case 0x06:
            chip.SCC_freq[3] = (chip.SCC_freq[3]&0xF00) + reg;
            break;
        case 0x07:
            chip.SCC_freq[3] = (chip.SCC_freq[3]&0xFF) + (reg<<8);
            break;
//freq 5
        case 0x08:
            chip.SCC_freq[4] = (chip.SCC_freq[4]&0xF00) + reg;
            break;
        case 0x09:
            chip.SCC_freq[4] = (chip.SCC_freq[4]&0xFF) + (reg<<8);
            break;            
//vol 1
        case 0x0A:
            chip.SCC_volume[0] = reg & 0x0F;
            break;
//vol 2
        case 0x0B:
            chip.SCC_volume[1] = reg & 0x0F;
            break;
//vol 3
        case 0x0C:
            chip.SCC_volume[2] = reg & 0x0F;
            break;
//vol 4
        case 0x0D:
            chip.SCC_volume[3] = reg & 0x0F;
            break;
//vol 5
        case 0x0E:
            chip.SCC_volume[4] = reg & 0x0F;
            break;
// channels_enable
        case 0x0F:
            chip.SCC_channels_enable = reg & 0x1F;
            break;                                                               
        default:
            break;
        }
        
}