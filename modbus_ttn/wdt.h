
uint8_t * WDT_MEM_LOC = reinterpret_cast<uint8_t*>(0x40001000);
uint8_t * GCLK_MEM_LOC = reinterpret_cast<uint8_t*>(0x40000C00);

uint8_t * WDT_CTRL;
uint16_t * GCLK_CTRL;
uint8_t * WDT_CLR;
uint8_t * WDT_CONFIG;

void wdt_init(){
    WDT_CTRL = WDT_MEM_LOC;
    GCLK_CTRL = reinterpret_cast<uint16_t*>(GCLK_MEM_LOC)+2;
    WDT_CONFIG = WDT_MEM_LOC+1;
    WDT_CLR = WDT_MEM_LOC+8;

    *GCLK_CTRL = 0x4000|0x03;
    *WDT_CONFIG |= 0xB;
    
}

void wdt_enable(){
    *WDT_CTRL = 0x2;
}

void wdt_disable(){
    *WDT_CTRL = 0x0;
}

void wdt_clear(){
    *WDT_CLR = 0xA5;
}