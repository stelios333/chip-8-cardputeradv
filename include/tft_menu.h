#pragma once
#include <vector>
#include "lgfx_conf.h"
#include <string>
#include "fonts.h"
class TFTMenu
{
private:
    LGFX& m_tft;
    const std::vector<std::string>& m_items;
    
    const char* m_title = "Menu";
    const GFXfont* m_title_font = &FreeMonoBold9pt7b;
    uint8_t m_title_font_height = 12;
    const GFXfont* m_items_font = &FreeMono9pt7b;
    uint8_t m_items_font_height = 16;
    uint8_t m_title_height = 18;
    int m_scroll_offset = 0;

public:
    TFTMenu(LGFX& tft, const std::vector<std::string>& items);
    void draw();
    void draw_title();
    void set_title(const char* new_title);
    ~TFTMenu() = default;
    int m_selected = 0;
};