#ifndef Header_Metronome
#define Header_Metronome


#include <string>
#include <vector>
#include <memory>
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "OnMeasureCompleteListener.h"


class Metronome {
    public:
    Metronome(std::string, int, int, int samplingRate, std::shared_ptr<OnMeasureCompleteListener>);
    void startRecord();
    void recordBytes(std::vector<short int>);
    void play();
    void pause();
    double getCurrentBpm();
    double getMsElapsedSinceLastBeat();
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
    std::shared_ptr<OnMeasureCompleteListener> listener;
    bool process(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat);
    private:

};

#endif