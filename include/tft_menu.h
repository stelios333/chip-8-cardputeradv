#pragma once
#include <vector>
#include <TFT_eSPI.h>
#include <string>
#include "fonts.h"
class TFTMenu
{
private:
    TFT_eSPI& m_tft;
    const std::vector<std::string>& m_items;
    
    const char* m_title = "Menu";
    const uint8_t* m_title_font = RobotoMonoBold16;
    uint8_t m_title_font_height = 16;
    const uint8_t* m_items_font = RobotoMono16;
    uint8_t m_items_font_height = 16;
    uint8_t m_title_height = 18;
    int m_scroll_offset = 0;

public:
    TFTMenu(TFT_eSPI& tft, const std::vector<std::string>& items);
    void draw();
    void draw_title();
    void set_title(const char* new_title);
    ~TFTMenu() = default;
    int m_selected = 0;
};