#include "RecordingTrack.h"

#define CACHE_POINTS 2
#define INIT_SIZE 960000

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                                                                        void *value) {

}

RecordingTrack::RecordingTrack(int samplingRate, std::string filePath) :
    player(&player, playerEventCallback, samplingRate, CACHE_POINTS),
    recorder((filePath + "_TEMP").c_str(), samplingRate),
    recBuffer(INIT_SIZE),
    filePath(filePath)
{

}

void RecordingTrack::startRecord() {
    recorder.start(filePath.c_str());
}

#undef CACHE_POINTS
#undef INIT_SIZE
