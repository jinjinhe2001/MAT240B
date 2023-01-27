#include <iostream>
#include <cmath>

#define pi 3.1415926
class QuasiSaw
{
public:
    // variables and constants
    float osc; // output of the saw oscillator
    float osc2; // output of the saw oscillator 2
    float phase; // phase accumulator
    float w; // normalized frequency
    float scaling; // scaling amount
    float DC; // DC compensation
    //float *output; // pointer to array of floats
    float output = 0;
    float pw = 0.5; // pulse width of the pulse, 0..1
    float norm; // normalization amount
    float const a0 = 2.5f; // precalculated coeffs
    float const a1 = -1.5f; // for HF compensation
    float in_hist; // delay for the HF filter
public:
    // calculate w and scaling
    void config(double freq = 440.0, double samplerate = 44100) { // double freq, double sampleFrames, double samplerate
        double lastW = w;
        w = freq/samplerate; // normalized frequency
        float n = 0.5f-w;
        scaling = 13.0f * pow(n,4); // calculate scaling
        DC = 0.376f - w*0.752f; // calculate DC compensation
        osc = 0.f; phase = 0.f; // reset oscillator and phase
        norm = 1.0f - 2.0f*w; // calculate normalization
    }
    // saw wave
    float operator()() {
         // increment accumulator
        phase += 2.0f*w; if (phase >= 1.0f) phase -= 2.0f;
        // calculate next sample
        osc = (osc + sin(pi*(phase + osc*scaling)))*0.5f;
        // compensate HF rolloff
        float out = a0*osc + a1*in_hist; in_hist = osc;
        out = out + DC; // compensate DC offset
        output = out*norm; // store normalized result
        return output;
    }
};

class QuasiPulse
{
public:
    // variables and constants
    float osc; // output of the saw oscillator
    float osc2; // output of the saw oscillator 2
    float phase; // phase accumulator
    float w; // normalized frequency
    float scaling; // scaling amount
    float DC; // DC compensation
    //float *output; // pointer to array of floats
    float output = 0;
    float pw = 0.5; // pulse width of the pulse, 0..1
    float norm; // normalization amount
    float const a0 = 1.9f; // precalculated coeffs
    float const a1 = -0.9f; // for HF compensation
    float in_hist; // delay for the HF filter
    // calculate w and scaling
public:
    void config(double freq = 440.0, double samplerate = 44100) { // double freq, double sampleFrames, double samplerate
        double lastW = w;
        w = freq/samplerate; // normalized frequency
        if (fabs(w - lastW) < 1e-6) return;
        float n = 0.5f-w;
        scaling = 13.0f * pow(n,4); // calculate scaling
        DC = 0.376f - w*0.752f; // calculate DC compensation
        osc = 0.f; phase = 0.f; // reset oscillator and phase
        norm = 1.0f - 2.0f*w; // calculate normalization
    }
    // PWM pulse
    float operator()() {
        // increment accumulator
        phase += 2.0f*w; if (phase >= 1.0f) phase -= 2.0f;
        // calculate saw1
        osc = (0.45 * osc + 0.55* sin(pi*(phase - osc*osc*scaling)));
        // calculate saw2
        osc2 = (0.45 * osc2 + 0.55*sin(pi*(phase - osc2*osc2*scaling + 2*pw)));
        float out = osc-osc2; 
        // compensate HF rolloff
        out = a0*out + a1*in_hist; in_hist = osc-osc2;
        output = out*norm; // store normalized result
        return output;
    }
};
