#include <Arduino.h>
#include "audio.h"
#include "chip8.h"

//constructor
Chip8::Chip8()
{
    reset();   
}

void Chip8::reset() {
    pc = 0x200; // 0x000 to 0x1FF is reserved for  interpreter

    //resetting registers
    I = 0;
    sp = 0;
    sound_timer = 0;
    delay_timer = 0;
    
    //clear registers, stack and memory
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));
    
    draw_flag = false;

    //load fontset from 0 to 80
    for (int i = 0; i < 80; ++i)
    {
        memory[i] = chip8_fontset[i];
    }

    //resetting display and keypad
    memset(display, 0, sizeof(display));
    memset(keypad, 0, sizeof(keypad));
    ulong last_timer_update = millis();
}

DebugInfo& Chip8::get_debug_info() {
    return debug_info;
}

uint8_t* Chip8::get_memory() {
    return memory;
}
//function to load ROM, with path to ROM given as argument
bool Chip8::load_rom(const uint8_t* rom_data, size_t len)
{
    // load rom in memory at address 0x200
    int j = 512;
    // ensure rom can fit in memory
    if (len >= 4096-j)
    {
        return false; 
    }
    memcpy(memory+j, rom_data, len);
    return true;
}


bool Chip8::get_draw_flag()
{
    return draw_flag;
}

void Chip8::set_draw_flag(bool flag)
{
    draw_flag = flag;
}

bool Chip8::get_display_value(int i)
{
    return display[i];
}

bool* Chip8::get_display_buffer() 
{
    return display;
}

void Chip8::set_keypad_value(int index, bool val)
{
    keypad[index] = val;
}

uint8_t Chip8::get_sound_timer()
{
    return sound_timer;
}

void Chip8::decrease_sound_timer()
{
    --sound_timer;
}

void Chip8::decrease_delay_timer()
{
    --delay_timer;
}

uint8_t Chip8::get_delay_timer()
{
    return delay_timer;
}

void Chip8::set_paused(bool is_paused) {
    paused = is_paused;
}

bool Chip8::get_paused() {
    return paused;
}

void Chip8::seed_prng() {
    srand(time(NULL));
}

