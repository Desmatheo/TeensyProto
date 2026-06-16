#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include "AudioEffectDelayMod.h"
#include "AudioEffectDrive.h"
#include "AudioEffectRev.h"
#include "EffetEarthTeensy/Earth.h"
#include "EffetDelayTeensy/Delay.h"

#define MODE_HEXAPHONIQUE

#ifdef MODE_HEXAPHONIQUE

    // --- CONFIGURATION ---
    #define InputTDM 1    // 1: Entrée Guitare (TDM), 0: Séquenceur d'Oscillateurs
    #if InputTDM
    #define OutputTDM 1   // 1: Sortie Jack CS42448 (TDM), 0: Désactivé
    #endif
    #define OutputUSB 1   // 1: Sortie Casque/PC (USB), 0: Désactivé
    #define DelayPaulo 0

    // -----------------------------------------

    void OnControlChange(byte channel, byte control, byte value);

    #pragma region Objet audios
    #if !InputTDM
    AudioSynthWaveform       mesOscs[6];       // combinaison d'oscillateurs
    #endif

    AudioEffectDrive         mesDistos[6];     // reverb, distos et delays pour chaque corde
    #if DelayPaulo
    AudioEffectDelayMod      mesDelays[6];     
    #else 
    DelayEffect              mesDelays[6];
    #endif
    AudioEffectRev           mesReverbs[6];    
    EarthEffect              EffetEarth[6];
    

    // Comme la reverb est stéréo, il faut 2 canaux de mixage
    AudioMixer4              mixerL_1a4;   
    AudioMixer4              mixerL_5et6;  
    AudioMixer4              masterL;      

    // --- Analyseurs de pic pour le signal des cordes ---
    AudioAnalyzePeak         stringPeaks[6];
    // -------------------------------------------------------------

    AudioMixer4              mixerR_1a4;   
    AudioMixer4              mixerR_5et6;  
    AudioMixer4              masterR;      

    #if OutputUSB
    AudioOutputUSB           usbOut;           // Sortie audio de la teensy
    #endif

    #if InputTDM || OutputTDM
    DMAMEM AudioControlCS42448 cs42448_1;      // Contrôleur matériel CS42448
    #endif
    
    #if InputTDM
    AudioInputTDM            inputTDM;         // Entrée TDM depuis la guitare
    #endif

    #if InputTDM || OutputTDM
    AudioOutputTDM           outputTDM;       
    #endif

    #pragma endregion

    #pragma region Connexions audio 
    //Source, Port de Sortie, Desrtination, Port d'Entrée)
    
    #if InputTDM == 0
    AudioConnection p_osc_dist0(mesOscs[0], 0, mesDistos[0], 0);
    AudioConnection p_osc_dist1(mesOscs[1], 0, mesDistos[1], 0);
    AudioConnection p_osc_dist2(mesOscs[2], 0, mesDistos[2], 0);
    AudioConnection p_osc_dist3(mesOscs[3], 0, mesDistos[3], 0);
    AudioConnection p_osc_dist4(mesOscs[4], 0, mesDistos[4], 0);
    AudioConnection p_osc_dist5(mesOscs[5], 0, mesDistos[5], 0);
    #else
    AudioConnection p_tdm_dist0(inputTDM, 10, EffetEarth[0], 0);
    AudioConnection p_tdm_dist1(inputTDM, 8,  EffetEarth[1], 0);
    AudioConnection p_tdm_dist2(inputTDM, 6,  EffetEarth[2], 0);
    AudioConnection p_tdm_dist3(inputTDM, 4,  EffetEarth[3], 0);
    AudioConnection p_tdm_dist4(inputTDM, 2,  EffetEarth[4], 0);
    AudioConnection p_tdm_dist5(inputTDM, 0,  EffetEarth[5], 0);
    #endif


    #if !SoloEffect
    // AudioConnection p_dist_earth0(mesDistos[0], 0, EffetEarth[0], 0);
    // AudioConnection p_dist_earth1(mesDistos[1], 0, EffetEarth[1], 0);
    // AudioConnection p_dist_earth2(mesDistos[2], 0, EffetEarth[2], 0);
    // AudioConnection p_dist_earth3(mesDistos[3], 0, EffetEarth[3], 0);
    // AudioConnection p_dist_earth4(mesDistos[4], 0, EffetEarth[4], 0);
    // AudioConnection p_dist_earth5(mesDistos[5], 0, EffetEarth[5], 0);

    AudioConnection p_earth_dly0(EffetEarth[0], 0, mesDelays[0], 0);
    AudioConnection p_earth_dly1(EffetEarth[1], 0, mesDelays[1], 0);
    AudioConnection p_earth_dly2(EffetEarth[2], 0, mesDelays[2], 0);
    AudioConnection p_earth_dly3(EffetEarth[3], 0, mesDelays[3], 0);
    AudioConnection p_earth_dly4(EffetEarth[4], 0, mesDelays[4], 0);
    AudioConnection p_earth_dly5(EffetEarth[5], 0, mesDelays[5], 0);

    AudioConnection p_dly_rev0(mesDelays[0], 0, mixerR_1a4, 0);
    AudioConnection p_dly_rev1(mesDelays[1], 0, mixerR_1a4, 1);
    AudioConnection p_dly_rev2(mesDelays[2], 0, mixerR_1a4, 2);
    AudioConnection p_dly_rev3(mesDelays[3], 0, mixerR_1a4, 3);
    AudioConnection p_dly_rev4(mesDelays[4], 0, mixerR_5et6, 0);
    AudioConnection p_dly_rev5(mesDelays[5], 0, mixerR_5et6, 1);

    /*
    // Reverb gauche (entrée 0) dans le mixer Mixers gauche
    AudioConnection p_rev0_L(mesReverbs[0], 0, mixerL_1a4, 0);
    AudioConnection p_rev1_L(mesReverbs[1], 0, mixerL_1a4, 1);
    AudioConnection p_rev2_L(mesReverbs[2], 0, mixerL_1a4, 2);
    AudioConnection p_rev3_L(mesReverbs[3], 0, mixerL_1a4, 3);
    AudioConnection p_rev4_L(mesReverbs[4], 0, mixerL_5et6, 0);
    AudioConnection p_rev5_L(mesReverbs[5], 0, mixerL_5et6, 1);

    // Reverb droite (entrée 1) dans le mixer Mixers droit
    AudioConnection p_rev0_R(mesReverbs[0], 1, mixerR_1a4, 0);
    AudioConnection p_rev1_R(mesReverbs[1], 1, mixerR_1a4, 1);
    AudioConnection p_rev2_R(mesReverbs[2], 1, mixerR_1a4, 2);
    AudioConnection p_rev3_R(mesReverbs[3], 1, mixerR_1a4, 3);
    AudioConnection p_rev4_R(mesReverbs[4], 1, mixerR_5et6, 0);
    AudioConnection p_rev5_R(mesReverbs[5], 1, mixerR_5et6, 1);
    // // ------------------------------------------------------------------ */

    #else
    // #if !InputTDM
    // AudioConnection p_in_earth0(mesOscs[0], 0, EffetEarth[0], 0);
    // AudioConnection p_in_earth1(mesOscs[1], 0, EffetEarth[1], 0);
    // AudioConnection p_in_earth2(mesOscs[2], 0, EffetEarth[2], 0);
    // AudioConnection p_in_earth3(mesOscs[3], 0, EffetEarth[3], 0);
    // AudioConnection p_in_earth4(mesOscs[4], 0, EffetEarth[4], 0);
    // AudioConnection p_in_earth5(mesOscs[5], 0, EffetEarth[5], 0);
    
    // AudioConnection p_in_peak0(mesOscs[0], 0, stringPeaks[0], 0);
    // AudioConnection p_in_peak1(mesOscs[1], 0, stringPeaks[1], 0);
    // AudioConnection p_in_peak2(mesOscs[2], 0, stringPeaks[2], 0);
    // AudioConnection p_in_peak3(mesOscs[3], 0, stringPeaks[3], 0);
    // AudioConnection p_in_peak4(mesOscs[4], 0, stringPeaks[4], 0);
    // AudioConnection p_in_peak5(mesOscs[5], 0, stringPeaks[5], 0);
    // #else
    // AudioConnection p_in_earth0(inputTDM, 10, EffetEarth[0], 0);
    // AudioConnection p_in_earth1(inputTDM, 8,  EffetEarth[1], 0);
    // AudioConnection p_in_earth2(inputTDM, 6,  EffetEarth[2], 0);
    // AudioConnection p_in_earth3(inputTDM, 4,  EffetEarth[3], 0);
    // AudioConnection p_in_earth4(inputTDM, 2,  EffetEarth[4], 0);
    // AudioConnection p_in_earth5(inputTDM, 0,  EffetEarth[5], 0);

    // AudioConnection p_in_peak0(inputTDM, 10, stringPeaks[0], 0);
    // AudioConnection p_in_peak1(inputTDM, 8,  stringPeaks[1], 0);
    // AudioConnection p_in_peak2(inputTDM, 6,  stringPeaks[2], 0);
    // AudioConnection p_in_peak3(inputTDM, 4,  stringPeaks[3], 0);
    // AudioConnection p_in_peak4(inputTDM, 2,  stringPeaks[4], 0);
    // AudioConnection p_in_peak5(inputTDM, 0,  stringPeaks[5], 0);
    // #endif

    // // Earth -> Mixers L & R (On duplique le signal mono d'Earth sur les bus L et R)
    // AudioConnection p_earth_mixL0(EffetEarth[0], 0, mixerL_1a4, 0);   AudioConnection p_earth_mixR0(EffetEarth[0], 0, mixerR_1a4, 0);
    // AudioConnection p_earth_mixL1(EffetEarth[1], 0, mixerL_1a4, 1);   AudioConnection p_earth_mixR1(EffetEarth[1], 0, mixerR_1a4, 1);
    // AudioConnection p_earth_mixL2(EffetEarth[2], 0, mixerL_1a4, 2);   AudioConnection p_earth_mixR2(EffetEarth[2], 0, mixerR_1a4, 2);
    // AudioConnection p_earth_mixL3(EffetEarth[3], 0, mixerL_1a4, 3);   AudioConnection p_earth_mixR3(EffetEarth[3], 0, mixerR_1a4, 3);
    // AudioConnection p_earth_mixL4(EffetEarth[4], 0, mixerL_5et6, 0);  AudioConnection p_earth_mixR4(EffetEarth[4], 0, mixerR_5et6, 0);
    // AudioConnection p_earth_mixL5(EffetEarth[5], 0, mixerL_5et6, 1);  AudioConnection p_earth_mixR5(EffetEarth[5], 0, mixerR_5et6, 1);
    // // -----------------------------------------------------------------
    #endif


    // Mixers sous-groupes dans le Masters
    AudioConnection p_mastL1(mixerL_1a4, 0, masterL, 0);
    AudioConnection p_mastL2(mixerL_5et6, 0, masterL, 1);
    AudioConnection p_mastR1(mixerR_1a4, 0, masterR, 0);
    AudioConnection p_mastR2(mixerR_5et6, 0, masterR, 1);

    // Masters -> Sorties
    #if OutputUSB
    AudioConnection p_outL_usb(masterL, 0, usbOut, 0);
    AudioConnection p_outR_usb(masterR, 0, usbOut, 1);
    #endif
    #if OutputTDM
    AudioConnection p_outL_tdm(masterL, 0, outputTDM, 14); // Sortie Analogique Gauche
    AudioConnection p_outR_tdm(masterR, 0, outputTDM, 12); // Sortie Analogique Droite
    #endif
    #pragma endregion

    
    unsigned long tempsDerniereNote = 0;
    int cordeCourante = 0;
    float volumesCordes[6] = {0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f, 0.00001f};
    const float frequencesGuitare[6] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};
    
    // --- NOUVELLES VARIABLES MUTE ET BYPASS ---
    bool Bypass = false;
    int effetActif[6] = {0, 0, 0, 0, 0, 0}; // 0 = Aucun, 1 = Delay, 2 = Disto, 3 = Earth
    bool stringBypass[6] = {false, false, false, false, false, false};
    bool globalBypassState = false;

    const int reset_p = 2; 

    void setup() {
        pinMode(13, OUTPUT); // NOUVEAU : LED de statut MIDI
        Serial.begin(115200);

        AudioMemory(1500); // on alloue une mémoire suffisante 

        #if InputTDM || OutputTDM
        
        //CTRL_UART.begin(115200);
        
        pinMode(reset_p, OUTPUT);                                    
        //Power-Up Sequence
        digitalWrite(reset_p, LOW);
        delay(800);
        digitalWrite(reset_p, HIGH);

        if (cs42448_1.enable()) {
            Serial.println("configured CS42448");
        } else {
            Serial.println("failed to config CS42448");
        }
        
        cs42448_1.inputLevel(1);
        cs42448_1.volume(1.0);



        #endif

        #pragma region Initialisation des effets et oscillateurs
        for (int i = 0; i < 6; i++) {
            #if InputTDM == 0
            mesOscs[i].begin(WAVEFORM_TRIANGLE);
            mesOscs[i].amplitude(volumesCordes[i]);
            mesOscs[i].frequency(frequencesGuitare[i]);
            #endif
            
            // Initialisation de la Disto
            mesDistos[i].begin(2048);
            mesDistos[i].setMix(0.0f); // Par défaut bypass
            
            // Initialisation de chaque delay
            #if DelayPaulo
            mesDelays[i].begin(800);
            #else
            mesDelays[i].begin();
            #endif
            mesDelays[i].setMix(0.0f); // Par défaut bypass

            // Initialisation de la Reverb
            EffetEarth[i].setMix(0.0f);
            // --------------------------------------------------------- 
        }
        #pragma endregion

        usbMIDI.setHandleControlChange(OnControlChange); //Active la fonction OnControlChange() des qu'il y a un CC 
    }

    void loop() {
        unsigned long tempsActuel = millis(); // temps actuel en ms depuis le démarrage du programme
        static unsigned long lastHeartbeat = 0;

        // Heartbeat : Affichage régulier des diagnostics
        // NOUVEAU : Passé de 5ms à 500ms. 5ms saturait le port USB Série et bloquait complètement la Teensy !
        if (tempsActuel - lastHeartbeat >= 500) { 
            lastHeartbeat = tempsActuel;
            Serial.print("Charge CPU Audio Actuelle : ");
            Serial.print(AudioProcessorUsage());
            Serial.println(" %");

            Serial.print("Charge CPU Audio Max : ");
            Serial.print(AudioProcessorUsageMax());
            Serial.println(" %");

                
            Serial.print("Delay actif Premiere corde : ");
            Serial.print(mesDelays[0].isEnabled());
            Serial.print(" Delay Mix Premiere corde : ");
            Serial.println(mesDelays[0].getMix());
            
            digitalWrite(13, !digitalRead(13)); // Clignotement lent
        }

        while (usbMIDI.read()) {
            // On ignore les messages de synchronisation (Clock) et ActiveSensing 
            // qui spamment le port série des centaines de fois par seconde et bloquent la Teensy
            /*if (usbMIDI.getType() != usbMIDI.Clock && usbMIDI.getType() != usbMIDI.ActiveSensing) {
                Serial.print(">>> MSG MIDI BRUT Recu ! Type : ");
                Serial.print(usbMIDI.getType());
                Serial.print(" | Data 1 : ");
                Serial.print(usbMIDI.getData1());
                Serial.print(" | Data 2 : ");
                Serial.println(usbMIDI.getData2());
            }*/
        }



#if !InputTDM
        if (tempsActuel - tempsDerniereNote >= 400) { // on joue une corde toutes les 400 ms
            tempsDerniereNote = tempsActuel;  
            if (!cordeMute[cordeCourante]) {         // On ne joue la corde QUE si elle n'est pas muette
                volumesCordes[cordeCourante] = 0.3f; 
                mesOscs[cordeCourante].frequency(frequencesGuitare[cordeCourante]);
            }
            cordeCourante = (cordeCourante + 1) % 6; // Modulo 6 pour tourner en boucle
        }
        

        // Gestion du Decay (extinction du son) indépendant pour chaque corde
        for (int i = 0; i < 6; i++) {
            if (volumesCordes[i] > 0.00001f) {
                volumesCordes[i] *= 0.99f; // Extinction lente pour laisser résonner les notes ensemble
                
                if (volumesCordes[i] < 0.00001f) volumesCordes[i] = 0.00001f;
                mesOscs[i].amplitude(volumesCordes[i]);
            }
        }
#endif

        delay(2);

    }

    void OnControlChange(byte channel, byte control, byte value) {
        float valNorm = value / 127.0f;

        // --- TRANCHE 1 : DELAY (CC 10 à 45) ---
        if (control >= 10 && control <= 45) {
            int ccRelatif = control - 10;
            int corde = ccRelatif / 6;  // Division entière -> Donne la corde (0 à 5)
            int potard = ccRelatif % 6; // Reste -> Donne le bouton (0 à 5)

            if (corde >= 0 && corde < 6) {
                effetActif[corde] = 1; // Mémorise que Delay est l'effet de cette corde
                if (!stringBypass[corde] && !globalBypassState) {
                    mesDelays[corde].setEnabled(true);
                }
                // Affichage mouchard dans la console VS Code
                Serial.print("MIDI -> Effet: DELAY | Corde: ");
                Serial.print(corde);
                Serial.print(" | Potard: P");
                Serial.print(potard + 1);
                Serial.print(" | Valeur: ");
                Serial.println(value);

                mesDelays[corde].setParameter(potard, valNorm);

                // // Application du paramètre sur la corde ciblée uniquement
                // if (potard == 0) mesDelays[corde].setTimeMs(valNorm * 750.0f);
                // if (potard == 1) mesDelays[corde].setFeedback(valNorm * 0.95f);
                // if (potard == 2) mesDelays[corde].setMix(valNorm);
                // if (potard == 3) mesDelays[corde].setModRate(valNorm * 5.0f);
                // if (potard == 4) mesDelays[corde].setModDepthMs(valNorm * 10.0f);
            }
            EffetEarth[corde].setEnabled(false);
            mesDistos[corde].setEnabled(false);
        }
        
        // --- TRANCHE 2 : DISTORTION (CC 50 à 85) ---
        else if (control >= 50 && control <= 85) { 
            int ccRelatif = control - 50;
            int corde = ccRelatif / 6;
            int potard = ccRelatif % 6;

            if (corde >= 0 && corde < 6) {
                effetActif[corde] = 2; // Mémorise que Disto est l'effet de cette corde
                if (!stringBypass[corde] && !globalBypassState) {
                    mesDistos[corde].setEnabled(true);
                }   
                // Affichage mouchard dans la console VS Code
                Serial.print("MIDI -> Effet: DELAY | Corde: ");
                Serial.print(corde);
                Serial.print(" | Potard: P");
                Serial.print(potard + 1);
                Serial.print(" | Valeur: ");
                Serial.println(value);

                
                if (potard == 0) mesDistos[corde].setDriveDb(valNorm * 30.0f);            // Gain
                if (potard == 1) mesDistos[corde].setToneHz(800.0f + valNorm * 7200.0f);  // Tone
                if (potard == 2) mesDistos[corde].setMix(valNorm);                        // Mix
            }
            EffetEarth[corde].setEnabled(false);
            mesDelays[corde].setEnabled(false);
        }

        // --- TRANCHE 3 : EARTH (CC 90 à 125 - Remplace la Reverb) ---
        else if (control >= 90 && control <= 125) {
            int ccRelatif = control - 90;
            int corde = ccRelatif / 6;
            int potard = ccRelatif % 6;


            if (corde >= 0 && corde < 6) {
                effetActif[corde] = 3; // Mémorise que Earth est l'effet de cette corde
                if (!stringBypass[corde] && !globalBypassState) {
                    EffetEarth[corde].setEnabled(true);
                }
                // Affichage mouchard dans la console VS Code
                Serial.print("MIDI -> Effet: DELAY | Corde: ");
                Serial.print(corde);
                Serial.print(" | Potard: P");
                Serial.print(potard + 1);
                Serial.print(" | Valeur: ");
                Serial.print(value);
                Serial.print(" | Valeur Normalisé: ");
                Serial.println(valNorm);

                EffetEarth[corde].setParameter(potard, valNorm);
            }
            mesDistos[corde].setEnabled(false);
            mesDelays[corde].setEnabled(false);
        }

        // --- TRANCHE 4 : BYPASS DES CORDES INDIVIDUELLES (CC 0 à 5) ---
        else if (control >= 0 && control <= 5) {
             // La valeur > 63 suppose que 127 = Bypass ON (son coupé) et 0 = Bypass OFF.
            // (Si ton bouton envoie l'inverse pour "Allumer l'effet", remplace par "value < 64")
            bool isBypassed = (value > 63); 
            stringBypass[control] = isBypassed;
            
            if (isBypassed || globalBypassState) {
                mesDistos[control].setEnabled(false);
                mesDelays[control].setEnabled(false);
                EffetEarth[control].setEnabled(false);
            } else {
                // Si on sort du bypass, on réactive le dernier effet utilisé
                if (effetActif[control] == 1) mesDelays[control].setEnabled(true);
                else if (effetActif[control] == 2) mesDistos[control].setEnabled(true);
                else if (effetActif[control] == 3) EffetEarth[control].setEnabled(true);
            }
        }
        
        // --- TRANCHE 5 : BYPASS GLOBAL (CC 126) ---
        else if (control == 126) {
            globalBypassState = (value > 63);
            for (int i = 0; i < 6; i++) {
                if (globalBypassState || stringBypass[i]) {
                    mesDistos[i].setEnabled(false);
                    mesDelays[i].setEnabled(false);
                    EffetEarth[i].setEnabled(false);
                } else {
                    if (effetActif[i] == 1) mesDelays[i].setEnabled(true);
                    else if (effetActif[i] == 2) mesDistos[i].setEnabled(true);
                    else if (effetActif[i] == 3) EffetEarth[i].setEnabled(true);
                }

            }
        }
    }
