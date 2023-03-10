// Karl Yerkes
// 2023-01-30
// MAT 240B ~ Audio Programming
// Assignment 3 ~ Karplus-Strong string modeling
// Jinjin He

#include <juce_audio_processors/juce_audio_processors.h>

template <typename T>
T mtof(T m) {
  return T(440) * pow(T(2), (m - T(69)) / T(12));
}
template <typename T>
T dbtoa(T db) {
  return pow(T(10), db / T(20));
}

// valid on (-1, 1)
template <class T>
inline T sine(T n) {
  T nn = n * n;
  return n * (T(3.138982) +
              nn * (T(-5.133625) + nn * (T(2.428288) - nn * T(0.433645))));
}

template <class T>
inline T softclip(T x) {
  if (x >= T(1)) return T(1);
  if (x <= T(-1)) return T(-1);
  return (T(3) * x - x * x * x) / T(2);
}

template <class T>
inline T wrap(T v, T hi, T lo) {
  if (lo == hi) return lo;

  // if(v >= hi){
  if (!(v < hi)) {
    T diff = hi - lo;
    v -= diff;
    if (!(v < hi)) v -= diff * (T)(unsigned)((v - lo) / diff);
  } else if (v < lo) {
    T diff = hi - lo;
    v += diff;  // this might give diff if range is too large, so check at end
                // of block...
    if (v < lo) v += diff * (T)(unsigned)(((lo - v) / diff) + 1);
    if (v == diff) return std::nextafter(v, lo);
  }
  return v;
}
// DelayLine from Karl
class DelayLine : std::vector<float> {
  //
  int index = 0;

 public:
  float read(float seconds_ago, float samplerate) {
    //
    jassert(seconds_ago < size() / samplerate);

    float i = index - seconds_ago * samplerate;
    if (i < 0) {
      i += size();
    }
    return at((int)i);  // no linear interpolation
  }

  void write(float value) {
    jassert(size() > 0);
    at(index) = value;  // overwrite the oldest value

    // handle the wrapping for circular buffer
    index++;
    if (index >= size()) index = 0;
  }

  void allocate(float seconds, float samplerate) {
    // floor(seconds * samplerate) + 1 samples
    resize((int)floor(seconds * samplerate) + 1);
  }
};
// Biquad Filter from Karl
class BiquadFilter {
  // Audio EQ Cookbook
  // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

  // x[n-1], x[n-2], y[n-1], y[n-2]
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

  // filter coefficients
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

 public:
  float operator()(float x0) {
    // Direct Form 1, normalized...
    float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    y2 = y1;
    y1 = y0;
    x2 = x1;
    x1 = x0;
    return y0;
  }

  void normalize(float a0) {
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
  }

  void lpf(float f0, float Q, float samplerate) {
    float w0 = 2 * float(M_PI) * f0 / samplerate;
    float alpha = sin(w0) / (2 * Q);
    b0 = (1 - cos(w0)) / 2;
    b1 = 1 - cos(w0);
    b2 = (1 - cos(w0)) / 2;
    float a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    normalize(a0);
  }
};

struct BooleanOscillator {
  float value = 1;
  float increment = 0;
  void frequency(float hertz, float samplerate) {
    assert(hertz >= 0);
    increment = hertz / samplerate;
  }
  void period(float hertz, float samplerate) {
    frequency(1 / hertz, samplerate);
  }
  bool operator()() {
    value += increment;
    bool b = value >= 1;
    value = wrap(value, 1.0f, 0.0f);
    return b;
  }
};

// https://en.wikipedia.org/wiki/Harmonic_oscillator
struct MassSpringModel {
  // this the whole state of the simulation
  //
  float position{0};  // m
  float velocity{0};  // m/s

  // These are cached properties of the model; They govern the behaviour. We
  // recalculate them given frequency, decay time, and playback rate.
  //
  float springConstant{0};      // N/m
  float dampingCoefficient{0};  // N??s/m
  float timeStep{1.0f};
  float mass{1.0f};

  void show() {
    printf("position:%f velocity:%f springConstant:%f dampingCoefficient:%f\n",
           position, velocity, springConstant, dampingCoefficient);
  }
  void reset() {
    // show();
    position = 0;
    velocity = 0;
  }

  float operator()() {
    // This is semi-implicit Euler integration with time-step 1. The
    // playback rate is "baked into" the constants. Spring force and damping
    // force are accumulated into velocity. We let mass is 1, so it
    // disappears. Velocity is accumulated into position which is
    // interpreted as oscillator amplitude.
    //
    float acceleration = 0;

    // Jinjin put code here
    acceleration += (-springConstant * position - 
                    dampingCoefficient * velocity) / mass;


    velocity += acceleration * timeStep;
    position += velocity * timeStep;

    
      /*printf("position:%f velocity:%f springConstant:%f dampingCoefficient: %f\n",
       position, velocity, springConstant,
       dampingCoefficient);
      */
    return position;
  }

  // Use these to measure the kinetic, potential, and total energy of the
  // system.
  float ke() { return velocity * velocity / 2; }
  float pe() { return position * position * springConstant / 2; }
  float te() { return ke() + pe(); }

  // "Kick" the mass-spring system such that we get a nice (-1, 1) oscillation.
  //
  void trigger() {
    // We want the "mass" to move in (-1, 1). What is the potential energy
    // of a mass-spring system at 1? PE == k * x * x / 2 == k / 2. So, we
    // want a system with k / 2 energy, but we don't want to just set the
    // displacement to 1 because that would make a click. Instead, we want
    // to set the velocity. What velocity would we need to have energy k /
    // 2? KE == m * v * v / 2 == k / 2. or v * v == k. so...
    //

    // Jinjin put code here
    velocity += sqrt(springConstant / mass);

    // How might we improve on this? Consider triggering at a level
    // depending on frequency according to the Fletcher-Munson curves.
  }

