#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
// #include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/dma.h"

#include "hardware/vreg.h"
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip.h>

#include "ff.h"
#include "f_util.h"

#include "drivers/SSG_AY/aySoundSoft.h"
#include "drivers/SCC/SCC.h"

#include "tusb_config.h"
#include "tusb.h"

#include "MSX_keycode.h"

#include "Z80pinout.h"
#include "VDP_int_Z80.pio.h"


// #define UART_ID uart0
// #define BAUD_RATE 115200
// #define UART_TX_PIN 32
// #define UART_RX_PIN 33

#define QNT_KEYS    128

// NES-compatible gamepad (shift register) pins
#define PIN_NES_JOYPAD_CLOCK 32
#define PIN_NES_JOYPAD_LATCH 33
#define PIN_NES_JOYPAD_DATA1  34
#define PIN_NES_JOYPAD_DATA2  35
#define NES_JOYPAD_SHIFT_COUNT 8


//RESET
#define RESET_PIN      44
#define RESET_MASK (1ULL << RESET_PIN)

// /VDP_RD 
#define VDP_RD      36
#define VDP_RD_MASK (1ULL << VDP_RD)

// /VDP_WR 
#define VDP_WR      37
#define VDP_WR_MASK (1ULL << VDP_WR)

// /AUD_W
#define AUD_W       39
#define AUD_W_MASK (1ULL << AUD_W)

// TAPE_IN
#define TAPE_IN     45
#define TAPE_OUT    46


#define VDP_INT_PIO     pio0
#define VDP_INT_SM      0


#define PAGE    (0xFC)
#define PPI     (0xA8)
#define SSG     (0xA0)
#define VDP     (0x98)
// #define IO_SEL  (0b11000000)

#define CS0     0
#define CS1     1
#define CS2     2
#define CS3     3

#define SLOT0   0
#define SLOT1   1
#define SLOT2   2
#define SLOT3   3

#define SEC_SLOT0   0
#define SEC_SLOT1   1
#define SEC_SLOT2   2
#define SEC_SLOT3   3

 // Secondary Slot Register
#define SSR     0xFFFF

uint8_t __aligned(4) bios_rom[0x8000];
uint8_t __aligned(4) cart_rom[16][0x2000];
uint8_t __aligned(4) page_ram[0x10000];

// звук
uint8_t SCC_ram[0x80] = {0};

// uint8_t sin255[] = {0, 53, 104, 150, 189, 242, 253, 255 };


bool SCC_on = 0;
// bool sound_start = 0;
// uint16_t sound_out;
// max value 0x1B00
uint16_t sample_to_send = 0; 

// Memory Mapper Register PAGE0 = FCh,  PAGE1 = FD, PAGE2 = FE, PAGE3 = FF
// uint8_t page0 = 3;                      
// uint8_t page1 = 2;
// uint8_t page2 = 1;
// uint8_t page3 = 0;
// Primary Slot Register
uint8_t pri_slot_reg[4] = {0,0,0,0};
// Secondary Slot Register
uint8_t sec_slot_reg = 0;
//cartrige bank
uint8_t cart_page0 = 0;
uint8_t cart_page1 = 1;
uint8_t cart_page2 = 2;
uint8_t cart_page3 = 3;
uint8_t cart_page4 = 4;

//TAPE_IN

// PPI
uint8_t portA_8255 = 0;                         // port 0xA8        primary slot register
uint8_t portB_8255 = 0;                         // port 0xA9        input from polling keyboard
uint8_t portC_8255 = 0;                         // port 0xAA        output polling keyboard and other needs
uint8_t mode_8255;                              // port 0xAB        mode 8255

// keyboard
uint8_t keymapMSX[11] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
// Массив для хранения кодов нажатых клавиш (HID позволяет до 6 одновременно)
uint8_t pressed_keys[6] = {0};

// joysticks
static uint8_t msx_joysticks [2] = {0,0};
volatile uint8_t data_joysticks[2];
volatile bool start_read_joypad = true;
volatile bool start_read_keyboard = true;

void inInit(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

void outInit(uint gpio,bool set_up_out) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, set_up_out);
}

