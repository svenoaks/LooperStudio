
#include "Looper.h"
#include "SuperpoweredSimple.h"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <thread>

#include <string>

#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "SOUNDPROCESS", __VA_ARGS__)

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        playerA->setBpm(126.0f);
        playerA->setFirstBeatMs(353);
        playerA->setPosition(playerA->firstBeatMs, false, false);
    };
}

static void playerEventCallbackB(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerB = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        playerB->setBpm(123.0f);
        playerB->setFirstBeatMs(40);
        playerB->setPosition(playerB->firstBeatMs, false, false);
    };
}

static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
	return ((Looper *)clientdata)->process(audioIO, numberOfSamples);
}

Looper::Looper(const char *path, int *params) : activeFx(0), crossValue(0.0f),
                                        volB(0.0f), volA(1.0f * headroom), currentReadBuffer(0), currentWriteBuffer(0),
                                        fullBuffers(0), processing(false), buffersize(params[5]) {

    pthread_mutex_init(&mutex, NULL); // This will keep our player volumes and playback states in sync.
    unsigned int samplerate = params[4];
    std::fill(processed.begin(), processed.end(), std::vector<short int>(buffersize * 2 + 16));

    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    playerA = new SuperpoweredAdvancedAudioPlayer(&playerA , playerEventCallbackA, samplerate, 0);
    playerA->open(path, params[0], params[1]);
    playerB = new SuperpoweredAdvancedAudioPlayer(&playerB, playerEventCallbackB, samplerate, 0);
    playerB->open(path, params[2], params[3]);

    playerA->syncMode = playerB->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;

    roll = new SuperpoweredRoll(samplerate);
    filter = new SuperpoweredFilter(SuperpoweredFilter_Resonant_Lowpass, samplerate);
    flanger = new SuperpoweredFlanger(samplerate);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, 0);
}

Looper::~Looper() {
    delete playerA;
    delete playerB;
    delete audioSystem;
    free(stereoBuffer);
    pthread_mutex_destroy(&mutex);
}

void Looper::onPlayPause(bool play) {
    pthread_mutex_lock(&mutex);
    if (!play) {
        playerA->pause();
        playerB->pause();
        processing = false;
    } else {
        bool masterIsA = (crossValue <= 0.5f);
        playerA->play(!masterIsA);
        playerB->play(masterIsA);
        processing = true;
        std::thread t(&Looper::processLoop, this);
        t.detach();
    };
    pthread_mutex_unlock(&mutex);
}

void Looper::onCrossfader(int value) {
    pthread_mutex_lock(&mutex);
    crossValue = float(value) * 0.01f;
    if (crossValue < 0.01f) {
        volA = 1.0f * headroom;
        volB = 0.0f;
    } else if (crossValue > 0.99f) {
        volA = 0.0f;
        volB = 1.0f * headroom;
    } else { // constant power curve
        volA = cosf(M_PI_2 * crossValue) * headroom;
        volB = cosf(M_PI_2 * (1.0f - crossValue)) * headroom;
    };
    pthread_mutex_unlock(&mutex);
}

void Looper::onFxSelect(int value) {
	__android_log_print(ANDROID_LOG_VERBOSE, "Looper", "FXSEL %i", value);
	activeFx = value;
}

void Looper::onFxOff() {
    filter->enable(false);
    roll->enable(false);
    flanger->enable(false);
}

#define MINFREQ 60.0f
#define MAXFREQ 20000.0f

static inline float floatToFrequency(float value) {
    if (value > 0.97f) return MAXFREQ;
    if (value < 0.03f) return MINFREQ;
    value = powf(10.0f, (value + ((0.4f - fabsf(value - 0.4f)) * 0.3f)) * log10f(MAXFREQ - MINFREQ)) + MINFREQ;
    return value < MAXFREQ ? value : MAXFREQ;
}

void Looper::onFxValue(int ivalue) {
    float value = float(ivalue) * 0.01f;
    switch (activeFx) {
        case 1:
            filter->setResonantParameters(floatToFrequency(1.0f - value), 0.2f);
            filter->enable(true);
            flanger->enable(false);
            roll->enable(false);
            break;
        case 2:
            if (value > 0.8f) roll->beats = 0.0625f;
            else if (value > 0.6f) roll->beats = 0.125f;
            else if (value > 0.4f) roll->beats = 0.25f;
            else if (value > 0.2f) roll->beats = 0.5f;
            else roll->beats = 1.0f;
            roll->enable(true);
            filter->enable(false);
            flanger->enable(false);
            break;
        default:
            flanger->setWet(value);
            flanger->enable(true);
            filter->enable(false);
            roll->enable(false);
    };
}

