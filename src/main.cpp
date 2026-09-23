#include <Arduino.h>
#include <Adafruit_TCA8418.h>
#include "chip8.h"
#include "roms.h"
#include "keypad.h"
#include "tft_menu.h"
#include "audio.h"
#include "settings_menu.h"
#include "popup_screens.h"
#include "fonts.h"
#include <Wire.h>
#include "lgfx_conf.h"
#include <LovyanGFX.hpp>

//#include <Keypad.h>
#include <Preferences.h>
#include <string>

SET_LOOP_TASK_STACK_SIZE(16 * 1024);

const int KEYPAD_SDA = 8;
const int KEYPAD_SCL = 9;
const int KEYPAD_INT = 11;
volatile bool TCA8418_event = false;


const uint8_t BUZZER_VOL = 0x9F;
const int BUZZER_FREQ = 440;
const int FULLSCREEN_W = 240;
const int FULLSCREEN_H = 120;
const int MENU_OPTION_MAX_CHARACTERS = 21;
const char VERSION_STRING[] = "Version       0.6-adv";

static int selected_game = 0;

static const int MAX_SCALE = 4;
static uint16_t framebuffer[64*MAX_SCALE * 32*MAX_SCALE];

static Adafruit_TCA8418 keypad;
#ifdef PRECOMPUTED_SCALE_MAP
static uint8_t mapX[FULLSCREEN_W];
static uint8_t mapY[FULLSCREEN_H];
#endif
static LGFX tft;

static Preferences prefs;
static ToneGenerator emulator_audio(BUZZER_FREQ, BUZZER_VOL);

void KeypadIRQ() {
  TCA8418_event = true;
}

std::string joinOptionAndValue(const MenuOption& option, int value, int new_str_len=MENU_OPTION_MAX_CHARACTERS) {
    std::string tmp(new_str_len, ' ');
    int old_size = tmp.length();
    tmp.replace(0, option.name_len, option.name);
    if (option.value_map != nullptr) {
        int value_len = strlen(option.value_map->at(value));
        tmp.replace(new_str_len-value_len, value_len, option.value_map->at(value));
    } 
    else if(option.max_value == 1 && option.min_value == 0) 
    {
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
        
        int value = prefs.getInt(settings_arr.at(i).name, settings_arr.at(i).default_value);
        if (value > settings_arr.at(i).max_value || value < settings_arr.at(i).min_value)
        {
            Serial.print("WARN: The value of preference \"");
            Serial.print(settings_arr.at(i).name);
            Serial.println("\" is out of range and will be reset.");
            value = settings_arr.at(i).default_value;
            prefs.putInt(settings_arr.at(i).name, value);
        }
        if (settings_arr.at(i).value_map != nullptr) {
            if (settings_arr.at(i).value_map->find(value) == settings_arr.at(i).value_map->end()) {
                Serial.print("WARN: The value of preference \"");
                Serial.print(settings_arr.at(i).name);
                Serial.println("\" is invalid and will be reset.");
                value = settings_arr.at(i).default_value;
                prefs.putInt(settings_arr.at(i).name, value);
            }
        }
        settings_vec.push_back(joinOptionAndValue(settings_arr.at(i), value));
            
    }
    settings_vec.push_back(VERSION_STRING);
    TFTMenu settings_menu(tft, settings_vec);
    settings_menu.set_title("Settings");
    settings_menu.draw_title();
    settings_menu.draw();
    while (true) {
        if (TCA8418_event) {
            KeypadEvent event = HandleKeypadInput(keypad, TCA8418_event);
            //Serial.printf("Key event: raw=%d, pressed=%d, row=%d, col=%d\n", event, pressed, row, col);
            char key = CARDPUTER_KEYMAP[event.col][event.row].value_first;
            if (event.pressed) {
                if(key == '.') {
                    if (settings_menu.m_selected < settings_vec.size()-1) {++settings_menu.m_selected;}
                    else {settings_menu.m_selected = 0;}
                    settings_menu.draw();
                } else if (key == ';') {
                    if (settings_menu.m_selected > 0) {--settings_menu.m_selected;}
                    else {settings_menu.m_selected = settings_vec.size()-1;}
                    settings_menu.draw();
                
                } else if (key == ',' || key == '/') {
                    if (settings_menu.m_selected >= settings_arr.size()) continue;

                    MenuOption selected_option = settings_arr.at(settings_menu.m_selected);
                    
                    int prev_val = prefs.getInt(selected_option.name, selected_option.default_value);
                    int new_val;
                    if (key == '/')
                        new_val = { prev_val < selected_option.max_value ? prev_val + selected_option.step :  selected_option.min_value};
                    else
                        new_val = { prev_val > selected_option.min_value ? prev_val - selected_option.step  : selected_option.max_value};

                    prefs.putInt(selected_option.name, new_val);

                    settings_vec[settings_menu.m_selected] = joinOptionAndValue(selected_option, new_val);

                    settings_menu.draw();
                }
                else if (key == '`' || key == 's') {
                    break;
                }
            }
        }
    }

}