int Chip8::single_cycle(bool trace_mode, bool sound_on, ToneGenerator& audio, bool shift_quirk, bool memory_quirk)
{
    if (paused) return 0;

    // 2 byte opcode
    uint16_t opcode = (memory[pc] << 8) | (memory[pc + 1]);
    uint8_t opcode_msb_nibble = get_nibble(opcode, 12, 0xF000);
    uint8_t x_reg = get_nibble(opcode, 8, 0x0F00);
    uint8_t y_reg = get_nibble(opcode, 4, 0x00F0);
    uint8_t val, initial_value;
    bool single_bit;
    

    if (trace_mode)
    {
    
        Serial.printf("%#05x: %#04x %02x\n",
              static_cast<unsigned>(pc),
              static_cast<unsigned>(memory[pc]),
              static_cast<unsigned>(memory[pc + 1]));
    }

    switch (opcode_msb_nibble)
    {
        case 0:
            switch (opcode)
            {
                case 0x00E0:
                    memset(display, 0, sizeof(display));
                    draw_flag = true;
                    pc += 2;
                    break;

                case 0x00EE:
                    if (sp > 0) {
                        --sp;
                        pc = stack[sp];
                        pc += 2;
                    } else {
                        Serial.println("Not in a subroutine, could not return.");
                        return 1;
                    }
                    break;

                default:
                    Serial.printf("Unknown opcode -> %#06x\n", opcode);
                    break;
            }
            break;

        case 1:
            pc = opcode & 0x0FFF;
            break;

        case 2:
            // 2NNN: Call subroutine at NNN.
            // so push the current address to the stack and move pc to NNN
            if (!(sp > stack_size-1)) {
                stack[sp] = pc;
                ++sp;
                pc = opcode & 0x0FFF;
            } else {
                Serial.println("Error: Reached maximum nesting level, unable to call subroutine.");
                return 1;
            }
            break;

        case 3:
            // 3XNN: Skip the next instruction if VX equals NN.
            val = get_nibble(opcode, 0, 0x00FF); // Extracts the lower byte
            
            pc += 2;
            if (V[x_reg] == val)
            {
                pc += 2;
            }
            break;

        case 4:
            // 4XNN: Skip the next instruction if VX doesn't equal NN.
            val = get_nibble(opcode, 0, 0x00FF);
            pc += 2;
            if (V[x_reg] != val)
            {
                pc += 2;
            }
            break;

        case 5:
            // 5XY0: Skip the next instruction if VX equals VY.
            pc += 2;
            if (V[x_reg] == V[y_reg])
            {
                pc += 2;
            }
            break;

        case 6:
            // 6XNN: Set VX to NN.
            val = get_nibble(opcode, 0, 0x00FF);
            V[x_reg] = val;
            pc += 2;
            break;

        case 7:
            // 7XNN: Add NN to VX.
            val = get_nibble(opcode, 0, 0x00FF);
            V[x_reg] += val;
            pc += 2;
            break;

        case 8:
            val = get_nibble(opcode, 0, 0x000F); // extract last 4 bits
            switch (val)
            {
                case 0:
                    // 8XY0: Sets VX to the value of VY.
                    V[x_reg] = V[y_reg];
                    pc += 2;
                    break;

                case 1:
                    // 8XY1: Sets VX to VX or VY (Bitwise OR operation). VF is reset to 0.
                    V[x_reg] |= V[y_reg];
                    V[0xF] = 0;
                    pc += 2;
                    break;

                case 2:
                    //8XY2: Set VX to VX and VY (Bitwise AND operation). VF is reset to 0.
                    V[x_reg] &= V[y_reg];
                    V[0xF] = 0;
                    pc += 2;
                    break;

                case 3:
                    //8XY3: Set VX to VX xor VY. VF to 0
                    V[x_reg] ^= V[y_reg];
                    V[0xF] = 0;
                    pc += 2;
                    break;

                case 4:
                {
                    // 8XY4: Add VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                    initial_value = V[x_reg];
                    V[x_reg] += V[y_reg];
                    //V[reg1] = (uint8_t) V[reg1];
                    V[0xf] = initial_value > V[x_reg] || V[y_reg] > V[x_reg];
                    pc += 2;
                    break;
                }
                case 5:
                    // 8XY5: VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there isn't.
                    single_bit = V[x_reg] >= V[y_reg];
                    V[x_reg] = V[x_reg] - V[y_reg];
                    V[0xf] = (uint8_t) single_bit;
                    pc += 2;
                    break;

                case 6:
                    // 8XY6:
                    // Optionaly copy the set VX to the value of VY then,
                    // shift VX right by one. VF is set to the value of the least significant bit of VX before the shift.
                    if (shift_quirk) V[x_reg] = V[y_reg];
                    single_bit = V[x_reg] & 0x1;
                    V[x_reg] >>= 1;
                    V[0xf] = (uint8_t) single_bit;
                    pc += 2;
                    break;

                case 7:
                    // 8XY7:
                    // Set VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there isn't.
                    single_bit = V[y_reg] >= V[x_reg];
                    V[x_reg] = V[y_reg] - V[x_reg];
                    V[0xf] = single_bit;
                    pc += 2;
                    break;

                case 0xE:
                    // 8XYE:
                    // Optionaly copy the set VX to the value of VY then,
                    // shift VX left by one. VF is set to the value of the most significant bit of VX before the shift.[2]
                    if (shift_quirk) V[x_reg] = V[y_reg];
                    single_bit = V[x_reg] >> 7;
                    V[x_reg] <<= 1;
                    V[0xF] = (uint8_t) single_bit;
                    pc += 2;
                    break;

                default:
                    Serial.printf("Unknown opcode -> %#06x\n", opcode);
                    break;
            }
            break;

        case 9:
            // 9XY0: Skip the next instruction if VX doesn't equal VY.
            pc += 2;
            if (V[x_reg] != V[y_reg])
            {
                pc += 2;
            }
            break;

        case 0xA: // 0xA
            // ANNN: Set I to the address NNN.
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB: 
            // BNNN: Jump to the address NNN plus V0.
            pc = (opcode & 0x0FFF);
            pc += V[0];
            break;

        case 0xC: 
            // CXNN: Set VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            val = get_nibble(opcode, 0, 0x00FF); //extract the lower 8 bits
            V[x_reg] = rand() % 256 & val;
            pc += 2;
            break;

        case 0xD: //0xD
        {
            //DXYN: Draw a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
            // Each row of 8 pixels is read as bit-coded starting from memory location I;
            // I value doesn’t change after the execution of this instruction.
            // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen


            // My implementation was buggy so I got claude to write it. Sorry :/

            uint8_t x = V[x_reg] % 64;   // wrap starting position onto the screen
            uint8_t y = V[y_reg] % 32;
            const uint8_t ht = opcode & 0x000F; //N
            const uint8_t wt = 8;
            V[0x0F] = 0;

            for (int i = 0; i < ht; ++i)
            {
                int pixel = memory[I + i];
                int py = y + i;
                if (py >= 32) break;          // clip: stop drawing rows off the bottom

                for (uint8_t j = 0; j < wt; ++j)
                {
                    int px = x + j;
                    if (px >= 64) continue;   // clip: skip pixels off the right edge

                    if ((pixel & (0x80 >> j)) != 0)
                    {
                        int index = px + (py * 64);
                        if (display[index])
                        {
                            V[0x0F] = 1;
                        }
                        display[index] ^= 1;
                    }
                }
            }

            draw_flag = true;
            pc += 2;
            break;
        }
        case 0xE:
            val = get_nibble(opcode, 0, 0x00FF);
            switch (val)
            {
                case 0x9E:
                    // EX9E: Skip the next instruction if the key stored in VX is pressed.
                    pc += 2;
                    if (keypad[V[x_reg]] != 0)
                    {
                        pc += 2;
                    }
                    break;

                case 0xA1:
                    // EXA1: Skip the next instruction if the key stored in VX isn't pressed.
                    pc += 2;
                    if (keypad[V[x_reg]] == 0)
                    {
                        pc += 2;
                    }
                    break;

                default:
                    Serial.printf("Unknown opcode -> %#06x\n", opcode);
                    break;

            }
            break;

        case 0xF: 
            val = get_nibble(opcode, 0, 0x00FF);
            switch (val)
            {
                case 0x07:
                    // FX07: Set VX to the value of the delay timer.
                    V[x_reg] = delay_timer;
                    pc += 2;
                    break;

                case 0x0A:
                    // FX0A: A key press is awaited, and then stored in VX (blocking operation).
                {
                    bool key_pressed = false;
                    for (int i = 0; i < 16; ++i)
                    {
                        if (keypad[i] != 0)
                        {
                            key_pressed = true;
                            V[x_reg] = (uint8_t) i;
                        }
                    }
                    if (key_pressed)
                    {
                        pc += 2;
                    }
                    break;
                }

                case 0x15:
                    // FX15: Set the delay timer to VX.
                    delay_timer = V[x_reg];
                    pc += 2;
                    break;

                case 0x18:
                    // FX18: Set the sound timer to VX
                    sound_timer = V[x_reg];
                    pc += 2;
                    break;

                case 0x1E:
                    // FX1E: Add VX to I
                    I += V[x_reg];
                    //I = (uint16_t) I;
                    pc += 2;
                    break;

                case 0x29:
                    // FX29: Set I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    I = V[x_reg] * 0x5;
                    pc += 2;
                    break;

                case 0x33:
                    // FX33:
                    // Store the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, 
                    // the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, 
                    // place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)

                    memory[I] = (uint8_t) (V[x_reg] / 100);
                    memory[I + 1] = (uint8_t) ((V[x_reg] / 10) % 10);
                    memory[I + 2] = (uint8_t) ((V[x_reg]) % 10);
                    pc += 2;
                    break;

                case 0x55:
                    // FX55: Store V0 to VX (including VX) in memory starting at address I
                    for (int i = 0; i <= x_reg; ++i)
                    {
                        memory[I + i] = V[i];
                    }
                    if (memory_quirk) {
                        I = I + x_reg + 1U;
                        I = (uint16_t) I;
                    }
                    pc += 2;
                    break;

                case 0x65:
                    // FX65: Fill V0 to VX (including VX) with values from memory starting at address I
                    for (int i = 0; i <= x_reg; ++i)
                    {
                        V[i] = memory[I + i];
                    }
                    if (memory_quirk) {
                        I = I + x_reg + 1U;
                        I = (uint16_t) I;
                    }
                    pc += 2;
                    break;

                default:
                    Serial.printf("Invalid opcode -> %#06x\n", opcode);
                    break;
            }
            break;

        default:
            Serial.printf("Invalid opcode -> %#06x\n", opcode);
            break;
    }
    ulong now = millis();
    ulong elapsed = now - last_timer_update;

    const int ms_per_tick = 1000 / 60;
    
    if (elapsed >= ms_per_tick) {
        uint8_t ticks = elapsed / ms_per_tick;
        if (delay_timer > 0) {
            //Serial.print("Ticks: ");
            delay_timer = (ticks >= delay_timer) ? 0 : delay_timer - ticks;
        }
        if (sound_timer > 0) {
            sound_timer = (ticks >= sound_timer) ? 0 : sound_timer - ticks;
            if (sound_on) {
                
                if (sound_timer>0) {
                    if (!audio.playing) {
                        audio.play();
                    }
                } else {
                    if (audio.playing){
                        audio.stop();
                    } 
                }
                
            }
        }

        last_timer_update = now;
    }
    return 0;
    
}

//destructor
Chip8::~Chip8()
{}

//helper functions
uint8_t Chip8::get_nibble(int val, int bits, int val_to_binary_and = 0xFFFF) //extracts 4 bits from val
{
    return ((val & val_to_binary_and) >> bits);
}
