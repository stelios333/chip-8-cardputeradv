#include "popup_screens.h"

void drawRectWithBorder(LGFX& tft, int x, int y, int w, int h) {
    
    tft.fillRect(x,y,w,h, TFT_BLACK);
    tft.drawRect(x,y,w,h, TFT_WHITE);
    
}

void showPausedScreen(LGFX& tft) {
    
    const int center_x = tft.width()/2;
    const int center_y = tft.height()/2;
    const int box_w = 164;
    const int box_h = 90;
    const int box_x = center_x - box_w/2;
    tft.startWrite();
    drawRectWithBorder(tft, box_x, center_y-box_h/2,box_w,box_h);
    tft.unloadFont();
    tft.setTextColor(TFT_WHITE);
    tft.setFont(&FreeSansBold18pt7b);
    const char* message = "Paused";
    const int message_w = tft.textWidth(message);
    const int message_h = 24;
    tft.drawString(message, center_x - message_w/2, center_y - message_h-8);
    tft.setFont(&FreeSans9pt7b);
    tft.drawString("Press p to continue", box_x + 2, center_y+4);
    tft.drawString("Press esc to quit", box_x + 2, center_y + 20);
    tft.endWrite();
}

void showStartingScreen(LGFX& tft) {
    
    const int center_x = tft.width()/2;
    const int center_y = tft.height()/2;
    const int box_w = 200;
    const int box_h = 48;
    const int box_x = center_x - box_w/2;
    tft.startWrite();
    drawRectWithBorder(tft, box_x, center_y-box_h/2,box_w,box_h);
    tft.unloadFont();
    tft.setTextColor(TFT_WHITE);
    tft.setFont(&FreeSansBold12pt7b);
    const char* message = "Starting game...";
    const int message_w = tft.textWidth(message);
    const int message_h = 16;
    tft.drawString(message, center_x - message_w/2, center_y - message_h/2);
    tft.endWrite();
}