// Read NES-compatible gamepad via latch/clocked serial interface.
// Returns an active-high button mask after converting from active-low bits.
uint16_t read_joystick_data() {
    uint8_t serial_data1 = 0xFF;
    uint8_t serial_data2 = 0XFF;
    uint16_t out_joypads;
    // Latch current button states into the controller's shift register
    gpio_put(PIN_NES_JOYPAD_LATCH, 1);
    busy_wait_us(20);
    gpio_put(PIN_NES_JOYPAD_LATCH, 0);
    busy_wait_us(20);

    // Shift in 8 bits (standard NES controller protocol)
    for (int i = 0; i < NES_JOYPAD_SHIFT_COUNT; i++) {
        gpio_put(PIN_NES_JOYPAD_CLOCK, 0);
        busy_wait_us(20);
        serial_data1 <<= 1;
        serial_data1 |= gpio_get(PIN_NES_JOYPAD_DATA1);
        serial_data2 <<= 1;
        serial_data2 |= gpio_get(PIN_NES_JOYPAD_DATA2);        
        gpio_put(PIN_NES_JOYPAD_CLOCK, 1);
        busy_wait_us(20);
    }
    out_joypads = (serial_data2<<8) | serial_data1;
    return out_joypads ;
};

//convert NES to COLECO joypad
#define     NES_UP              (1 << 3)
#define     NES_DOWN            (1 << 2)
#define     NES_LEFT            (1 << 1)
#define     NES_RIGHT           (1 << 0)
#define     NES_A               (1 << 7)
#define     NES_B               (1 << 6)
#define     NES_SELECT          (1 << 5)
#define     NES_START           (1 << 4)

#define     UP_BUTTON1        ~(1 << 0)
#define     RIGHT_BUTTON1     ~(1 << 3)
#define     DOWN_BUTTON1      ~(1 << 1)
#define     LEFT_BUTTON1      ~(1 << 2)
#define     SHOTL_BUTTON1     ~(1 << 4)
#define     SHOTR_BUTTON1     ~(1 << 5)

#define     UP_BUTTON2        ~(1 << 0)
#define     RIGHT_BUTTON2     ~(1 << 3)
#define     DOWN_BUTTON2      ~(1 << 1)
#define     LEFT_BUTTON2      ~(1 << 2)
#define     SHOTL_BUTTON2     ~(1 << 4)
#define     SHOTR_BUTTON2     ~(1 << 5)

void convert_nes_to_MSX_joypad(uint16_t nes_joy_data){

    // Convert active-low layout from controller: pressed -> clear bit.
    uint8_t nes_joy_data1 = (uint8_t)(nes_joy_data & 0x00ff);
    uint8_t nes_joy_data2 = (uint8_t)((nes_joy_data & 0xff00) >> 8);
    msx_joysticks[0] = 0xFF;
    msx_joysticks[1] = 0XFF;
// first joypad
    if ((nes_joy_data1 & NES_UP) == 0) {msx_joysticks[0] &= UP_BUTTON1;}
    if ((nes_joy_data1 & NES_DOWN) == 0) {msx_joysticks[0] &= DOWN_BUTTON1;}
    if ((nes_joy_data1 & NES_LEFT) == 0) {msx_joysticks[0] &= LEFT_BUTTON1;}
    if ((nes_joy_data1 & NES_RIGHT) == 0) {msx_joysticks[0] &= RIGHT_BUTTON1;}
    if ((nes_joy_data1 & NES_B) == 0) {msx_joysticks[0] &= SHOTL_BUTTON1;}
    if ((nes_joy_data1 & NES_A) == 0) msx_joysticks[0] &= SHOTR_BUTTON1;
 
// second joypad
    if ((nes_joy_data2 & NES_UP) == 0) {msx_joysticks[1] &= UP_BUTTON2;}
    if ((nes_joy_data2 & NES_DOWN) == 0) {msx_joysticks[1] &= DOWN_BUTTON2;}
    if ((nes_joy_data2 & NES_LEFT) == 0) {msx_joysticks[1] &= LEFT_BUTTON2;}
    if ((nes_joy_data2 & NES_RIGHT) == 0) {msx_joysticks[1] &= RIGHT_BUTTON2;}
    if ((nes_joy_data2 & NES_B) == 0) {msx_joysticks[1] &= SHOTL_BUTTON2;}
    if ((nes_joy_data2 & NES_A) == 0) msx_joysticks[1] &= SHOTR_BUTTON2;
}
// setting pin direction
void Z80_pin_setup() {
    inInit(MREQ_PIN);
    inInit(WR_PIN);
    inInit(RD_PIN);
    inInit(IORQ_PIN);
    // inInit(M1_PIN);
    
    inInit(PIN_NES_JOYPAD_DATA1);
    inInit(PIN_NES_JOYPAD_DATA2); 
    
    inInit(TAPE_IN);    
    outInit(TAPE_OUT,0);
    // inInit(ENCODER_IN_A);
    // inInit(ENCODER_IN_B);

    outInit(RESET_PIN,0);
    // outInit(WAIT_PIN, 1);           //для MSX отключить у него свой формирователь

    outInit(NMI_PIN, 1);
    // outInit(INT_PIN, 1);
    outInit(VDP_RD,1);
    outInit(VDP_WR,1);
    // outInit(AUD_W,1);

    outInit(PIN_NES_JOYPAD_CLOCK,1);
    outInit(PIN_NES_JOYPAD_LATCH,1);
}

    PIO piopio = VDP_INT_PIO;
    uint smsm  = VDP_INT_SM;