int gameSelectionMenu(int default_game = 0) {
    TFTMenu tft_menu(tft, ROM_NAMES);

    tft_menu.m_selected = default_game;
    tft_menu.set_title("Select a game:");
    tft_menu.draw_title();
    tft_menu.draw();
    
    while (true) {
        if (TCA8418_event) {
            KeypadEvent event = HandleKeypadInput(keypad, TCA8418_event);
            //Serial.printf("Key event: raw=%d, pressed=%d, row=%d, col=%d\n", event, pressed, row, col);
            char key = CARDPUTER_KEYMAP[event.col][event.row].value_first;
            if (event.pressed) {
                if(key == '.') {
                    if (tft_menu.m_selected < ROM_NAMES.size()-1) {++tft_menu.m_selected;}
                    else {tft_menu.m_selected = 0;}
                    tft_menu.draw();
                } else if (key == ';') {
                    if (tft_menu.m_selected > 0) {--tft_menu.m_selected;}
                    else {tft_menu.m_selected = ROM_NAMES.size()-1;}
                    tft_menu.draw();
                
                } else if (key == 's') {
                    settingsMenu();
                    tft_menu.draw_title();
                    tft_menu.draw();
                }
                else if (key == KEY_ENTER) {
                    break;
                }
            }
        } 
        keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
        
    }

    return tft_menu.m_selected;
}


