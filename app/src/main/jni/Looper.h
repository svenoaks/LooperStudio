#ifndef Header_Looper
#define Header_Looper

#include <math.h>
#include <pthread.h>
#include <queue>
#include <array>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredFilter.h"
#include "SuperpoweredRoll.h"
#include "SuperpoweredFlanger.h"
#include "SuperpoweredAndroidAudioIO.h"
#include "SuperpoweredRecorder.h"
#include "Metronome.h"
#include "RecordingTrack.h"

#define NUM_BUFFERS 2
#define HEADROOM_DECIBEL 3.0f
#define NUM_TRACKS 8

static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025);

class Looper {
public:

	Looper(const char *path, int *params, bool, double bpm=120, int measures=4, int beatsPerMeasure=4);
	~Looper();

	bool process(short int *output, unsigned int numberOfSamples);
	void setProcessThread(bool);
	void onPlayPause(bool play);
	void onStartStopRecording(bool, int);
	void onCrossfader(int value);
	void onFxSelect(int value);
	void onFxOff();
	void onFxValue(int value);
	void setBpm(double bpm);

private:
    void processLoop();

    double masterBpm;
    int measures, beatsPerMeasure;
    int samplesFromZero;

    void recordSamples(short int *, unsigned int);
    bool renderSamples(short int *, unsigned int);

    std::array<std::vector<short int>, NUM_BUFFERS> processed;

    int currentReadBuffer;
    int currentWriteBuffer;
    std::atomic_int fullBuffers;

    std::atomic_bool playing;
    std::atomic_bool recording;
    std::atomic_bool queueRecording;
    std::atomic_int currentlyRecordingTrack;

    std::atomic_bool useProcessThread;

    int buffersize;

    Metronome metronome;
    std::vector<std::shared_ptr<RecordingTrack>> tracks;

    std::mutex mutex;

    std::shared_ptr<SuperpoweredRecorder> audioRecorder;

    SuperpoweredAndroidAudioIO *audioSystem;
    SuperpoweredRoll *roll;
    SuperpoweredFilter *filter;
    SuperpoweredFlanger *flanger;
    float *stereoBuffer;
    unsigned char activeFx;
    float crossValue, volA, volB;
};

#endif