#pragma once

#include <Arduino.h>
#include "AudioStream.h"

// Inclusions des algorithmes DSP (à copier dans ton projet)
#include "../src/EffetEarthTeensy/Util/Multirate.h"
#include "../src/EffetEarthTeensy/Util/OctaveGenerator.h"

class AudioEffectOctaver : public AudioStream {
public:
    AudioEffectOctaver();
    virtual ~AudioEffectOctaver() = default;

    // Méthode principale de traitement audio de Teensy
    virtual void update() override;

    // Paramètres
    void setOctaveMode(int mode); // 1 = Up1, 2 = Down1, 3 = Down2
    void setMix(float mix);
    void setEnabled(bool enable);

private:
    audio_block_t* inputQueueArray_[1];

    // Classes DSP (identiques à la version Daisy)
    Decimator2 decimate2_;
    Interpolator interpolate_;
    OctaveGenerator octave_;

    // Buffers pour le multirate
    float buff_[6];
    float buff_out_[6];
    int bin_counter_ = 0;

    // Paramètres utilisateur
    int effect_mode_ = 1;
    float mix_ = 0.5f;
    float dryMix_ = 0.5f;
    float wetMix_ = 0.5f;
    bool enabled_ = true;
};