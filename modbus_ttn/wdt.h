/**
 * Module for watchdog timer implemented from reading the Cortex M0
 * manual. The watchdog timer has basic enable and disable. No early
 * warning interrupt.
 * 
 * Comment Updated 8/10/2018
*/

uint8_t * WDT_MEM_LOC = reinterpret_cast<uint8_t*>(0x40001000);
uint8_t * GCLK_MEM_LOC = reinterpret_cast<uint8_t*>(0x40000C00);

uint8_t * WDT_CTRL;
uint16_t * GCLK_CTRL;
uint8_t * WDT_CLR;
uint8_t * WDT_CONFIG;


/**
 * @brief
 * Sets the control and config register for WDT. 
 * Sets the generic clock controller for WDT.
*/
void wdt_init(){
    WDT_CTRL = WDT_MEM_LOC;
    GCLK_CTRL = reinterpret_cast<uint16_t*>(GCLK_MEM_LOC)+2;
    WDT_CONFIG = WDT_MEM_LOC+1;
    WDT_CLR = WDT_MEM_LOC+8;
    *GCLK_CTRL = 0x4000|0x03; //sets the generic clock controller 
    *WDT_CONFIG |= 0xB; //sets countdown to 16384 clock cycles
    //This is the longest amount of time
}

/**
 * @brief
 * Enables watchdog timer to start counting. Will reset MC when 
 * countdown ends
*/
void wdt_enable(){
    *WDT_CTRL = 0x2;
}

/**
 * @brief
 * Disables the watchdog timer
*/
void wdt_disable(){
    *WDT_CTRL = 0x0;
}

/**
 * @brief
 * "snoozes" the countdown. Reset timer and the timer will countdown
 * again.
*/
void wdt_clear(){
    *WDT_CLR = 0xA5;
}