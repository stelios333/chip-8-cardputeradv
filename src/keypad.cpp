#include "keypad.h"

KeypadEvent HandleKeypadInput(Adafruit_TCA8418& keypad, volatile bool& TCA8418_event) {
    int intStat = keypad.readRegister(TCA8418_REG_INT_STAT);

    if (intStat & 0x02)
    {
      //  reading the registers is mandatory to clear IRQ flag
      //  can also be used to find the GPIO changed
      //  as these registers are a bitmap of the gpio pins.
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_1);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_2);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_3);
      //  clear GPIO IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 2);
    }

    uint8_t row = 0xFF, col = 0xFF;
    bool pressed = 0;

    if (intStat & 0x01)
    {
      int event = keypad.getEvent();
      pressed = event & 0x80;
      event &= 0x7F;
      
      mapRawKeyToPhysical(event, row, col);
      

      //  clear the EVENT IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
    }

    //  check pending events
    int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if ((intstat & 0x03) == 0) TCA8418_event = false;
    return {pressed, row, col};
}
