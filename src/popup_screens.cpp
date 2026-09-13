#include "popup_screens.h"

void drawRectWithBorder(U8G2& u8g2, int x, int y, int w, int h) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(x,y,w,h);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x,y,w,h);
}

void showPausedScreen(U8G2& u8g2) {
    const int center_x = u8g2.getWidth()/2;
    const int center_y = u8g2.getHeight()/2;
    const int box_w = 96;
    const int box_h = 48;
    const int box_x = center_x - box_w/2;
    drawRectWithBorder(u8g2, box_x, center_y-box_h/2,box_w,box_h);
    u8g2.setFont(u8g2_font_9x15B_tr);
    const char* message = "  Paused";
    const int message_w = u8g2.getStrWidth(message);
    const int message_h = 15;
    u8g2.drawStr(center_x - message_w/2, center_y - message_h-2, message);
    u8g2.setFont(u8g2_font_open_iconic_play_1x_t);
    u8g2.drawGlyph(center_x - message_w/2, center_y - message_h-1, 0x44);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(box_x + 2, center_y + 10, "Press # to quit");
    u8g2.drawStr(box_x + 2, center_y, "Press D to play");
    u8g2.sendBuffer();
}

void showStartingScreen(U8G2& u8g2) {
    const int center_x = u8g2.getWidth()/2;
    const int center_y = u8g2.getHeight()/2;
    const int box_w = 120;
    const int box_h = 24;
    const int box_x = center_x - box_w/2;

    drawRectWithBorder(u8g2, box_x, center_y-box_h/2,box_w,box_h);

    u8g2.setFont(u8g2_font_7x13B_tr);
    const char* message = "Starting game...";
    const int message_w = u8g2.getStrWidth(message);
    const int message_h = 15;
    u8g2.drawStr(center_x - message_w/2, center_y - message_h/2, message);
    u8g2.sendBuffer();
}
