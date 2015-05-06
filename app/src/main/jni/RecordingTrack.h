#ifndef Header_RecordingTrack
#define Header_RecordingTrack


#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredRecorder.h"



class RecordingTrack {
    public:
    RecordingTrack(unsigned int samplingRate, std::string filePath, double);
    void startRecord();
    void stopRecord();
    void recordProcess(short int *, int, double);
    void play();
    void pause();
    private:
    std::atomic_bool recording;

    void setBpm(double);
    uint64_t calculateBufferPos(double);
    std::shared_ptr<SuperpoweredAdvancedAudioPlayer> player;
    SuperpoweredRecorder recorder;
    std::vector<short int> recBuffer;
    std::string filePath;
};

#endif