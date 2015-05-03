#ifndef Header_SuperpoweredAndroidAudioIO
#define Header_SuperpoweredAndroidAudioIO

struct SuperpoweredAndroidAudioIOInternals;

/**
 @brief This is the prototype of an audio processing callback function.

 If the application requires both audio input and audio output, this callback is called once (there is no separate audio input and audio output callback). Audio input is available in audioIO, and the application should change it's contents for audio output.

 @param clientdata A custom pointer your callback receives.
 @param audioIO 16-bit stereo interleaved audio input and/or output.
 @param numberOfSamples The number of samples received and/or requested.
 @param samplerate The current sample rate in Hz.
*/
typedef bool (*audioProcessingCallback) (void *clientdata, short int *audioIO, int numberOfSamples, int samplerate);

/**
 @brief Easy handling of OpenSL ES audio input and/or output.
 */
class SuperpoweredAndroidAudioIO {
public:
/**
 @brief Creates an audio I/O instance. Audio input and/or output will not start until startOutputProcessing() and/or startInputProcessing() is called.

 @param samplerate The requested sample rate in Hz.
 @param buffersize The requested buffer size (number of samples).
 @param enableInput Enable audio input.
 @param enableOutput Enable audio output.
 @param callback The audio processing callback function to call periodically.
 @param clientdata A custom pointer the callback receives.
 @param latencySamples How many samples to have in the internal fifo buffer minimum. Works only when both input and output are enabled. Might help if you have many dropouts.
 */
    SuperpoweredAndroidAudioIO(int samplerate, int buffersize, bool enableInput, bool enableOutput, audioProcessingCallback callback, void *clientdata, int latencySamples = 0);
    ~SuperpoweredAndroidAudioIO();
/**
 @brief Starts (or resumes) output processing. Callbacks will begin after this is called. No-op is object was constructed with enableOutput == false.
 */
    void start();
/**
 @brief Starts (or resumes) input processing. Callbacks will begin after this is called. No-op is object was constructed with enableInput == false.
 */


    void pause();


private:
    SuperpoweredAndroidAudioIOInternals *internals;
    SuperpoweredAndroidAudioIO(const SuperpoweredAndroidAudioIO&);
    SuperpoweredAndroidAudioIO& operator=(const SuperpoweredAndroidAudioIO&);
};

#endif
