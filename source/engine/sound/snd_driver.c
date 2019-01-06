
#include "snd_driver.h"


static int Null_Start(SndDriver* driver)
{
    return 1;
}

SndDriver* SndDriver_Null_Create()
{
    SndDriver* driver = malloc(sizeof(SndDriver));
    if (!driver)
        return NULL;
    
    driver->impl = NULL;
    driver->sampleRate = 44100;
    driver->bytesPerSample = 4;
    driver->start = Null_Start;
    return driver;
}