int startEmulator(const uint8_t* rom_data, const ulong rom_size)
{
    Chip8 chip8;
    if (!chip8.load_rom(rom_data, rom_size)) 
    {
        Serial.println("ROM could not be loaded. Check rom size\n");
        return 1;
    }
    chip8.seed_prng();
    
    bool trace_mode = false, debug_mode = false, audio_on = false;
    bool i_quirk = prefs.getInt("Memory quirk"), s_quirk = prefs.getInt("Shift quirk"), fast_forward = prefs.getInt("Turbo");
    int scale = prefs.getInt("Scale", 3), audio_vol = prefs.getInt("Volume", 0xAF), audio_wf = prefs.getInt("Audio waveform", 1);

    if (audio_vol > 0x7F)
    {
        audio_on = true;
        emulator_audio.setMute(false);
        emulator_audio.setVolume(audio_vol);
    }

    emulator_audio.setWaveform((Waveform) audio_wf);

    bool fullscreen = scale == 4;
    
    bool screen_has_been_cleared = false;
    uint16_t iter_count = 0;
    int fb_w, fb_h;
    if (fullscreen) {
        fb_w = FULLSCREEN_W;
        fb_h = FULLSCREEN_H;
    } else {
        fb_w = 64 * scale;
        fb_h = 32 * scale;
    }
    const int x_offset = {fullscreen? 0 : tft.width()/2-fb_w/2};
    const int y_offset = tft.height()/2-fb_h/2;
    ulong start = micros();

    while (true)
    {

        if(chip8.single_cycle(trace_mode, audio_on, emulator_audio, s_quirk, i_quirk)) {
            Serial.print("Failed to execute last instruction. Exiting...\n");
            return 1;
        }
        
        if (TCA8418_event) {
            KeypadEvent event = HandleKeypadInput(keypad, TCA8418_event);
            //Serial.printf("Key event: raw=%d, pressed=%d, row=%d, col=%d\n", event, pressed, row, col);
            char key = CARDPUTER_KEYMAP[event.col][event.row].value_first;

            
            if (event.pressed) {
                

                if (key == 'p') {
                    chip8.set_paused(!chip8.get_paused());
                    if (chip8.get_paused()) {
                        emulator_audio.stop();
                        showPausedScreen(tft);
                    } else {
                        chip8.set_draw_flag(1);
                    }
                }

                if (key == '`') {
                    if (chip8.get_paused()) {
                        goto quit_emulator;
                    }
                }
                if(CHIP8_KEYMAP.find(key) != CHIP8_KEYMAP.end()) {
                    chip8.set_keypad_value(CHIP8_KEYMAP.at(key), 1);
                }
                    
            } else {
                if(CHIP8_KEYMAP.find(key) != CHIP8_KEYMAP.end()) {
                    chip8.set_keypad_value(CHIP8_KEYMAP.at(key), 0);
                }
            }
            
        }
        
        if (chip8.get_draw_flag()||(debug_mode && iter_count%16==0))
        {
            if (!screen_has_been_cleared) {
                tft.fillScreen(TFT_BLACK);
                screen_has_been_cleared = 1;
            }
            
            
            bool* raw_pixels = chip8.get_display_buffer();
            

            if (fullscreen) {
                for (int dy = 0; dy < fb_h; ++dy) {
                    #ifdef PRECOMPUTED_SCALE_MAP
                    int sy = mapY[dy];
                    #else
                    int sy = (dy * 32) / FULLSCREEN_H;
                    #endif
                    const bool* srcRow = raw_pixels + sy * 64;
                    uint16_t* dstRow = framebuffer + dy * fb_w;
                    for (int dx = 0; dx < fb_w; ++dx) {
                        #ifdef PRECOMPUTED_SCALE_MAP
                        dstRow[dx] = srcRow[mapX[dx]] ? TFT_WHITE : TFT_BLACK;
                        #else
                        dstRow[dx] = srcRow[(dx * 64) / FULLSCREEN_W] ? TFT_WHITE : TFT_BLACK;
                        #endif
                    }
                }
                
            }
            else {
                for (int y = 0; y < 32; ++y) {
                    for (int x = 0; x < 64; ++x) {
                        uint16_t color = raw_pixels[x + y*64] ? TFT_WHITE : TFT_BLACK;
                        // fill the scale x scale block for this chip8 pixel
                        for (int sy = 0; sy < scale; ++sy) {
                            int row = (y*scale + sy) * fb_w + x*scale;
                            for (int sx = 0; sx < scale; ++sx) {
                                framebuffer[row + sx] = color;
                            }
                        }
                    
                    }
                }

            }

            tft.pushImageDMA(x_offset, y_offset, fb_w, fb_h, framebuffer);
        
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
    emulator_audio.setMute(true);
    return 0;
}

#ifdef PRECOMPUTED_SCALE_MAP
constexpr void initScaleMaps() {
    for (int dx = 0; dx < FULLSCREEN_W; ++dx) mapX[dx] = (dx * 64) / FULLSCREEN_W;
    for (int dy = 0; dy < FULLSCREEN_H; ++dy) mapY[dy] = (dy * 32) / FULLSCREEN_H;
}
#endif

void setup() {
    Serial.begin(115200);
    prefs.begin("chip-8");
    Wire.begin(KEYPAD_SDA, KEYPAD_SCL, 400000);
    emulator_audio.begin(&Wire);
    if (! keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
        Serial.println("Couldn't communicate with keypad.");
        while (1);
    }
    
    keypad.matrix(7, 8);
    keypad.flush();
    
    pinMode(KEYPAD_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(KEYPAD_INT), KeypadIRQ, CHANGE);
    keypad.enableInterrupts();

    tft.init();
    tft.initDMA();
    
    tft.setRotation(1); // Landscape orientation
    tft.fillScreen(TFT_BLACK);
    #ifdef PRECOMPUTED_SCALE_MAP
    initScaleMaps();
    #endif
}

void loop() {
    
    selected_game = gameSelectionMenu(selected_game);
    showStartingScreen(tft);
    startEmulator(ROMS_DATA[selected_game].data(), ROMS_DATA[selected_game].size());
}