bool Looper::process(short int *output, unsigned int numberOfSamples) {

    //LOGI("IN PROCESS %d", numberOfSamples);
    bool silence = fullBuffers == 0;
    if (!silence)
    {

        auto& curBuff = processed.at(currentReadBuffer);

        copy(curBuff.begin(), curBuff.end(), output);

        if (currentReadBuffer == NUM_BUFFERS - 1)
            currentReadBuffer = 0;
        else
            ++currentReadBuffer;
         --fullBuffers;

    }
    else
    {
        LOGI("DRY!!");
    }
    //loopCv.notify_all();
    //
    return !silence;
}

void Looper::processLoop()
{
    while (processing)
    {
        //LOGI("IN loop %d", fullBuffers);
        pthread_mutex_lock(&mutex);
        if (fullBuffers < NUM_BUFFERS)
        {
            static int numberOfSamples = buffersize;
            //LOGI("sam %d %d", numberOfSamples, currentWriteBuffer);

            bool masterIsA = (crossValue <= 0.5f);
            float masterBpm = masterIsA ? playerA->currentBpm : playerB->currentBpm;
            double msElapsedSinceLastBeatA = playerA->msElapsedSinceLastBeat; // When playerB needs it, playerA has already stepped this value, so save it now.

            bool silence = !playerA->process(stereoBuffer, false, numberOfSamples, volA, masterBpm, playerB->msElapsedSinceLastBeat);
            if (playerB->process(stereoBuffer, !silence, numberOfSamples, volB, masterBpm, msElapsedSinceLastBeatA)) silence = false;

            roll->bpm = flanger->bpm = masterBpm; // Syncing fx is one line.

            if (roll->process(silence ? NULL : stereoBuffer, stereoBuffer, numberOfSamples) && silence) silence = false;
            if (!silence) {
                filter->process(stereoBuffer, stereoBuffer, numberOfSamples);
                flanger->process(stereoBuffer, stereoBuffer, numberOfSamples);
            };

            // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
            if(!silence)
            {
                //LOGI("notsilence");
                SuperpoweredFloatToShortInt(stereoBuffer, &processed[currentWriteBuffer][0], numberOfSamples);
                if (currentWriteBuffer == NUM_BUFFERS - 1)
                    currentWriteBuffer = 0;
                 else
                    ++currentWriteBuffer;
                ++fullBuffers;
            }
            pthread_mutex_unlock(&mutex);

        }
        else
        {
            pthread_mutex_unlock(&mutex);
            //LOGI("IN WAIT");
            //std::unique_lock<std::mutex> lock(loopMutex);
            //loopCv.wait(lock);
        }
        //pthread_mutex_unlock(&mutex);


    }
}

extern "C" {
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_Looper(JNIEnv *javaEnvironment, jobject self, jstring apkPath, jlongArray offsetAndLength);
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onPlayPause(JNIEnv *javaEnvironment, jobject self, jboolean play);
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onCrossfader(JNIEnv *javaEnvironment, jobject self, jint value);
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onFxSelect(JNIEnv *javaEnvironment, jobject self, jint value);
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onFxOff(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT void Java_com_superpowered_crossexample_MainActivity_onFxValue(JNIEnv *javaEnvironment, jobject self, jint value);
}

static Looper *looper = NULL;
// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
extern "C" JNIEXPORT void Java_com_smp_looperstudio_LooperActivity_Looper(JNIEnv *javaEnvironment, jobject self, jstring apkPath, jlongArray params) {
    // Convert the input jlong array to a regular int array.
    jlong *longParams = javaEnvironment->GetLongArrayElements(params, JNI_FALSE);
    int arr[6];
    for (int n = 0; n < 6; n++) arr[n] = longParams[n];
    javaEnvironment->ReleaseLongArrayElements(params, longParams, JNI_ABORT);

    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    looper = new Looper(path, arr);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);

}

extern "C" JNIEXPORT void Java_com_smp_looperstudio_LooperActivity_onPlayPause(JNIEnv *javaEnvironment, jobject self, jboolean play) {
    looper->onPlayPause(play);
}
