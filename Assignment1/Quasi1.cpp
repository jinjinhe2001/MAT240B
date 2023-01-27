#include <iostream>
#include <cmath>
#include "Quasi.hpp"
#define pi 3.1415926
const double samplerate = 48000.0;

int main() {
    QuasiSaw saw;
    saw.config();
    for (int i = 0; i < 48000; i++) {
         printf("%lf\n", saw());
    }
}