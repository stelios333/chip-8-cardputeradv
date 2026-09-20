#include "audio.h"

#include <cmath>

#include "esp_check.h"
#include "esp_log.h"

static const char* TAG = "ToneGenerator";

// ---------------------------------------------------------------------------
// ES8311 register addresses (subset needed for basic DAC playback).
// See the ES8311 datasheet for the full register map.
// ---------------------------------------------------------------------------
#define ES8311_RESET_REG00 0x00
#define ES8311_CLK_MANAGER_REG01 0x01
#define ES8311_CLK_MANAGER_REG02 0x02
#define ES8311_CLK_MANAGER_REG03 0x03
#define ES8311_CLK_MANAGER_REG04 0x04
#define ES8311_CLK_MANAGER_REG05 0x05
#define ES8311_CLK_MANAGER_REG06 0x06
#define ES8311_CLK_MANAGER_REG07 0x07
#define ES8311_CLK_MANAGER_REG08 0x08
#define ES8311_SDPIN_REG09 0x09
#define ES8311_SDPOUT_REG0A 0x0A
#define ES8311_SYSTEM_REG0B 0x0B
#define ES8311_SYSTEM_REG0C 0x0C
#define ES8311_SYSTEM_REG0D 0x0D
#define ES8311_SYSTEM_REG0E 0x0E
#define ES8311_SYSTEM_REG0F 0x0F
#define ES8311_SYSTEM_REG10 0x10
#define ES8311_SYSTEM_REG11 0x11
#define ES8311_SYSTEM_REG12 0x12
#define ES8311_SYSTEM_REG13 0x13
#define ES8311_SYSTEM_REG14 0x14
#define ES8311_ADC_REG16 0x16
#define ES8311_ADC_REG1B 0x1B
#define ES8311_ADC_REG1C 0x1C
#define ES8311_DAC_REG31 0x31 // bit6 = mute
#define ES8311_DAC_REG32 0x32 // DAC volume, 0x00=mute .. 0xFF=max
#define ES8311_DAC_REG37 0x37
#define ES8311_GP_REG45 0x45

ToneGenerator::ToneGenerator(int FREQ, uint8_t VOL)
    : _freq(FREQ),
      _volume(VOL),
      _playing(false) {}

ToneGenerator::~ToneGenerator() {
    stop();
 
    if (_taskHandle) {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }
 
}

void ToneGenerator::begin(TwoWire* wire) {
    if (_began) {
        return;
    }

    _Wire = wire;
    es8311Init();
    i2sInit();

    xTaskCreatePinnedToCore(&ToneGenerator::audioTaskThunk, "tone_gen_task",
                             4096, this, 5, &_taskHandle, 1);

    _began = true;
    ESP_LOGI(TAG, "ToneGenerator initialized");
}

void ToneGenerator::setFrequency(int freq) {
    _freq.store(freq);
}

void ToneGenerator::setVolume(uint8_t vol) {
    _volume = vol;
    es8311WriteReg(ES8311_DAC_REG32, _volume);
}

void ToneGenerator::play() {
    _playing.store(true);
}

void ToneGenerator::stop() {
    _playing.store(false);
}

bool ToneGenerator::get_playing() {
    return _playing.load();
}



esp_err_t ToneGenerator::es8311WriteReg(uint8_t reg, uint8_t val) {
    if (_Wire == nullptr) return 1;
    _Wire->beginTransmission(ES8311_ADDR);
    _Wire->write(reg);
    _Wire->write(val);
    return _Wire->endTransmission() == 0;
}
 
esp_err_t ToneGenerator::es8311ReadReg(uint8_t reg, uint8_t* val) {
    if (_Wire == nullptr) return 1;
    _Wire->beginTransmission(ES8311_ADDR);
    _Wire->write(reg);
    _Wire->endTransmission(false);

    _Wire->requestFrom(ES8311_ADDR, (uint8_t)1);
    if (_Wire->available()) {
        *val = _Wire->read();
    }
    return 0;
}



void ToneGenerator::es8311Init() {
    es8311WriteReg(ES8311_RESET_REG00, 0x80);   // RESET / CSM power on
    vTaskDelay(pdMS_TO_TICKS(20));
    es8311WriteReg(ES8311_CLK_MANAGER_REG01, 0xB5);   // CLK manager: MCLK = BCLK
    es8311WriteReg(ES8311_CLK_MANAGER_REG02, 0x18);   // CLK manager: premulti=3 -> внутр. клок = 256*fs
    es8311WriteReg(ES8311_SYSTEM_REG0D, 0x01);   // power up analog
    es8311WriteReg(ES8311_SYSTEM_REG12, 0x00);   // power up DAC
    es8311WriteReg(ES8311_SYSTEM_REG13, 0x10);   // enable output to drive
    
    // Output volume (0x00 mute .. 0xFF max)
    es8311WriteReg(ES8311_DAC_REG32, _volume);

    es8311WriteReg(ES8311_DAC_REG37, 0x08);  // bypass DAC equalizer

    setMute(true);
}

void ToneGenerator::setMute(bool mute) {
    if (!_began) return;
    uint8_t val = 0;
    es8311ReadReg(ES8311_DAC_REG31, &val);
    if (mute) {
        val |= 0x40;
    } else {
        val &= ~0x40;
    }
    es8311WriteReg(ES8311_DAC_REG31, val);
}

// ---------------------------------------------------------------------------
// I2S setup (ESP-IDF i2s_std driver, 3-wire master: BCLK/LRCK/DOUT)
// ---------------------------------------------------------------------------
void ToneGenerator::i2sInit() {
    i2s_chan_config_t chanCfg =
        I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chanCfg, &_txHandle, nullptr));

    i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = static_cast<gpio_num_t>(_pinSclk),
                .ws = static_cast<gpio_num_t>(_pinLrck),
                .dout = static_cast<gpio_num_t>(_pinDsdin),
                .din = I2S_GPIO_UNUSED,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(_txHandle, &stdCfg));
    ESP_ERROR_CHECK(i2s_channel_enable(_txHandle));
}

// ---------------------------------------------------------------------------
// Audio generation task: continuously fills the I2S DMA buffer. Writes
// silence when not playing so the bus keeps running cleanly (and so
// play()/stop() are instant, with no re-init needed).
// ---------------------------------------------------------------------------
void ToneGenerator::audioTaskThunk(void* param) {
    static_cast<ToneGenerator*>(param)->audioTaskLoop();
}

void ToneGenerator::audioTaskLoop() {
    static int16_t buffer[FRAMES_PER_BUFFER * 2]; // interleaved L/R

    double phase = 0.0;

    while (true) {
        bool playing = _playing.load();
        int freq = _freq.load();

        if (playing && freq > 0) {
            const double phaseInc = 2.0 * M_PI * freq / SAMPLE_RATE;
            for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
                int16_t sample =
                    static_cast<int16_t>(sin(phase) * 32000.0); // headroom
                buffer[2 * i] = sample;     // left
                buffer[2 * i + 1] = sample; // right
                phase += phaseInc;
                if (phase >= 2.0 * M_PI) {
                    phase -= 2.0 * M_PI;
                }
            }
        } else {
            memset(buffer, 0, sizeof(buffer));
            phase = 0.0;
        }

        size_t bytesWritten = 0;
        i2s_channel_write(_txHandle, buffer, sizeof(buffer), &bytesWritten,
                           portMAX_DELAY);
    }
}