#pragma once


// Y0
#define MSXkey_0   			~(1<<0)
#define MSXkey_1   			~(1<<1)
#define MSXkey_2   			~(1<<2)
#define MSXkey_3   			~(1<<3)
#define MSXkey_4   			~(1<<4)
#define MSXkey_5   			~(1<<5)
#define MSXkey_6   			~(1<<6)
#define MSXkey_7   			~(1<<7)
// Y1
#define MSXkey_8   			~(1<<0)
#define MSXkey_9   			~(1<<1)
#define MSXkey_MINUS   		~(1<<2)
#define MSXkey_EQUALS  		~(1<<3)
#define MSXkey_BACKSLASH	~(1<<4)
#define MSXkey_LEFT_BR  	~(1<<5)
#define MSXkey_RIGHT_BR  	~(1<<6)
#define MSXkey_SEMICOLON 	~(1<<7)
// Y2
#define MSXkey_TILDE   		~(1<<0)
#define MSXkey_QUOTE		~(1<<1)
#define MSXkey_PERIOD		~(1<<2)
#define MSXkey_COMMA   		~(1<<3)
#define MSXkey_SLASH		~(1<<4)
#define MSXkey_DK			~(1<<5)
#define MSXkey_A   			~(1<<6)
#define MSXkey_B   			~(1<<7)
// Y3
#define MSXkey_C 	  		~(1<<0)
#define MSXkey_D			~(1<<1)
#define MSXkey_E			~(1<<2)
#define MSXkey_F   			~(1<<3)
#define MSXkey_G			~(1<<4)
#define MSXkey_H			~(1<<5)
#define MSXkey_I   			~(1<<6)
#define MSXkey_J   			~(1<<7)
// Y4
#define MSXkey_K 	  		~(1<<0)
#define MSXkey_L			~(1<<1)
#define MSXkey_M			~(1<<2)
#define MSXkey_N   			~(1<<3)
#define MSXkey_O			~(1<<4)
#define MSXkey_P			~(1<<5)
#define MSXkey_Q   			~(1<<6)
#define MSXkey_R   			~(1<<7)
// Y5
#define MSXkey_S 	  		~(1<<0)
#define MSXkey_T			~(1<<1)
#define MSXkey_U			~(1<<2)
#define MSXkey_V   			~(1<<3)
#define MSXkey_W			~(1<<4)
#define MSXkey_X			~(1<<5)
#define MSXkey_Y   			~(1<<6)
#define MSXkey_Z   			~(1<<7)
// Y6
#define MSXkey_SHIFT  		~(1<<0)
#define MSXkey_CTRL			~(1<<1)
#define MSXkey_GRPH			~(1<<2)
#define MSXkey_CAPS			~(1<<3)
#define MSXkey_CODE			~(1<<4)
#define MSXkey_F1			~(1<<5)
#define MSXkey_F2 			~(1<<6)
#define MSXkey_F3  			~(1<<7)
// Y7
#define MSXkey_F4 	  		~(1<<0)
#define MSXkey_F5			~(1<<1)
#define MSXkey_ESC			~(1<<2)
#define MSXkey_TAB 			~(1<<3)
#define MSXkey_STOP			~(1<<4)
#define MSXkey_BACKSPC		~(1<<5)
#define MSXkey_SEL 			~(1<<6)
#define MSXkey_ENTER		~(1<<7)
// Y8
#define MSXkey_SPACE 		~(1<<0)
#define MSXkey_HOME			~(1<<1)
#define MSXkey_INS			~(1<<2)
#define MSXkey_DEL	 		~(1<<3)
#define MSXkey_LEFT			~(1<<4)
#define MSXkey_UP			~(1<<5)
#define MSXkey_DOWN			~(1<<6)
#define MSXkey_RIGHT		~(1<<7)