void VDP_INT_init() {
    // PIO piopio = VDP_INT_PIO;
    // uint smsm  = VDP_INT_SM;
    pio_set_gpio_base(piopio, 16);
    const uint pio_offset = pio_add_program(piopio, &vdpintz80_program);
    vdpintz80_program_init(piopio, smsm, pio_offset);
    pio_sm_set_enabled(piopio, smsm, true);
}

// PPI write read
__always_inline static inline void write8255(const uint8_t reg, const uint8_t value)
        {
                switch (reg & 3)
                { 
                    case 0:                
                        portA_8255 = value;                      
                        pri_slot_reg[0] = (value & 3);
                        pri_slot_reg[1] = ((value >> 2) & 3);
                        pri_slot_reg[2] = ((value >> 4) & 3);
                        pri_slot_reg[3] = ((value >> 6) & 3);
                        break;
                    case 2:   
                        portC_8255 = value;
                        break;
                    case 3:              
                        if (value & 0x80) {
                            mode_8255 = value; // Установка режима (например, 0x82)
                        } else {
        // Режим Bit Set/Reset для Port C
                        uint8_t bit_to_set = (value >> 1) & 0x07; // какой бит менять (0-7)
                        uint8_t set_high = value & 0x01;          // 1 = set, 0 = reset 
                            if(bit_to_set == 5){gpio_put(TAPE_OUT,set_high);}      
                            if (set_high) {
                                portC_8255 |= (1 << bit_to_set);
                            } else {
                                portC_8255 &= ~(1 << bit_to_set);
                            }
// start_print_regs = true;
                        }                     
                        break;   
                    default:
                        break;
                }        
    }

__always_inline static inline uint8_t read8255(const uint8_t reg)
        {
            switch (reg & 3)
            {
            case 0:
                return portA_8255; 
                break;
            case 1:
                // start_read_keyboard = true;
                return keymapMSX[portC_8255 & 0x0f]; 
                break;
            case 2:
                return portC_8255; 
                break;
            case 3:
                return mode_8255; 
                break;                               
            default:
                return 0xff;
                break;
            }
    
    }

// Z80 write-side address decode: ioport,.
__always_inline static inline void write_MSX_io(const uint8_t portIO, const uint8_t value) {

    switch (portIO & 0xFC)
        {
        case PPI: write8255(portIO, value);               
            break;
// write to PSG                                 
        case SSG:                                     
            // gpio_put(AUD_W,0);                          // CS PSG
            if(portIO == 0xA0){
            AY_select_reg(value);
            }          
            if(portIO == 0xA1){
            AY_set_reg(value);
            }            
            break;                            
        case VDP:  gpio_put(VDP_WR,0);                          // CS VDP_WR
            break;              
        default:
        break;
        }            
}

