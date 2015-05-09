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
#include "OnMeasureCompleteListener.h"



class RecordingTrack {
    public:
    RecordingTrack(unsigned int samplingRate, std::string filePath, double, int);
    ~RecordingTrack();
    void startRecord();
    void stopRecord();
    void recordProcess(short int *, int);
    double getMsElapsedSinceLastBeat();
    bool playProcess(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat);
    void play();
    void pause();
    double bpm;
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
    std::atomic_bool playerIsPlayable;
    private:
    std::atomic_bool recording, recInMemory;
    std::atomic_llong recordingIndex;
    void setBpm(double);

    SuperpoweredRecorder recorder;
    std::vector<short int> recBuffer;
    std::string filePath;

    float *stereoBuffer;
};

#endif