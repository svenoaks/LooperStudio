#include "Metronome.h"

#define RECORDED_BPM 120.0f
#define CACHE_POINTS 2
#define START_POINT 0
#define NO_ID 255

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                                                                 void *value) {

    SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        playerA->setBpm(RECORDED_BPM);
        playerA->setFirstBeatMs(START_POINT);
        playerA->setPosition(playerA->firstBeatMs, false, false);
        playerA->cachePosition(START_POINT, NO_ID);
    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
            playerA->setPosition(START_POINT, false, false);
    };
}

Metronome::Metronome(std::string filePath, int start, int end, int samplingRate, int bpm) : bpm(bpm)
{
    player = std::make_shared<SuperpoweredAdvancedAudioPlayer>(&player, playerEventCallback, samplingRate, CACHE_POINTS);
    player->open(filePath.c_str(), start, end);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
}

void Metronome::play() {
    player->play(false);

}
void Metronome::pause() {
    player->pause();
}
double Metronome::getCurrentBpm() {
    return player->currentBpm;
}
void Metronome::setCurrentBpm(double bpm) {
    this->bpm = bpm;
}
double Metronome::getMsElapsedSinceLastBeat() {
    return player->msElapsedSinceLastBeat;
}
bool Metronome::process(float *buffer, unsigned int numberOfSamples, float volume, double masterMsElapsedSinceLastBeat) {
    return player->process(buffer, false, numberOfSamples, volume, getCurrentBpm(), masterMsElapsedSinceLastBeat);
}
#undef RECORDED_BPM
#undef CACHE_POINTS
#undef START_POINT
#undef NO_ID