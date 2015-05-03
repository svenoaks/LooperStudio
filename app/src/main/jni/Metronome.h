#ifndef Header_Metronome
#define Header_Metronome


#include <string>
#include <vector>
#include <memory>
#include "SuperpoweredAdvancedAudioPlayer.h"


class Metronome {
    public:
    Metronome(std::string, int, int, int samplingRate, int bpm = 120);
    void startRecord();
    void recordBytes(std::vector<short int>);
    void play();
    void pause();
    double getCurrentBpm();
    void setCurrentBpm(double bpm);
    double getMsElapsedSinceLastBeat();
    bool process(float *buffer, unsigned int numberOfSamples, float volume, double masterMsElapsedSinceLastBeat);
    private:
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
    double bpm;
};

#endif