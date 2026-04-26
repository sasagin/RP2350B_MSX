
#include <stdio.h>
#include "pico/stdlib.h"
#include "inttypes.h"

#include "SCC.h"

typedef struct SCC_regs_t {
	uint16_t SCC_freq[5];
    uint16_t SCC_counter[5];
    int16_t SCC_ch_out[5];                       
	uint8_t SCC_volume[5];
    uint8_t SCC_channels_enable[5];
    
} SCC_regs_t;

static SCC_regs_t chip;
static uint8_t inx_sample[5];



// out SCC 
uint16_t get_SCC_Out(){

    int16_t outSCC = 0;
    uint8_t j;
// mixer  

    for (size_t i = 0; i < 4; i++)
    {
        if((chip.SCC_counter[i]++) >= (chip.SCC_freq[i]>>4)){
            chip.SCC_counter[i] = 0;
            inx_sample[i]++;
            j = (i < 4)? i : 3; // 3 и 4 из одной таблицы читаются
            chip.SCC_ch_out[i] = (chip.SCC_channels_enable[i])? (SCC_ram[(0x20*(j)) + (inx_sample[i]&0x1F)] * chip.SCC_volume[i]) : 0;         
        }       
        outSCC += chip.SCC_ch_out[i]; 
    }
        outSCC = outSCC * 2;
        uint16_t out = ((uint16_t)(outSCC) + 0x8000)>>3;
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
            for (size_t i = 0; i < 5; i++)
            {
                chip.SCC_channels_enable[i] = (reg>>i) & 0x01;
            }           
            break;                                                               
        default:
            break;
        }
        
}