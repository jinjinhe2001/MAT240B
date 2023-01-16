#include "everything.h"

int main(int argc, char* argv[]) {
    float phase = 0;
    double fourier = 0;

    for (float note = 127; note > 0; note -= 0.001) {
        float frequency = mtof(note);
        int maxH = ((double)SAMPLE_RATE / 2.0f * 0.8f) / frequency;
        for (int j = 0; j < maxH; j++) {
            fourier += sin(j * phase);
        }
        fourier /= maxH;
        mono(fourier * 0.707);
        phase += 2 * pi * frequency / SAMPLE_RATE;
        if (phase > 2 * pi)  //
        phase -= 2 * pi;
    }
}
