Build and test like this:

    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./sine-sweep | ./wav-write sine-sweep.wav
    # open sine-sweep.wav in a DAW and look at the spectrum

Aspects of the UNIX philosophy are present in Cycling74's Max environment and other audio programming systems. We make lots of little programs that generate outputs and/or analyse or process inputs. We then wires these together to make more complex programs. This structure is a [graph](https://en.wikipedia.org/wiki/Graph_(abstract_data_type)).

The files below have empty implementations:

- `impulse.cpp`
- `sawtooth.cpp`
- `square.cpp`
- `triangle.cpp`
- `impulse-sweep.cpp`
- `sawtooth-sweep.cpp`
- `square-sweep.cpp`
- `triangle-sweep.cpp`

Complete these implementations taking care to limit harmonic partials to those below 80% of the Nyquist freqnency.

