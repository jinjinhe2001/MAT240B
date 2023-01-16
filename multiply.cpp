#include <iostream>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double gain = 1;
  if (argc > 1) {
    gain = std::stof(argv[1]);
  }
  double value;
  while (std::cin >> value) {
    mono(value * gain);
  }
}
