#include <Arduino.h>
#include "audio.h"

ToneGenerator::ToneGenerator(int PIN, int FREQ) : m_pin(PIN), m_freq(FREQ)
{
    //ledcAttach(PIN, FREQ, m_ledc_res);
}
ToneGenerator::~ToneGenerator()
{
    //ledcDetach(m_pin);
}
void ToneGenerator::setFrequency(int freq)
{
    m_freq = freq;
    //ledcChangeFrequency(m_pin, m_freq, m_ledc_res);
}
void ToneGenerator::play()
{
    //ledcWrite(m_pin, 128);
    playing = true;
}
void ToneGenerator::stop()
{
    //ledcWrite(m_pin, 0);
    playing = false;
}