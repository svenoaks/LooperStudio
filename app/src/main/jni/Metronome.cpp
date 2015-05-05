#include "Metronome.h"

#define RECORDED_BPM 120.0f
#define CACHE_POINTS 2
#define START_POINT 0
#define NO_ID 255

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                                                                 void *value) {

    SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        player->setBpm(RECORDED_BPM);
        player->setFirstBeatMs(START_POINT);
        player->setPosition(player->firstBeatMs, false, false);
        player->cachePosition(START_POINT, NO_ID);

    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
            player->setPosition(START_POINT, false, true);
    };
}

Metronome::Metronome(std::string filePath, int start, int end, int samplingRate)
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

double Metronome::getMsElapsedSinceLastBeat() {
    return player->msElapsedSinceLastBeat;
}
bool Metronome::process(float *buffer, unsigned int numberOfSamples, float volume, double bpm, double masterMsElapsedSinceLastBeat) {
    return player->process(buffer, false, numberOfSamples, volume, bpm, masterMsElapsedSinceLastBeat);
}
#undef RECORDED_BPM
#undef CACHE_POINTS
#undef START_POINT
#undef NO_ID