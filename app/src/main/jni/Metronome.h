#ifndef Header_Metronome
#define Header_Metronome


#include <string>
#include <vector>
#include <memory>
#include "SuperpoweredAdvancedAudioPlayer.h"


class Metronome {
    public:
    Metronome(std::string, int, int, int samplingRate);
    void startRecord();
    void recordBytes(std::vector<short int>);
    void play();
    void pause();
    double getCurrentBpm();
    double getMsElapsedSinceLastBeat();
    bool process(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat);
    private:
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
};

#endif