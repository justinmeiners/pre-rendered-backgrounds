
#include "snd_driver_core_audio.h"

#include <stdlib.h>
#include <AudioToolbox/AudioToolbox.h>

#define NUM_BUFFERS 3

#define BYTES_PER_SAMPLE 2
#define AUDIO_BUFFERS 3

#define CHANNEL_COUNT 2


struct CoreAudio_SndDriverImpl
{
    int frameCount;

    AudioQueueRef queue;
    AudioQueueBufferRef buffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription dataFormat;
};

static void CoreAudioCallback(void* userData,
                               AudioQueueRef inQ,
                               AudioQueueBufferRef outQB)
{

    if (!userData) {
        return;
    }
    
    SndDriver* driver = userData;
    struct CoreAudio_SndDriverImpl* impl = driver->impl;

    short* coreAudioBuffer = (short*)outQB->mAudioData;
        
    if (!driver->running)
    {
        AudioQueueStop(impl->queue, 1);
        return;
    }
    
    if (impl->frameCount > 0)
    {
        outQB->mAudioDataByteSize = 4 * impl->frameCount;
        
        float left, right;
        for (int i = 0; i < impl->frameCount * 2; i += 2)
        {
            driver->callback(driver->delegate, driver, &left, &right);
            coreAudioBuffer[i] = (left * SHRT_MAX);
            coreAudioBuffer[i + 1] = (right * SHRT_MAX);
        }
        AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
    }
}

static int CoreAudio_Prime(SndDriver* driver)
{
    struct CoreAudio_SndDriverImpl* impl = driver->impl;

    int bytesPerFrame = 4;
    size_t bufferSize =  impl->frameCount * bytesPerFrame;
    
    int i;
    for (i = 0; i < AUDIO_BUFFERS; ++i)
    {
        OSStatus err = AudioQueueAllocateBuffer(impl->queue,
                                                (UInt32)bufferSize,
                                                &impl->buffers[i]);
        if (err)
        {
            return 0;
        }
        
        driver->running = 1;
        CoreAudioCallback(driver, impl->queue, impl->buffers[i]);
        driver->running = 0;
    }
    
    return 1;
}

static int CoreAudio_Start(SndDriver* driver)
{
    struct CoreAudio_SndDriverImpl* impl = driver->impl;

    OSStatus err = AudioQueueStart(impl->queue, NULL);
    
    if (err)
    {
        return 0;
    }
    
    return 1;
}

static void CoreAudio_Stop(SndDriver* driver)
{
    struct CoreAudio_SndDriverImpl* impl = driver->impl;

    AudioQueueDispose(impl->queue, 1);
    
    free(impl);
}

SndDriver* SndDriver_CoreAudio_Create(int sampleRate)
{
    SndDriver* driver = malloc(sizeof(SndDriver));
    
    if (!driver)
        return NULL;
    
    driver->start = CoreAudio_Start;
    driver->stop = CoreAudio_Stop;
    driver->prime = CoreAudio_Prime;
    driver->running = 0;
    
    driver->sampleRate = sampleRate;
    
    int bytesPerFrame = 4;
    
    driver->impl = malloc(sizeof(struct CoreAudio_SndDriverImpl));
    
    if (!driver->impl)
        return NULL;
    
    struct CoreAudio_SndDriverImpl* impl = driver->impl;
    
    impl->dataFormat.mSampleRate = driver->sampleRate;
    impl->dataFormat.mFormatID = kAudioFormatLinearPCM;
    impl->dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    impl->dataFormat.mFramesPerPacket = 1;
    impl->dataFormat.mBytesPerPacket = BYTES_PER_SAMPLE * CHANNEL_COUNT;
    impl->dataFormat.mBytesPerFrame = bytesPerFrame;
    impl->dataFormat.mChannelsPerFrame = CHANNEL_COUNT;
    impl->dataFormat.mBitsPerChannel = BYTES_PER_SAMPLE * 8;
    
    impl->frameCount = driver->sampleRate / 60;
    
    
    OSStatus error = AudioQueueNewOutput(&impl->dataFormat,
                                       CoreAudioCallback,
                                       driver,
                                       NULL,
                                       kCFRunLoopCommonModes,
                                       0,
                                       &impl->queue);
    
    if (error)
    {
        printf("core audio init error\n");
        return NULL;
    }

    
    return driver;
}
