#pragma once
#include <vector>
#include <U8g2lib.h>
#include <string>

class OledMenu
{
private:
    U8G2& m_u8g2;
    const std::vector<std::string>& m_items;
    
    const char* m_title = "Menu";
    const uint8_t* m_title_font = u8g2_font_6x13_tr;
    uint8_t m_title_font_height = 13;
    const uint8_t* m_items_font = u8g2_font_6x12_tr;
    uint8_t m_items_font_height = 12;
    uint8_t m_title_height = 16;
    int m_scroll_offset = 0;

public:
    OledMenu(U8G2& u8g2, const std::vector<std::string>& items);
    void draw();
    void draw_title();
    void set_title(const char* new_title);
    ~OledMenu() = default;
    int m_selected = 0;
};