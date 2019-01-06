
#ifndef SND_H
#define SND_H

#include <stdlib.h>

#define SND_OGG_VORBIS 0


typedef enum
{
    kSndFormatRaw = 0,
    
} SndFormat;


/* a sound sample - memory intensive */
typedef struct
{
    /* private data access */
    union
    {
        void* p;
        char* p8;
        short* p16;
        float* p32;
    } data; 
    
    short bitsPerSample;
    int bytesPerSecond;
    int sampleRate;
    int channelCount;
    unsigned int sampleCount;
    SndFormat format;
    
} Snd;

extern int Snd_Init(Snd* snd,
                    SndFormat format,
                    size_t length,
                    int channelCount,
                    int sampleRate,
                    int bytesPerSecond,
                    short bitsPerSample);

extern int Snd_FromPath(Snd* sound, const char* path);


extern void Snd_Shutdown(Snd* snd);


extern void Snd_GenRand(Snd* snd, float amp);
extern void Snd_GenSquare(Snd* snd, int frequency, float amp);
extern float Snd_UnpackSample(const Snd* snd, int frame, int channel);
extern void Snd_PackSample(Snd* snd, int frame, int channel, float val);

extern unsigned int Snd_SampleBufferLength(const Snd* snd);


#endif
