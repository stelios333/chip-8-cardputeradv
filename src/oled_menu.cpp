#include "oled_menu.h"
#include <U8g2lib.h>
#include <string>

OledMenu::OledMenu(U8G2& u8g2, const std::vector<std::string>& items) : m_u8g2(u8g2), m_items(items)
{

}

void OledMenu::draw_title() {
    m_u8g2.setFontPosTop();
    m_u8g2.setFont(m_title_font);
    m_u8g2.setDrawColor(1);
    m_u8g2.drawBox(0,0,m_u8g2.getWidth(), m_title_height);
    m_u8g2.setCursor(1,1);
    m_u8g2.setDrawColor(0);
    m_u8g2.print(m_title);
    m_u8g2.sendBuffer();
}

void OledMenu::set_title(const char* new_title) {
    m_title = new_title;
}


void OledMenu::draw()
{    
    m_u8g2.setFont(m_items_font);
    int y_offset = 16;
    const int max_lines = (m_u8g2.getHeight()-y_offset)/m_items_font_height;
    int max_chars = m_u8g2.getWidth()/m_u8g2.getStrWidth(" ");
    m_u8g2.setDrawColor(0);
    m_u8g2.drawBox(0, y_offset, m_u8g2.getWidth(), m_u8g2.getHeight()-y_offset);
    if (m_selected >= max_lines+m_scroll_offset) {
        m_scroll_offset = m_selected-(max_lines-1);
    }
    if (m_selected < m_scroll_offset) {
        m_scroll_offset = m_selected;
    }
    for(int i=0; i < m_items.size() && i < max_lines; i++){
        int item_index = i+m_scroll_offset;
        m_u8g2.setDrawColor(1);
        m_u8g2.setCursor(1,y_offset);

        if (m_items[item_index].size()>max_chars) {
            Serial.println("String too large!");
        }
        if (item_index == m_selected) {
            m_u8g2.drawBox(0,y_offset,m_u8g2.getWidth(), m_items_font_height);
            m_u8g2.setDrawColor(0);
            m_u8g2.print(m_items[item_index].c_str());
        } else {
            m_u8g2.print(m_items[item_index].c_str());
        }
        y_offset += m_items_font_height;
        
    } 
    m_u8g2.sendBuffer();
}
