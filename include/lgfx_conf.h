#pragma once
#include <LovyanGFX.hpp>
class LGFX : public lgfx::LGFX_Device
{
private:
lgfx::Panel_ST7789 _panel_instance;
lgfx::Bus_SPI _bus_instance;
//lgfx::Light_PWM     _light_instance;
public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;

      cfg.pin_sclk = 36;
      cfg.pin_mosi = 35;
      cfg.pin_miso = -1;
      cfg.pin_dc = 34;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 37;
      cfg.pin_rst = 33;
      cfg.pin_busy = -1;

      cfg.panel_width = 135;
      cfg.panel_height = 240;
      cfg.offset_y = 40;
      cfg.offset_x = 52;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = true;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }
    /*
    // Couldn't get the backlight working through pwm
    {
      auto cfg = _light_instance.config();
        cfg.pin_bl      = 38;
        cfg.invert      = false;
        
        cfg.freq        = 44100;
        cfg.pwm_channel = 7;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }
    */
    setPanel(&_panel_instance);
    
  }
};