//
// Created by sarbajit on 5/5/17.
//

#pragma once




struct DebugInfo {
    uint16_t* stack; //16 16bit values can be stored in stack
    uint16_t* I;
    uint16_t* pc;
    uint8_t* V; 
    bool* keypad;
};

class Chip8
{
private:
    //CPU
    uint16_t I;
    uint16_t pc;
    uint8_t V[16]; 
    uint8_t sp; //16 bit register I,16 bit program counter, 8 bit stack pointer
    static constexpr uint8_t stack_size = 16;
    uint16_t stack[stack_size]; //16 16bit values can be stored in stack


    uint8_t delay_timer, sound_timer; //delay and sound 8 bit registers

    //MEMORY
    uint8_t memory[4096]; //4k RAM

    //DISPLAY
    bool display[64 * 32]; //64*382 display

    const unsigned char chip8_fontset[80] =
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
    //KEYPAD
    bool keypad[16]; //hexadecimal keypad

    //flags
    bool draw_flag; //if true, need to draw
    bool paused = 0;

    //helper functions
    uint8_t get_nibble(int, int, int); //returns 4 bits from 1st argument
    // right shifting by second argument number of bits with optional third argument to & first


    ulong last_timer_update;


    
    DebugInfo debug_info {stack, &I, &pc, V, keypad};
    

public:
    Chip8(); //constructor
    bool load_rom(const uint8_t* rom_data, size_t len); //returns false if any error occurs while loading
    bool get_draw_flag();

    void set_draw_flag(bool);

    int single_cycle(bool trace_mode, bool sound_on, bool shift_quirk=false, bool I_quirk=false);

    bool get_display_value(int);
    bool* get_display_buffer();

    void set_keypad_value(int, bool);
    void set_paused(bool);
    bool get_paused();
    void decrease_delay_timer();
    void decrease_sound_timer();
    void reset();
    void seed_prng();
    DebugInfo& get_debug_info();
    
    uint8_t* get_memory();
    uint8_t get_delay_timer();
    uint8_t get_sound_timer();
    
    ~Chip8(); //destructor

};

