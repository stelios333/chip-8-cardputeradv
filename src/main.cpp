#include <Arduino.h>
#include <U8g2lib.h>
#include "chip8.h"
#include "roms.h"
#include "keymap.h"
#include "oled_menu.h"
#include "audio.h"
#include "settings_menu.h"
#include <Wire.h>
#include <Keypad.h>
#include <Preferences.h>
#include <string>

SET_LOOP_TASK_STACK_SIZE(16 * 1024);

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11, 12, 13, 6};   // R1 -> R4
byte colPins[COLS] = {7, 8, 9, 10};   // C1 -> C4

constexpr int SSD1306_SCL = 1;
constexpr int SSD1306_SDA = 2;
constexpr int BUZZER_PIN = 3;
constexpr int BUZZER_FREQ = 440;

const char VERSION_STRING[] = "Version           0.2";

static int selected_game = 0;

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Preferences prefs;
ToneGenerator emulator_audio(BUZZER_PIN, BUZZER_FREQ);

std::string joinOptionAndValue(MenuOption option, int value, int new_str_len=21) {
    std::string tmp(new_str_len, ' ');
    int old_size = tmp.length();
    tmp.replace(0, option.name_len, option.name);
    if(option.max_value == 1 && option.min_value == 0) {
        if (value) {
            tmp.replace(new_str_len-2, 2, "on");
        } else {
            tmp.replace(new_str_len-3, 3, "off");
        }
    } else {
        auto value_str = std::to_string(value);
        tmp.replace(new_str_len-value_str.size(), value_str.size(), value_str);
    }
    
    return tmp;
}

void settingsMenu() {
    std::vector<std::string> settings_vec; 

    settings_vec.reserve(settings_arr.size()+1);
    for (int i = 0; i<settings_arr.size(); ++i) {
        
        int value = prefs.getInt(settings_arr.at(i).name);
        settings_vec.push_back(joinOptionAndValue(settings_arr.at(i), value));
            
    }
    settings_vec.push_back(VERSION_STRING);
    OledMenu settings_menu(u8g2, settings_vec);
    settings_menu.set_title("Settings");
    settings_menu.draw_title();
    settings_menu.draw();
    while (true) {
        char key = keypad.getKey();

        if(key == '5') {
            if (settings_menu.m_selected < settings_vec.size()-1) {++settings_menu.m_selected;}
            else {settings_menu.m_selected = 0;}
            settings_menu.draw();
        } else if (key == '2') {
            if (settings_menu.m_selected > 0) {--settings_menu.m_selected;}
            else {settings_menu.m_selected = settings_vec.size()-1;}
            settings_menu.draw();
        
        } else if (key == '6' || key == '4') {
            if (settings_menu.m_selected >= settings_arr.size()) continue;

            MenuOption selected_option = settings_arr.at(settings_menu.m_selected);
            
            int prev_val = prefs.getInt(selected_option.name, 0);
            int new_val;
            if (key == '6')
                new_val = { prev_val < selected_option.max_value ? prev_val + 1 : 0 };
            else
                new_val = { prev_val > selected_option.min_value ? prev_val - 1 : selected_option.max_value};

            prefs.putInt(selected_option.name, new_val);

            settings_vec[settings_menu.m_selected] = joinOptionAndValue(selected_option, new_val);

            settings_menu.draw();
        }
        else if (key == '*') {
            break;
        }
    }

}

int gameSelectionMenu(int default_game = 0) {
    OledMenu oled_menu(u8g2, ROM_NAMES);

    oled_menu.m_selected = default_game;
    oled_menu.set_title("Select a game:");
    oled_menu.draw_title();
    oled_menu.draw();
    
    while (true) {
        char key = keypad.getKey();

        if(key == '5') {
            if (oled_menu.m_selected < ROM_NAMES.size()-1) {++oled_menu.m_selected;}
            else {oled_menu.m_selected = 0;}
            oled_menu.draw();
        } else if (key == '2') {
            if (oled_menu.m_selected > 0) {--oled_menu.m_selected;}
            else {oled_menu.m_selected = ROM_NAMES.size()-1;}
            oled_menu.draw();
        
        } else if (key == '*') {
            settingsMenu();
            oled_menu.draw_title();
            oled_menu.draw();
        }
        else if (key == '6') {
            break;
        }
    }

    return oled_menu.m_selected;
}


void showPausedScreen() {
    const int center_x = u8g2.getWidth()/2;
    const int center_y = u8g2.getHeight()/2;
    const int box_w = 64;
    const int box_h = 32;
    u8g2.setDrawColor(0);
    u8g2.drawBox(center_x - box_w/2,center_y-box_h/2,box_w,box_h);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(center_x - box_w/2,center_y-box_h/2,box_w,box_h);
    u8g2.setFont(u8g2_font_9x15B_tr);
    const char* message = "Paused";
    const int message_w = u8g2.getStrWidth(message);
    const int message_h = 15;
    u8g2.drawStr(center_x - message_w/2, center_y - message_h/2, message);
    u8g2.sendBuffer();
}