// Z80 write-side address decode: ioport,.
__always_inline static inline void read_MSX_io(const uint8_t portIO) {
   
    switch (portIO & 0xFC)
        {                 
        case VDP:                
            gpio_put(VDP_RD,0);
            break;          
        case SSG:
            {
                uint8_t temp = 0xFF;                
                if(portIO == 0xA2){
// tape in
                    tape_in_load (gpio_get(TAPE_IN)? 0x80 : 0x00);                                      
                    temp = AY_get_reg();
                    }    
                const uint32_t data = ((uint32_t)temp) << 16 ; 
                gpio_put_masked(DATA_MASK,data);           
                gpio_set_dir_out_masked(DATA_MASK);                
            }        
            break;                     
        case PPI:                
            {
                uint8_t temp = 0xFF;
                temp = read8255(portIO);
                const uint32_t data = ((uint32_t)temp) << 16 ; 
                gpio_put_masked(DATA_MASK,data);           
                gpio_set_dir_out_masked(DATA_MASK);                
            }
            break;                                        
        default:
            {
                uint8_t temp = 0xFF;                
                const uint32_t data = ((uint32_t)temp) << 16 ; 
                gpio_put_masked(DATA_MASK,data);           
                gpio_set_dir_out_masked(DATA_MASK);                
            }
            break;
        }            
}

// Z80 read-side address decode: RAM, ROM banks,and BIOS.
__always_inline static inline uint8_t read_MSX_memory(const uint16_t address) {

    uint8_t ad = (uint8_t)(address >> 14);
    
        switch (ad)
            {
                case CS0:                                               //page_0 0x0000 - 0x3FFF
                    {
                        switch (pri_slot_reg[0])
                            {
                            case SLOT0:
                                return bios_rom[address];
                                break;
                                                                
                            case SLOT3:
                                return page_ram[address];
                                break;                                                    
                            default:
                                return 0xff;
                                break;
                            }
                    }
                    break;                       
                case CS1:                                               //page_1 0x4000 - 0x7FFF                                             
                    {
                        switch (pri_slot_reg[1])
                            {
                            case SLOT0:
                                return bios_rom[address];
                                break;
                            case SLOT2:
                                if((address & 0x2000)==0){return cart_rom[cart_page0][address & 0x1fff];}else{
                                    return cart_rom[cart_page1][address & 0x1fff];}
                                break;                                
                            case SLOT3:
                                return page_ram[address];
                                break;                                                    
                            default:
                                return 0xff;
                                break;
                            }
                    }
                    break;
                case CS2:
                    {
                        switch (pri_slot_reg[2])
                            {
                            case SLOT2:
                                if((address & 0x2000)==0){return cart_rom[cart_page2][address & 0x1fff];}else{
                                    return cart_rom[cart_page3][address & 0x1fff];}
                                if(SCC_on && ((address & 0xFF80) == 0x9800)){ return SCC_ram[address & 0x07F];}        
                                break;                                        
                            case SLOT3:
                                return page_ram[address];
                                break;                                                                                                       
                            default:
                                return 0xFF;
                                break;                                                   
                            }   
                    }
                case CS3:                                               //page_3 0xC000 - 0xFFFF                       
                    {                       
                        switch (pri_slot_reg[3])
                            { 
                                                                       
                            case SLOT3:
                                return page_ram[address];
                                break;                                                                                                       
                            default:
                                return 0xFF;
                                break;                                                   
                            }   
                    }
                    break;      
            // Default open-bus value
                default:
                return 0xFF;
                break;
            }

    return 0xFF;
    
}

