#ifndef Header_RecordingTrack
#define Header_RecordingTrack


#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredRecorder.h"
#include "utility.h"
#include "SuperpoweredSimple.h"



class RecordingTrack {
    public:
    RecordingTrack(unsigned int samplingRate, std::string filePath, double, int);
    ~RecordingTrack();
    void startRecord();
    void stopRecord();
    void recordProcess(short int *, int);
    void playProcess(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat);
    void play();
    void pause();
    private:
    std::atomic_bool recording, recInMemory;
    std::atomic_llong recordingIndex;

    void setBpm(double);
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
    SuperpoweredRecorder recorder;
    std::vector<short int> recBuffer;
    std::string filePath;

    float *stereoBuffer;
};

#endif