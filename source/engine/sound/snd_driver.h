
#ifndef SND_DRIVER_H
#define SND_DRIVER_H

#include <stdlib.h>

typedef struct SndDriver
{
    int sampleRate;
    int bytesPerSample;
    void (*callback)(void* delegate, struct SndDriver* driver, float* leftOut, float* rightOut);
    
    int (*prime)(struct SndDriver* driver);
    int (*start)(struct SndDriver* driver);
    void (*stop)(struct SndDriver* driver);
    
    int running;
    void* impl;
    
    void* delegate;
} SndDriver;


extern SndDriver* SndDriver_Null_Create();

#endif
