#include "everything.h"  // SAMPLE_RATE, mono, uniform

int main(int argc, char* argv[]) {
  double seconds = 1;
  int N = seconds * SAMPLE_RATE;
  for (int n = 0; n < N; n++) {
    double v = uniform(1, -1);
    mono(v);
  }
}
