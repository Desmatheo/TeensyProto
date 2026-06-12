#pragma once

#define TEENSY 

#ifndef TEENSY
#include "daisy_seed.h"
#include "daisysp.h"
#include "../include/funbox.h"

#include <q/fx/biquad.hpp>
#include "../Effect.h"
namespace q = cycfi::q;
using namespace daisy;
using namespace daisysp;
using namespace funbox; 
#else
#include <Arduino.h>
#include "AudioStream.h"
#endif

#include "Util/Multirate.h"
#include "Util/OctaveGenerator.h"

#ifndef TEENSY
class EarthEffect : public Effect {
#else
class EarthEffect : public AudioStream {
#endif

    public:

    float dryMix;
    float wetMix;


    Decimator2 decimate2;
    Interpolator interpolate;
    OctaveGenerator octave;
#ifndef TEENSY
    q::highshelf eq1;
    q::lowshelf eq2;
    Overdrive overdrive;
#endif
    float buff[6];
    float buff_out[6];
    int bin_counter = 0;

    float current_ODswell;

    int effect_mode = 0;

    bool odOn = false;
    bool bypass = false;

    // Control States
    int fs_action = 2;   // 1=OD, 2=Octave
    bool octave_only = false;
    bool momentary_active = false;

#ifndef TEENSY
    EarthEffect(float sampleRate); 
#else
    EarthEffect(); 
    virtual void update() override;
#endif

    void midiUpdate();

#ifndef TEENSY
    void update(const float** in, float** out, int idx) override;
    float updateTest(const float in, float out, int idx) override;
#endif

    void SetMix(float mix);                // Ctrl 2 (0.0 -> 1.0)
    
    void SetOctaveMode(int mode);          // 3-Way Switch 2 (0, 1, 2)
    void SetFootswitchAction(int action);  // 3-Way Switch 3 (1, 2)
    
    void SetOctaveOnlyMode(bool enable);         // Dip Switch 2
    
    void SetMomentaryAction(bool active);        // FS 2

#ifndef TEENSY
    void SetParameter(int param_id, float value) override;
#else
    void SetParameter(int param_id, float value);
#endif

#ifdef TEENSY

    // ON/OFF soft basé sur le mix
    void setEnabled(bool e)          { enabled_ = e; wetMix = enabled_ ? wetMix : 0.0f; }
    bool isEnabled() const           { return enabled_; }

private:

    bool  enabled_      = true;      // effet actif ou non
    audio_block_t* inputQueueArray_[1];
#endif
};
