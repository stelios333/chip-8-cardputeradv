#include "tft_menu.h"
#include <TFT_eSPI.h>
#include <string>

TFTMenu::TFTMenu(TFT_eSPI& tft, const std::vector<std::string>& items) : m_tft(tft), m_items(items)
{

}

void TFTMenu::draw_title() {
    m_tft.loadFont(m_title_font);
    m_tft.fillRect(0,0,m_tft.width(), m_title_height, TFT_GREEN);
    m_tft.setCursor(1,1);
    m_tft.setTextColor(TFT_TRANSPARENT);
    //m_tft.setTextSize(2);
    m_tft.print(m_title);
}

void TFTMenu::set_title(const char* new_title) {
    m_title = new_title;
}


void TFTMenu::draw()
{    
    
    m_tft.loadFont(m_items_font);
    m_tft.gFont.spaceWidth = m_tft.textWidth("m");
    int y_offset = m_title_height;
    const int max_lines = (m_tft.height()-y_offset)/m_items_font_height;
    int max_chars = m_tft.width()/m_tft.gFont.spaceWidth;
    m_tft.fillRect(0, y_offset, m_tft.width(), m_tft.height()-y_offset, TFT_BLACK);
    if (m_selected >= max_lines+m_scroll_offset) {
        m_scroll_offset = m_selected-(max_lines-1);
    }
    if (m_selected < m_scroll_offset) {
        m_scroll_offset = m_selected;
    }
    m_tft.startWrite();
    for(int i=0; i < m_items.size() && i < max_lines; i++){
        int item_index = i+m_scroll_offset;
        m_tft.setTextColor(TFT_WHITE);
        m_tft.setCursor(1,y_offset);

        if (m_items[item_index].size()>max_chars) {
            Serial.println("String too large!");
        }
        if (item_index == m_selected) {
            m_tft.fillRect(0,y_offset,m_tft.width(), m_items_font_height, TFT_WHITE);
            m_tft.setTextColor(TFT_TRANSPARENT);
            m_tft.print(m_items[item_index].c_str());
        } else {
            m_tft.print(m_items[item_index].c_str());
        }
        y_offset += m_items_font_height;
        
    } 
    m_tft.endWrite();

}