int startEmulator(const uint8_t* rom_data, const ulong rom_size)
{
    Chip8 chip8;
    if (!chip8.load_rom(rom_data, rom_size)) 
    {
        Serial.println("ROM could not be loaded. Possibly invalid path given\n");
        return 1;
    }
    chip8.seed_prng();
    
    bool trace_mode = false, audio_on = !prefs.getInt("Mute"), debug_mode = false;
    bool i_quirk = prefs.getInt("Memory quirk"), s_quirk = prefs.getInt("Shift quirk"), fast_forward = prefs.getInt("Turbo");

    const char* window_title = "Chip-8 Emulator";

    uint16_t iter_count = 0;

    uint8_t* display_buf = u8g2.getBufferPtr();
    const int display_buf_size = u8g2_GetBufferSize(u8g2.getU8g2());

    ulong start = micros();
    while (true)
    {

        if(chip8.single_cycle(trace_mode, audio_on, emulator_audio, s_quirk, i_quirk)) {
            Serial.print("Failed to execute last instruction. Exiting...\n");
            return 1;
        }
        if (keypad.getKeys()) {
    
            for (int i = 0; i < LIST_MAX; i++) { 
            
                if (keypad.key[i].stateChanged) {
                    
                    char key = keypad.key[i].kchar;
                    
                    switch (keypad.key[i].kstate) {
                    case PRESSED:
                        if (key == 'D') {
                            chip8.set_paused(!chip8.get_paused());
                            if (chip8.get_paused()) {
                                showPausedScreen();
                            } else {
                                chip8.set_draw_flag(1);
                            }
                        }
                        if (key == '#') {
                            if (chip8.get_paused()) {
                                goto quit_emulator;
                            }
                        }
                        chip8.set_keypad_value(CHIP8_KEYMAP.at(key), 1);
                        break;
                        
                    case RELEASED:
                        chip8.set_keypad_value(CHIP8_KEYMAP.at(key), 0);
                        break;
                        
                    case IDLE:
                        break;
                    }
                }
            }
        }

        if (chip8.get_draw_flag()||(debug_mode && iter_count%16==0))
        {
            // Clear display buffer
            memset(display_buf, 0, display_buf_size);
            
            
            bool* raw_pixels = chip8.get_display_buffer();

            /*
            // More readable but slower implementation:
            for (int y = 0; y < 32; ++y)
            {
                for (int x = 0; x < 64; ++x) {
                    //Serial.println(display_buf[x+(y*64)]);
                    if(raw_pixels[x+(y*64)]) {
                    
                        u8g2.drawBox(x*2, y*2, 2, 2);
                    }
                }
            }
            */
           
           for (int cy = 0; cy < 32; ++cy) {
                for (int cx = 0; cx < 64; ++cx) {
                    if (!raw_pixels[cx + cy * 64]) continue;

                    const int sx = cx << 1;   // *2
                    const int sy = cy << 1;

                    // set the 2×2 block
                    for (int dy = 0; dy < 2; ++dy) {
                        const int y = sy + dy;
                        const int page = y >> 3;          // /8
                        const uint8_t mask = 1u << (y & 7);

                        uint8_t *row = display_buf + page * 128 + sx;
                        row[0] |= mask;
                        row[1] |= mask;
                    }
                }
            }
            u8g2.sendBuffer();
            
        }

        ++iter_count;
        ulong now = micros();
        ulong elapsed = now - start;
        
        
        if (!(iter_count%1024) && debug_mode /*&& chip8.get_draw_flag()*/) {
            Serial.printf("us per cycle: %lu\n", elapsed);
        }
        if (!fast_forward&&elapsed < 1500) {
            
            delayMicroseconds(1500-(elapsed));
        } 
        if (chip8.get_draw_flag()) chip8.set_draw_flag(false);
        start = micros();


    }
    quit_emulator: ;
    return 0;
}

void setup() {
    Serial.begin(115200);
    prefs.begin("chip-8");
    Wire.begin(SSD1306_SDA, SSD1306_SCL, 1000000);
    if (!u8g2.begin())
    {
        Serial.println("Failed to initialize display!");
        return;
    }
    u8g2.setBusClock(1000000);
    u8g2.setFont(u8g2_font_6x13_tr);

}

void loop() {
    selected_game = gameSelectionMenu(selected_game);
    startEmulator(ROMS_DATA[selected_game].data(), ROMS_DATA[selected_game].size());
}

