
#ifndef SND_DRIVER_CORE_AUDIO_H
#define SND_DRIVER_CORE_AUDIO_H

#include "snd_driver.h"

/* Core Audio is an Apple API. Mac/iOS */

/* 44100 */
extern SndDriver* SndDriver_CoreAudio_Create(int sampleRate);

#endif
