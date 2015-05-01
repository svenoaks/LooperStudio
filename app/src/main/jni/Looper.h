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

#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredFilter.h"
#include "SuperpoweredRoll.h"
#include "SuperpoweredFlanger.h"
#include "SuperpoweredAndroidAudioIO.h"

#define NUM_BUFFERS 2
#define HEADROOM_DECIBEL 3.0f
static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025);

class Looper {
public:

	Looper(const char *path, int *params);
	~Looper();

	bool process(short int *output, unsigned int numberOfSamples);
	void onPlayPause(bool play);
	void onCrossfader(int value);
	void onFxSelect(int value);
	void onFxOff();
	void onFxValue(int value);

private:
    void processLoop();

    std::array<std::vector<short int>, NUM_BUFFERS> processed;
    int currentReadBuffer;
    int currentWriteBuffer;
    std::atomic_int fullBuffers;
    bool processing;
    std::atomic_int buffersize;

    std::mutex loopMutex;
    std::condition_variable loopCv;

    pthread_mutex_t mutex;
    SuperpoweredAndroidAudioIO *audioSystem;
    SuperpoweredAdvancedAudioPlayer *playerA, *playerB;
    SuperpoweredRoll *roll;
    SuperpoweredFilter *filter;
    SuperpoweredFlanger *flanger;
    float *stereoBuffer;
    unsigned char activeFx;
    float crossValue, volA, volB;
};

#endif