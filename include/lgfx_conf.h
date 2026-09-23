#pragma once
#include <LovyanGFX.hpp>
class LGFX : public lgfx::LGFX_Device
{
private:
lgfx::Panel_ST7789 _panel_instance;
lgfx::Bus_SPI _bus_instance;
public:
  LGFX(void)
  {
    lgfx::gpio_hi(GPIO_NUM_38);
    lgfx::pinMode(GPIO_NUM_38, lgfx::v1::pin_mode_t::output);
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 20000000;
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
    

    
    setPanel(&_panel_instance);
    
  }
};