#include <string>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
    double frequency = 440.0;

    if (argc > 1) {
        frequency = std::stof(argv[1]);
    }

    double duration = 1;
    if (argc > 2) {
        duration = std::stof(argv[2]);
    }

    double phase = 0;
    double fourier = 0;
    // below 80% of the Nyquist frequency
    int maxH = (double)SAMPLE_RATE / 2.0f * 0.8f / frequency;
    for (int i = 0; i < duration * SAMPLE_RATE; ++i) {
        fourier = 0;
        for (int j = 0; j < maxH; j++) {
            fourier += sin(j * phase);
        }
        fourier /= maxH;
        mono(fourier);

        phase += 2 * pi * frequency / SAMPLE_RATE;

        if (phase > 2 * pi) {
        phase -= 2 * pi;
        }
    }

    return 0;
}