__always_inline static inline void write_MSX_memory(const uint16_t address,const uint8_t value){

        uint8_t ad = (uint8_t)(address >> 14);

        switch (ad)
        {
            case CS0:                                                   //page_0 0x0000 - 0x3FFF          
                {
                 switch (pri_slot_reg[0])                      
                        {                                                   
                        case SLOT3:
                            page_ram[address] = value;                           
                            break;                                      
                        default:
                            break;
                        }  
                }
                break;            
            case CS1:                                                   //page_1 0x4000 - 0x7FFF          
                {
                 switch (pri_slot_reg[1])                      
                        {
                        case SLOT2:
                            // reg_2 = address;
                            // reg_1 = value;
                            // start_print_regs = true;
                            if(address == 0x7000) {cart_page1 = value&0x0F;}
//nemesis  ok!                      
                                if(address == 0x68FF){
                                    cart_page1 = value;
                                }
                                if(address == 0x70FF){
                                    cart_page2 = value;
                                }
                                if(address == 0x78ff){
                                    cart_page3 = value;
                                }
                            break;
                        case SLOT3:
                            page_ram[address] = value;                           
                            break;                                      
                        default:
                            break;
                        }  
                }
                break;                
            case CS2:                                                   //page_2 0x8000 - 0xBFFF          
                {
                 switch (pri_slot_reg[2])                      
                        {
                        case SLOT2:                      
                            if(address == 0x9000) {
                                if(value >= 0x20){SCC_on = 1;}else{cart_page2 = value & 0x0F;}                               
                            }
                            if(address == 0xB000) {cart_page3 = value&0x0F;}
                            
                            if((address & 0xFF80) == 0x9800){
                                 SCC_ram[address & 0x07F] = value;
                                }else if((address & 0xFF80) == 0x9880){
                                    set_SCC_reg(address , value);
// start_print_regs = true;                                     
                                }
                        case SLOT3:
                            page_ram[address] = value;                           
                            break;                                      
                        default:
                            break;
                        }  
                }
                break;                    
            case CS3:                                                   //page_3 0xC000 - 0xFFFF          
                {
                 switch (pri_slot_reg[3])                      
                        {                                                   
                        case SLOT3:
                            page_ram[address] = value;                           
                            break;                                      
                        default:
                            break;
                        }  
                }
                break;
            default:
            break;
        }    
}

void __time_critical_func(Z80_loop)() {
    while(true){
            gpio_set_dir_in_masked(DATA_MASK);

        //------------memory ok-----------------

        while(!(gpio_get_all64() & MREQ_MASK)) {                                    // memory read write
                uint16_t address = (uint16_t)((gpio_get_all()) & 0x0000ffff);  
                while(!(gpio_get_all() & READ_MASK)) {                                  //  read from         
                    const uint32_t data = read_MSX_memory(address) << 16;
                    gpio_put_masked(DATA_MASK,data);
                    gpio_set_dir_out_masked(DATA_MASK);
                }

            gpio_set_dir_in_masked(DATA_MASK); 
                            
                while(!(gpio_get_all() & WRITE_MASK)) {                                 // write to ram                            
                        // : MSX RAM
                    uint8_t data_to_ram = (uint8_t)((gpio_get_all() >> 16) & 0x000000FF);
                    write_MSX_memory(address,data_to_ram);                 
                }         
        }
    
    // io port
        while(!(gpio_get_all64() & IORQ_MASK)) {
            uint16_t address = (gpio_get_all()) ;              // mask io port address
            uint8_t portIO = (uint8_t)(0x00FF & address);

    // read from IO             
            while(!(gpio_get_all() & READ_MASK)) {                              // read from IO
                read_MSX_io(portIO);                                
                }
    // write to IO        
            while(!(gpio_get_all64() & WRITE_MASK)) {                              // write to IO

                uint8_t datawritetoio = (uint8_t)((gpio_get_all() >> 16)&0x000000ff);
                write_MSX_io(portIO,datawritetoio);
                }
            gpio_set_dir_in_masked(DATA_MASK);

        }
        gpio_put(VDP_WR,1);
        gpio_put(VDP_RD,1);
        // gpio_put(AUD_W,1);
    }
}

void print_key_state() {
    for (int i = 0; i < 6; i++) {
        if (pressed_keys[i]) translate_keys_to_MSX(keymapMSX,pressed_keys[i]); //printf("%02X ", pressed_keys[i]); 
    }
}


