#pragma once

class ToneGenerator
{
private:
    int m_freq = 440;
    const int m_pin = 0;
    const int m_ledc_channel = 0;
    const int m_ledc_res = 8;

public:
    ToneGenerator(int PIN, int FREQ);
    ~ToneGenerator();
    void setFrequency(int);
    void play();
    void stop();
    bool playing = false;
};