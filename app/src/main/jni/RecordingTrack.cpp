#include <thread>
#include "RecordingTrack.h"


#define CACHE_POINTS 2
#define INIT_SIZE 960000
#define SILENCE 0
#define START_POINT 0
#define NO_ID 255

static int bpm;
static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                                                                        void *value) {
    SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        player->setBpm(bpm);
        player->setFirstBeatMs(START_POINT);
        player->setPosition(player->firstBeatMs, false, false);
        player->cachePosition(START_POINT, NO_ID);
    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
        player->setPosition(START_POINT, false, false);
    };
}

RecordingTrack::RecordingTrack(unsigned int samplingRate, std::string filePath, double bpm) :
    recorder((filePath + "_TEMP").c_str(), samplingRate),
    recBuffer(INIT_SIZE, SILENCE),
    filePath(filePath)
{
    player = std::make_shared<SuperpoweredAdvancedAudioPlayer>(&player, playerEventCallback, samplingRate, CACHE_POINTS);
    //player->open(filePath.c_str(), start, end);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
    ::bpm = bpm;
}

void RecordingTrack::startRecord() {
    recording = true;
    recorder.start(filePath.c_str());
}
void RecordingTrack::stopRecord() {
    recorder.stop();
    recording = false;
}
void RecordingTrack::recordProcess(short int *input, int numberOfSamples, double msFromStartPoint) {
    auto startPos = calculateBufferPos(msFromStartPoint);
    for(int c = 0; c < numberOfSamples; ++c)
    {
        uint64_t i = startPos + c;
        if(i < recBuffer.size()) {
            recBuffer.at(i) = input[c];
        } else {
            recBuffer.push_back(input[c]);
        }
    }
}
void RecordingTrack::setBpm(double bpm) {
    ::bpm = bpm;
}
uint64_t RecordingTrack::calculateBufferPos(double msFromStartPoint) {
}

#undef CACHE_POINTS
#undef INIT_SIZE
#undef SILENCE
#undef START_POINT
#undef NO_ID
