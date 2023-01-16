#include <cmath>  // sin, cos, ... tanh
#include <iostream>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double value;
  while (std::cin >> value) {
    mono(tanh(value));
  }
}