void translate_keys_to_MSX(uint8_t *keymap,int i){

    switch (i)
    {
        case 0x04: keymap[2] &= MSXkey_A; break;
        case 0x05: keymap[2] &= MSXkey_B; break;
		
        case 0x06: keymap[3] &= MSXkey_C; break;
        case 0x07: keymap[3] &= MSXkey_D; break;
        case 0x08: keymap[3] &= MSXkey_E; break;
        case 0x09: keymap[3] &= MSXkey_F; break;
        case 0x0A: keymap[3] &= MSXkey_G; break;
        case 0x0B: keymap[3] &= MSXkey_H; break;
        case 0x0C: keymap[3] &= MSXkey_I; break;
        case 0x0D: keymap[3] &= MSXkey_J; break;
		
        case 0x0E: keymap[4] &= MSXkey_K; break;
        case 0x0F: keymap[4] &= MSXkey_L; break;
        case 0x10: keymap[4] &= MSXkey_M; break;
        case 0x11: keymap[4] &= MSXkey_N; break;   
        case 0x12: keymap[4] &= MSXkey_O; break;
		case 0x13: keymap[4] &= MSXkey_P; break;	
		case 0x14: keymap[4] &= MSXkey_Q; break;
		case 0x15: keymap[4] &= MSXkey_R; break;

        case 0x16: keymap[5] &= MSXkey_S; break;
        case 0x17: keymap[5] &= MSXkey_T; break;
        case 0x18: keymap[5] &= MSXkey_U; break;
        case 0x19: keymap[5] &= MSXkey_V; break;   
        case 0x1A: keymap[5] &= MSXkey_W; break;
		case 0x1B: keymap[5] &= MSXkey_X; break;	
		case 0x1C: keymap[5] &= MSXkey_Y; break;
		case 0x1D: keymap[5] &= MSXkey_Z; break;
		
        case 0x1E: keymap[0] &= MSXkey_1; break;
        case 0x1F: keymap[0] &= MSXkey_2; break;
        case 0x20: keymap[0] &= MSXkey_3; break;
        case 0x21: keymap[0] &= MSXkey_4; break;   
        case 0x22: keymap[0] &= MSXkey_5; break;
		case 0x23: keymap[0] &= MSXkey_6; break;	
		case 0x24: keymap[0] &= MSXkey_7; break;
		case 0x25: keymap[1] &= MSXkey_8; break;		

        case 0x26: keymap[1] &= MSXkey_9; break;
        case 0x27: keymap[0] &= MSXkey_0; break;
        case 0x28: keymap[7] &= MSXkey_ENTER; break;
        case 0x29: keymap[7] &= MSXkey_ESC; break;   
        case 0x2A: keymap[7] &= MSXkey_BACKSPC; break;
		case 0x2B: keymap[7] &= MSXkey_TAB; break;	
		case 0x2C: keymap[8] &= MSXkey_SPACE; break;
		case 0x2D: keymap[1] &= MSXkey_MINUS; break;

        case 0x2E: keymap[1] &= MSXkey_EQUALS; break;
        case 0x2F: keymap[1] &= MSXkey_LEFT_BR; break;
        case 0x30: keymap[1] &= MSXkey_RIGHT_BR; break;
        case 0x31: keymap[1] &= MSXkey_BACKSLASH; break;   
//        case 0x32: keymap[0] &= MSXkey_4; break;
		case 0x33: keymap[1] &= MSXkey_SEMICOLON; break;	
		case 0x34: keymap[2] &= MSXkey_QUOTE; break;
//		case 0x35: keymap[0] &= MSXkey_7; break;

        case 0x36: keymap[2] &= MSXkey_PERIOD; break;
        case 0x37: keymap[2] &= MSXkey_COMMA; break;
        case 0x38: keymap[2] &= MSXkey_SLASH; break;
        case 0x39: keymap[6] &= MSXkey_CAPS; break;   
        case 0x3A: keymap[6] &= MSXkey_F1; break;
		case 0x3B: keymap[6] &= MSXkey_F2; break;	
		case 0x3C: keymap[6] &= MSXkey_F3; break;
		case 0x3D: keymap[7] &= MSXkey_F4; break;
		
        case 0x3E: keymap[7] &= MSXkey_F5; break;
//        case 0x3F: keymap[0] &= MSXkey_1; break;
//        case 0x40: keymap[0] &= MSXkey_2; break;
//        case 0x41: keymap[0] &= MSXkey_3; break;   
//        case 0x42: keymap[0] &= MSXkey_4; break;
//		case 0x43: keymap[0] &= MSXkey_5; break;	
//		case 0x44: keymap[0] &= MSXkey_6; break;
//		case 0x45: keymap[0] &= MSXkey_7; break;

        case 0x45: keymap[7] &= MSXkey_SEL; break;
		case 0x48: keymap[7] &= MSXkey_STOP; break;
		case 0x49: keymap[8] &= MSXkey_INS; break;
		case 0x4A: keymap[8] &= MSXkey_HOME; break;
		case 0x4C: keymap[8] &= MSXkey_DEL; break;		
		
		case 0x4F: keymap[8] &= MSXkey_RIGHT; break;
		case 0x50: keymap[8] &= MSXkey_LEFT; break;
		case 0x51: keymap[8] &= MSXkey_DOWN; break;
		case 0x52: keymap[8] &= MSXkey_UP; break;		
  
        default:break;
    }
}
