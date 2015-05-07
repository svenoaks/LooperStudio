
#include "Looper.h"
#include "SuperpoweredSimple.h"
#include "utility.h"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <thread>
#include <string>

#define LOGI(...)   __android_log_print((int)ANDROID_LOG_INFO, "SOUNDPROCESS", __VA_ARGS__)




static bool audioplaying(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
	return ((Looper *)clientdata)->process(audioIO, numberOfSamples);
}

Looper::Looper(const char *path, int *params, bool useThreading, double bpm, int measures, int beatsPerMeasure)
                                        : activeFx(0), crossValue(0.0f), volB(0.0f), volA(1.0f * headroom), currentReadBuffer(0), currentWriteBuffer(0),
                                        fullBuffers(0), playing(false), buffersize(params[5]), useProcessThread(useThreading), 
                                        recording(false), metronome(std::string(path), params[0], params[1], params[4]), masterBpm(bpm),
                                        samplesFromZero(0), queueRecording(false) {

    
    unsigned int samplerate = params[4];
    std::fill(processed.begin(), processed.end(), std::vector<short int>(buffersize * 2 + 16));
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    audioRecorder = std::make_shared<SuperpoweredRecorder>("/storage/emulated/0/Download/helltest.wav", samplerate);

    roll = new SuperpoweredRoll(samplerate);
    filter = new SuperpoweredFilter(SuperpoweredFilter_Resonant_Lowpass, samplerate);
    flanger = new SuperpoweredFlanger(samplerate);

    for (int i = 0; i < NUM_TRACKS; ++i)
    {
        tracks.push_back(std::make_shared<RecordingTrack>(samplerate, "/storage/emulated/0/Download/helltest.wav" + std::to_string(i), bpm, buffersize));
    }


    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, true, true, audioplaying, this, buffersize * 2);
    audioSystem->start();
}

Looper::~Looper() {
    delete audioSystem;
    delete roll;
    delete filter;
    delete flanger;
    free(stereoBuffer);
}

void Looper::setProcessThread(bool useThreading) {
    useProcessThread = useThreading;
}
void Looper::onStartStopRecording(bool record, int trackToRecord) {
    std::unique_lock<std::mutex> lock(mutex);
    if(!record) {
        recording = false;
        audioRecorder->stop();
    }
    else {
        LOGI("RECORDING");
        currentlyRecordingTrack = trackToRecord;
        recording = true;
        tracks.at(currentlyRecordingTrack)->startRecord();
    }
   
}
void Looper::onPlayPause(bool play) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!play) {
        metronome.pause();
        for (auto&& track : tracks) track->pause();
        playing = false;
    } else {
        bool masterIsA = (crossValue <= 0.5f);
        metronome.play();
        for (auto&& track : tracks) track->play();
        playing = true;
        if (useProcessThread)
        {
            std::thread t(&Looper::processLoop, this);
            t.detach();
        }
    };

}

void Looper::onCrossfader(int value) {
    std::unique_lock<std::mutex> lock(mutex);
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

void Looper::recordSamples(short int *output, unsigned int numberOfSamples)
{
    //LOGI("RECORDING: %d", numberOfSamples);
    //SuperpoweredShortIntToFloat(output, stereoBuffer, numberOfSamples);
    //audioRecorder->process(stereoBuffer, NULL, numberOfSamples);
    tracks.at(currentlyRecordingTrack)->recordProcess(output, numberOfSamples);
}

bool Looper::process(short int *output, unsigned int numberOfSamples) {

    //LOGI("IN PROCESS %d", numberOfSamples);
    if (recording)
    {
        recordSamples(output, numberOfSamples);
    }
    if (useProcessThread)
    {
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
        return !silence;
    }
    else
    {
        bool silence = renderSamples(output, numberOfSamples);
        return !silence;
    }

}

bool Looper::renderSamples(short int *output, unsigned int numberOfSamples)
{
    std::unique_lock<std::mutex> lock(mutex);
    bool silence = true;
    if (!useProcessThread || fullBuffers < NUM_BUFFERS)
    {
        //LOGI("sam %d %d", numberOfSamples, currentWriteBuffer);

        double msElapsedSinceLastBeatA = metronome.getMsElapsedSinceLastBeat(); // When playerB needs it, metronome has already stepped this value, so save it now.

        silence = !metronome.process(stereoBuffer, numberOfSamples, volA, masterBpm, -1.0);


        roll->bpm = flanger->bpm = masterBpm; // Syncing fx is one line.

        if (roll->process(silence ? NULL : stereoBuffer, stereoBuffer, numberOfSamples) && silence) silence = false;
        if (!silence) {
            filter->process(stereoBuffer, stereoBuffer, numberOfSamples);
            flanger->process(stereoBuffer, stereoBuffer, numberOfSamples);
        };

        // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
        if(!silence)
        {
            SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
            if (currentWriteBuffer == NUM_BUFFERS - 1)
                currentWriteBuffer = 0;
             else
                ++currentWriteBuffer;
            ++fullBuffers;
        }
    }
    return silence;

}

void Looper::processLoop()
{
    while (playing)
    {
        renderSamples(&processed[currentWriteBuffer][0], buffersize);
        //LOGI("IN loop %d", fullBuffers);

    }
}

void Looper::setBpm(double bpm)
{
    std::unique_lock<std::mutex> lock(mutex);
    masterBpm = bpm;
}



static Looper *looper = NULL;
// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
extern "C" JNIEXPORT void Java_com_smp_looperstudio_LooperActivity_Looper(JNIEnv *javaEnvironment, jobject self, jstring apkPath, jlongArray params, jboolean useThreading) {
    // Convert the input jlong array to a regular int array.
    jlong *longParams = javaEnvironment->GetLongArrayElements(params, JNI_FALSE);
    int arr[6];
    for (int n = 0; n < 6; n++) arr[n] = longParams[n];
    javaEnvironment->ReleaseLongArrayElements(params, longParams, JNI_ABORT);

    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    looper = new Looper(path, arr, useThreading);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);

}

extern "C" JNIEXPORT void Java_com_smp_looperstudio_LooperActivity_onPlayPause(JNIEnv *javaEnvironment, jobject self, jboolean play) {
    looper->onPlayPause(play);
}

extern "C" JNIEXPORT void Java_com_smp_looperstudio_LooperActivity_onRecord(JNIEnv *javaEnvironment, jobject self, jboolean record) {
    looper->onStartStopRecording(record, 0); //TODO Select Track
}

#undef NUM_BUFFERS
#undef HEADROOM_DECIBEL
#undef NUM_TRACKS