// Обработчик прерывания таймера
bool alarm_callback(struct repeating_timer *t) {
        uint16_t SCC_out = 4100;
        if(SCC_on){SCC_out = get_SCC_Out();}
        sample_to_send = SCC_out + (get_AY_Out(1)<<3) - 4000;
    return true; // Продолжаем повторение
}

// Обработчик таймера 8ms
bool my_timer_callback(struct repeating_timer *t) {
                start_read_keyboard = true;
                start_read_joypad = true; 
    return true; // продолжаем повторение
}



// main------------------------------------------

    int main() {


    vreg_disable_voltage_limit();
    vreg_set_voltage(VREG_VOLTAGE_1_60);

    qmi_hw->m[0].timing = 0x60007304; // 4x FLASH divisor

    sleep_ms(100);
    // if (!set_sys_clock_hz(CPU_FREQ_MHZ * MHZ, 0) ) {
        set_sys_clock_hz(315 * MHZ, 1); // fallback to failsafe clocks
    // }

        // Инициализируем UART    
    // uart_init(UART_ID, BAUD_RATE);

    // // Настраиваем GPIO пины для UART
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
           
    stdio_init_all();

    // Инициализация USB стека
    tusb_init(); 

    // Initialize pins
    gpio_init_mask(ADDR_MASK | DATA_MASK);
    gpio_set_dir_in_masked(ADDR_MASK | DATA_MASK);
    Z80_pin_setup();

    // Initialize select out interrupt from VDP to Z80

    VDP_INT_init();

    // init PSG 

    // Создаём повторяющийся таймер
    struct repeating_timer timer;
	
	struct repeating_timer timer_8ms;
	
	
    // Период в микросекундах: 1 000 000 / 58 000 ≈ 17,24 мкс
    int64_t period_us = 1000000LL / 110000;
    // Запускаем таймер с заданным периодом
    if (!add_repeating_timer_us(-period_us, alarm_callback, NULL, &timer)) {
        printf("Failed to start timer\n");
        return -1;
    }
	
	    // Запускаем таймер: интервал 8 мс, немедленный старт
    if (!add_repeating_timer_ms(-8, my_timer_callback, NULL, &timer_8ms)) {
        printf("Failed to start timer\n");
        return -1;
    }


    const uint PWM_PIN = AUD_W;
    const float SYS_CLK = 315000000.0f;
    // const float PWM_FREQ = 44100.0f;
    const float PWM_FREQ = 88200.0f;
    // const uint SAMPLE_FREQ = 33000;

    // 1. Настройка PWM (44 кГц)
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    uint chan = pwm_gpio_to_channel(PWM_PIN);
    uint32_t top = (uint32_t)(SYS_CLK / PWM_FREQ) - 1;
    pwm_set_wrap(slice, top);
    pwm_set_enabled(slice, true);

    // Адрес регистра для записи уровня ШИМ (CC - Compare Counter)
    volatile void* pwm_cc_reg = (void*)&pwm_hw->slice[slice].cc;

    // 2. Настройка DMA
    int dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16); // 16 бит
    channel_config_set_read_increment(&c, false);          // Читаем из одного места
    channel_config_set_write_increment(&c, false);         // Пишем в один регистр
    
    // Синхронизация по таймеру (DREQ_TIMER0)
    int timer_dreq = dma_get_timer_dreq(0); 
    channel_config_set_dreq(&c, timer_dreq);

    // Настраиваем таймер на 33 кГц (315МГц / 10000 = 31.5кГц)
    // Либо используем встроенный механизм dma_timer:
    dma_timer_set_fraction(0, 1, 10000); // Настройка делителя таймера 0

    dma_channel_configure(
        dma_chan,
        &c,
        pwm_cc_reg,        // Куда (PWM CC register)
        &sample_to_send,   // Откуда (Ваша переменная)
        0xFFFFFFFF,        // Сколько раз (бесконечно в режиме зацикливания)
        true               // Старт
    ); 

    FATFS fs;
    FIL file_sd;
    UINT br;
    FRESULT res;      
 
        // Mount SD card filesystem
    if (FR_OK != f_mount(&fs, "", 1)) {
        // while (!stdio_usb_connected()) { tight_loop_contents(); }
        printf("SD Card not inserted or SD Card error!");
        // reset_usb_boot(0, 0);
    }
    
        // Load BIOS
    res = f_open(&file_sd, "/MSX/BIOS/main.rom", FA_READ);
    if (res != FR_OK) {
        printf("Cannot open BIOS!\n");
        // reset_usb_boot(0, 0);
    }

    res = f_read(&file_sd, bios_rom, 0x8000, &br);
    f_close(&file_sd);

    if (res != FR_OK || br == 0) {
        printf("BIOS read error!\n");
        // reset_usb_boot(0, 0);
    }
    printf("BIOS loaded: %u bytes\n", br);
   
    // Load program

    res = f_open(&file_sd, "/MSX/CART/game.rom", FA_READ);
    if (res != FR_OK) {
        printf("Cannot open cartridge!\n");
        // return;
    }

    size_t cart_size = 0;
    for (int i = 0; i < 16; i++) {
        res = f_read(&file_sd, cart_rom[i], 0x2000, &br);
        if (res != FR_OK || br == 0) break;
        cart_size += br;
        if (br < 0x2000) break;  // файл кончился
    }
    f_close(&file_sd);

    printf("Cartridge loaded: %u bytes, %u pages\n",
        cart_size, (cart_size + 0x1FFF) / 0x2000);
          
    // sleep_ms(2000);
    printf ("start\n ");

    multicore_launch_core1(Z80_loop);

    gpio_put(RESET_PIN,1);    busy_wait_ms(200);
    gpio_put(RESET_PIN,0);    busy_wait_ms(200);    
    gpio_put(RESET_PIN,1); 
    AY_reset();    

    while (true) {
        if(start_read_joypad){ 
            // busy_wait_us(300);           
            convert_nes_to_MSX_joypad(read_joystick_data());
            set_joy_val(msx_joysticks[0],msx_joysticks[1]);
            start_read_joypad = false;
        } 

        if(start_read_keyboard){
            start_read_keyboard = false;
        // busy_wait_us(500);    
        tuh_task(); // Обслуживание USB стека           
        }

    }//while
}//main