  void recalculate(float frequency, float decayTime, float playbackRate) {
    // sample rate is "baked into" these constants to save on per-sample
    // operations.

    // Jinjin put code here
    springConstant = mass * ((frequency * 2 * M_PI / playbackRate) 
      * (frequency * 2 * M_PI / playbackRate) + 1 / (decayTime * playbackRate * 
      decayTime * playbackRate));
    dampingCoefficient = mass * (2 / (decayTime * playbackRate));
  }
};

struct KarpusStrongModel {
  // Jinjin put code here
  DelayLine delay;
  BiquadFilter filter;
  float delayTime;
  float gain;
  float frequency;
  float samplerate_;

  void configure(float hertz, float seconds, float samplerate) {
    // given t60 (`seconds`) and frequency (`Hertz`), calculate
    // the gain...
    //
    // for a given frequency, our algorithm applies *gain*
    // frequency-many times per second. given a t60 time we can
    // calculate how many times (n)  gain will be applied in
    // those t60 seconds. we want to reduce the signal by 60dB
    // over t60 seconds or over n-many applications. this means
    // that we want gain to be a number that, when multiplied
    // by itself n times, becomes 60 dB quieter than it began.
    //
    // the size of the delay *is* the period of the vibration
    // of the string, so 1/period = frequency.
    frequency = hertz;
    delayTime = 1 / hertz;
    gain = pow(dbtoa(-60), 1.0f / (seconds / delayTime));
    samplerate_ = samplerate;
  }
  
  float trigger() {
    // fill the delay line with noise
    return 1.0;
  }
  
  float operator()() {
    float v = filter(delay.read(delayTime, samplerate_)) * gain;
    delay.write(v);
    return v;
  }
};

using namespace juce;


class KarplusStrong : public AudioProcessor {
  AudioParameterFloat* gain;
  AudioParameterFloat* note;
  AudioParameterBool* trigger;
  BooleanOscillator timer;
  MassSpringModel string;
  bool triggerLast{false};
  /// add parameters here ///////////////////////////////////////////////////
  TextButton triggerButton{ TRANS("Trigger") };
 public:
  KarplusStrong()
      : AudioProcessor(BusesProperties()
                           .withInput("Input", AudioChannelSet::stereo())
                           .withOutput("Output", AudioChannelSet::stereo())) {
    addParameter(gain = new AudioParameterFloat(
                     {"gain", 1}, "Gain",
                     NormalisableRange<float>(-65, -1, 0.01f), -65));
    addParameter(
        note = new AudioParameterFloat(
            {"note", 1}, "Note", NormalisableRange<float>(-2, 129, 0.01f), 40));
    /// add parameters here /////////////////////////////////////////////
    // XXX juce::getSampleRate() is not valid here
    addParameter(
        trigger = new AudioParameterBool(
          {"trigger", 1}, "trigger", false
        )
    );
  }


  float previous = 0;

  /// handling the actual audio! ////////////////////////////////////////////
  void triggerF() {
    float r = 0.1 + 0.9 * Random::getSystemRandom().nextFloat();
    timer.period(r / 2 + 0.5f, (float)getSampleRate());
    string.recalculate(mtof(note->get()), r * r * r,
                        (float)getSampleRate());
    string.trigger();
  }
  void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override {
    buffer.clear(0, 0, buffer.getNumSamples());
    auto left = buffer.getWritePointer(0, 0);
    auto right = buffer.getWritePointer(1, 0);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
      if (isnan(previous)) {
        string.reset();
      }
      if (trigger->get() != triggerLast) {
        triggerF();
        triggerLast = trigger->get();
      }
      if (timer()) {
        // printf("%f\n", previous);
        triggerF();
      }

      left[i] = previous = string() * dbtoa(gain->get());
      right[i] = left[i];
    }
  }

  void paint (juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
  }
  /// handle doubles ? //////////////////////////////////////////////////////
  // void processBlock(AudioBuffer<double>& buffer, MidiBuffer&) override {
  //   buffer.applyGain(dbtoa((float)*gain));
  // }

  /// start and shutdown callbacks///////////////////////////////////////////
  void prepareToPlay(double, int) override {
    // XXX when does this get called? seems to not get called in stand-alone
  }
  void releaseResources() override {}

  /// maintaining persistant state on suspend ///////////////////////////////
  void getStateInformation(MemoryBlock& destData) override {
    MemoryOutputStream(destData, true).writeFloat(*gain);
  }

  void setStateInformation(const void* data, int sizeInBytes) override {
    
    gain->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
  }

  /// general configuration /////////////////////////////////////////////////
  const String getName() const override { return "Kar plus Strong"; }
  double getTailLengthSeconds() const override { return 0; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }

  /// for handling presets //////////////////////////////////////////////////
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const String getProgramName(int) override { return "None"; }
  void changeProgramName(int, const String&) override {}

  /// ?????? ////////////////////////////////////////////////////////////////
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
    const auto& mainInLayout = layouts.getChannelSet(true, 0);
    const auto& mainOutLayout = layouts.getChannelSet(false, 0);

    return (mainInLayout == mainOutLayout && (!mainInLayout.isDisabled()));
  }

  /// automagic user interface //////////////////////////////////////////////
  AudioProcessorEditor* createEditor() override {
    return new GenericAudioProcessorEditor(*this);
  }
  bool hasEditor() const override { return true; }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KarplusStrong)
};

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new KarplusStrong();
}