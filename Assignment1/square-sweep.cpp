#include "everything.h"

int main(int argc, char* argv[]) {
    float phase = 0;
    double fourier = 0;
    double nyquist = (double)SAMPLE_RATE / 2.0f * 0.8;
    int index = 0;
    for (float note = 127; note > 0; note -= 0.001) {
        float frequency = mtof(note);
        int N = 0;
        fourier = 0;
        for (int j = 1; j * frequency <= nyquist; j += 2) {
            fourier += sin(j * phase) / (double)j;
            N++;
        }
        fourier *= (4 / pi);
        mono(fourier * 0.707);
        phase += 2 * pi * frequency / SAMPLE_RATE;
        if (phase > 2 * pi)  //
            phase -= 2 * pi;
    }
}
