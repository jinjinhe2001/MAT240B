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
    int nyquist = (double)SAMPLE_RATE / 2.0f * 0.8f;
    for (int i = 1; i <= duration * SAMPLE_RATE; ++i) {
        fourier = 0;
        int N = 0;
        for (int j = 1; j * frequency <= nyquist; j++) {
            fourier += sin(j * phase) / (double)j;
            N++;
        }
        fourier /= 2;
        mono(fourier);

        phase += 2 * pi * frequency / SAMPLE_RATE;

        if (phase > 2 * pi) {
        phase -= 2 * pi;
        }
    }

    return 0;
}
