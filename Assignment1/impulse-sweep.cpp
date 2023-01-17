#include "everything.h"

int main(int argc, char* argv[]) {
    float phase = 0;
    double fourier = 0;
    double nyquist = (double)SAMPLE_RATE / 2.0f * 0.8;
    for (float note = 127; note > 0; note -= 0.001) {
        float frequency = mtof(note);
        int N = 0;
        fourier = 0;
        for (int j = 1; j * frequency <= nyquist; j++) {
            fourier += sin(j * phase);
            N++;
        }
        fourier /= N;
        mono(fourier * 0.707);
        phase += 2 * pi * frequency / SAMPLE_RATE;
        if (phase > 2 * pi)  //
            phase -= 2 * pi;
    }
}
