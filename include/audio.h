#pragma once

#include <atomic>
#include <cstdint>
#include <Wire.h>
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---------------------------------------------------------------------------
// ToneGenerator
//
// Drives a sine wave out through the DAC of an ES8311 codec over I2S, using
// the ESP32 as I2S master (3-wire mode: BCLK / LRCK / DOUT, no MCLK pin).
//
// ---------------------------------------------------------------------------
class ToneGenerator {
public:
    // FREQ      -> initial tone frequency in Hz
    // VOL -> volume (0-255)
    ToneGenerator(int FREQ, uint8_t VOL=0xBF);
    ~ToneGenerator();

    // Sets up I2C, configures the ES8311 registers, and sets up the I2S
    // peripheral + audio task. Call once before play().
    void begin(TwoWire* wire);

    void setFrequency(int freq);
    void setVolume(uint8_t vol);
    void play();
    void stop();
    void setMute(bool mute);
    bool get_playing();

private:
    // ---- user-provided config ----
    int _pinSclk = 41;
    int _pinLrck = 43;
    int _pinDsdin = 42;
    std::atomic<int> _freq;
    std::atomic<bool> _playing;

    uint8_t _volume;
    bool _began = false;

    // --- I2C config ---
    static constexpr uint8_t ES8311_ADDR = 0x18; // 7-bit address (CE pin low)
    TwoWire* _Wire = nullptr; 

 
    // ---- I2S config ----
    static constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
    static constexpr uint32_t SAMPLE_RATE = 48000;
    static constexpr size_t FRAMES_PER_BUFFER = 256; // stereo frames per write
    i2s_chan_handle_t _txHandle = nullptr;
 
    // ---- audio generation task ----
    TaskHandle_t _taskHandle = nullptr;
    static void audioTaskThunk(void* param);
    void audioTaskLoop();
 
    // ---- ES8311 low-level control ----
    void i2cInit();
    esp_err_t es8311WriteReg(uint8_t reg, uint8_t val);
    esp_err_t es8311ReadReg(uint8_t reg, uint8_t* val);
    void es8311Init();
    
 
    // ---- I2S setup ----
    void i2sInit();
};