// Вызывается при подключении устройства
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    // Если это клавиатура — запрашиваем отчет
    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        tuh_hid_receive_report(dev_addr, instance);
    }
}

// Вызывается при получении данных от клавиатуры
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    hid_keyboard_report_t const* kbd_report = (hid_keyboard_report_t const*) report;


    uint8_t modifier = kbd_report->modifier; // Байт 0 — модификаторы
    bool left_ctrl = (modifier & 0x01);
    bool left_shift = (modifier & 0x02);
    bool left_alt = (modifier & 0x04);
    bool right_ctrl = (modifier & 0x10);
    bool right_shift = (modifier & 0x20);
    bool right_alt = (modifier & 0x40);

    // Сбрасываем все состояния клавиш 
    for (int i = 0; i < 11; i++) {
        keymapMSX[i] = 0xff;
        }    
    // printf("Modifiers: ");
    if (left_ctrl)  keymapMSX[6] &= MSXkey_CTRL;// printf("L-Ctrl ");
    if (left_shift) keymapMSX[6] &= MSXkey_SHIFT;//printf("L-Shift ");
    if (left_alt) keymapMSX[6] &= MSXkey_GRPH;//printf("L-Alt ");
    if (right_ctrl) keymapMSX[6] &= MSXkey_CTRL;//printf("R-Ctrl ");
    if (right_shift) keymapMSX[6] &= MSXkey_SHIFT;//printf("R-Shift ");
    if (right_alt) keymapMSX[6] &= MSXkey_GRPH;//printf("R-Alt ");
    // printf("\n");

    // Копируем состояние 6 клавиш в наш массив
    for(int i=0; i<6; i++) {
        pressed_keys[i] = kbd_report->keycode[i];
    }

    print_key_state();

    // Запрашиваем следующий отчет
    tuh_hid_receive_report(dev_addr, instance);
}
