#ifndef Header_RecordingTrack
#define Header_RecordingTrack


#include <string>
#include <vector>
#include <memory>
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredRecorder.h"



class RecordingTrack {
    public:
    RecordingTrack(int samplingRate, std::string filePath);
    void startRecord();
    void recordBytes(std::vector<short int>);
    private:
    SuperpoweredAdvancedAudioPlayer player;
    SuperpoweredRecorder recorder;
    std::vector<short int> recBuffer;
    std::string filePath;
};

#endif