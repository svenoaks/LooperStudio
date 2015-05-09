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
    RecordingTrack& track = *((RecordingTrack*)clientData);
    auto player = track.player;
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        player->setBpm(bpm);
        player->setFirstBeatMs(START_POINT);
        player->setPosition(player->firstBeatMs, false, false);
        player->cachePosition(START_POINT, NO_ID);
        player->play(true);
        LOGI("LOADED!");
    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
        player->setPosition(START_POINT, false, false);
    };
}

RecordingTrack::RecordingTrack(unsigned int samplingRate, std::string filePath, double bpm, int buffersize)
    : recorder((filePath + "_TEMP").c_str(), samplingRate),
    recBuffer(INIT_SIZE, SILENCE),
    filePath(filePath),
    recordingIndex(0),
    recInMemory(false),
    playerIsPlayable(false),
    bpm(bpm)
{
    player = std::make_shared<SuperpoweredAdvancedAudioPlayer>(this, playerEventCallback, samplingRate, CACHE_POINTS);
    //player->open(filePath.c_str(), start, end);

    player->open((filePath + std::string(".wav")).c_str());
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
    LOGI("%s", filePath.c_str());
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);
}
RecordingTrack::~RecordingTrack() {
    free(stereoBuffer);
}

void RecordingTrack::startRecord() {
    recording = true;
    recInMemory = true;
    recorder.start(filePath.c_str());
}
void RecordingTrack::stopRecord() {
    recorder.stop();
    recording = false;
    recInMemory = false;
    recordingIndex = 0;
    //player->open((filePath + std::string(".wav")).c_str());
    //player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
}
void RecordingTrack::recordProcess(short int *input, int numberOfSamples) {
    for(int c = 0; c < numberOfSamples; ++c)
    {
        if(recordingIndex < recBuffer.size()) {
            recBuffer.at(recordingIndex++) = input[c];
        } else {
            recBuffer.push_back(input[c]);
            ++recordingIndex;
        }
    }
    SuperpoweredShortIntToFloat(input, stereoBuffer, numberOfSamples);
    recorder.process(stereoBuffer, NULL, numberOfSamples);
}
void RecordingTrack::setBpm(double bpm) {
    this->bpm = bpm;
}
double RecordingTrack::getMsElapsedSinceLastBeat() {
    return player->msElapsedSinceLastBeat;
}

void RecordingTrack::pause() {
}
void RecordingTrack::play() {
    player->play(true);
}

bool RecordingTrack::playProcess(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat) {
    if (recInMemory) {
        SuperpoweredShortIntToFloat(&recBuffer[0] + recordingIndex, buffer, numberOfSamples);
        recordingIndex += numberOfSamples;
        return false;
    } else {
        player->process(buffer, true, numberOfSamples, volume, bpm, masterMsElapsedSinceLastBeat);
        return false;
    }
}

#undef CACHE_POINTS
#undef INIT_SIZE
#undef SILENCE
#undef START_POINT
#undef NO_ID