#else

    void OnControlChange(byte channel, byte control, byte value);

    #pragma region Objet audios
    AudioSynthWaveform       monOscillateur; 
    AudioEffectDrive         maDisto;
    AudioEffectDelayMod      monDelay;       
    AudioEffectRev           maReverb;
    AudioOutputUSB           usbOut;         

    AudioConnection          patch1(monOscillateur, 0, maDisto, 0); 
    AudioConnection          patch2(maDisto, 0, monDelay, 0); 
    AudioConnection          patch3(monDelay, 0, maReverb, 0); 
    AudioConnection          patch4(maReverb, 0, usbOut, 0); 
    AudioConnection          patch5(maReverb, 1, usbOut, 1); // La reverb sort en stéréo
    #pragma endregion

    unsigned long tempsDerniereNote = 0;
    float volumeCourant = 0.00001f;
    int indexNote = 0;

    const float notes[] = {164.81, 220.00, 293.66, 392.00}; 

    bool isMuted = false;
    bool globalBypass = false;

    void setup() {
        pinMode(13, OUTPUT); // NOUVEAU : LED de statut MIDI
        Serial.begin(9600);
        AudioMemory(40); 

        monOscillateur.begin(WAVEFORM_SINE);
        monOscillateur.amplitude(volumeCourant);
        monOscillateur.frequency(notes[0]);

        // Initialisation des effets
        maDisto.begin(2048);
        maDisto.setMix(0.0f); 

        monDelay.begin(800);
        monDelay.setMix(0.0f); 

        maReverb.begin();
        maReverb.setMix(0.0f); 

        usbMIDI.setHandleControlChange(OnControlChange);
    }

    void loop() {
        unsigned long tempsActuel = millis();
        static unsigned long lastHeartbeat = 0;

        if (tempsActuel - lastHeartbeat >= 1000) {
            lastHeartbeat = tempsActuel;
            Serial.println("[TEENSY] 🟢 En attente de MIDI via USB...");
            digitalWrite(13, !digitalRead(13)); // Clignotement lent
        }

        while (usbMIDI.read()) {
            Serial.print(">>> MSG MIDI BRUT Recu ! Type : ");
            Serial.print(usbMIDI.getType());
            Serial.print(" | Data 1 : ");
            Serial.print(usbMIDI.getData1());
            Serial.print(" | Data 2 : ");
            Serial.println(usbMIDI.getData2());
        }

    
        if (tempsActuel - tempsDerniereNote >= 1200) {
            tempsDerniereNote = tempsActuel;
            
        
            monOscillateur.frequency(notes[indexNote]);
            indexNote = (indexNote + 1) % 4; // Boucle de 0 à 3
            
            if (!isMuted) {
                volumeCourant = 0.4f; 
            }
        }

        
        if (volumeCourant > 0.00001f) {
            // On diminue le volume de 2% à chaque passage dans la loop
            volumeCourant *= 0.98f; 
            
        
            if (volumeCourant < 0.00001f) volumeCourant = 0.00001f;
            
            monOscillateur.amplitude(volumeCourant);
        }
        
        
        delay(2); 
    }

    void OnControlChange(byte channel, byte control, byte value) {
        float valNorm = value / 127.0f;

        
        if (control >= 10 && control <= 15) {
            if (control == 10) monDelay.setTimeMs(valNorm * 750.0f); // 
            if (control == 11) monDelay.setFeedback(valNorm * 0.95f); // 
            if (control == 12) monDelay.setMix(valNorm);
            if (control == 13) monDelay.setModRate(valNorm * 5.0f);
            if (control == 14) monDelay.setModDepthMs(valNorm * 10.0f);
        }
        
        else if (control >= 50 && control <= 55) {
            if (control == 50) maDisto.setDriveDb(valNorm * 30.0f);            
            if (control == 51) maDisto.setToneHz(800.0f + valNorm * 7200.0f);  
            if (control == 52) maDisto.setMix(valNorm);                       
        }
        
        else if (control >= 90 && control <= 95) {
            if (control == 90) maReverb.setRoomSize(valNorm);
            if (control == 91) maReverb.setDamping(valNorm);
            if (control == 92) maReverb.setMix(valNorm);
        }
    }


#endif
