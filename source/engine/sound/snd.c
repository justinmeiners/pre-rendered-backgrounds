
#include "snd.h"
#include "utils.h"

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>


#if SND_OGG_VORBIS
extern int Snd_FromOGG(Snd* snd, FILE* file);
#endif


/*
 https://developer.valvesoftware.com/wiki/Env_speaker
 https://developer.valvesoftware.com/wiki/Env_microphone
*/
 

int Snd_Init(Snd* snd,
             SndFormat format,
             size_t length,
             int channelCount,
             int sampleRate,
             int bytesPerSecond,
             short bitsPerSample)
{
    if (!snd)
        return 0;
    
    if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 32)
    {
        printf("invalid bit depth\n");
        return 0;
    }
    
    memset(snd, 0x0, sizeof(Snd));
    
    snd->format = format;
    
    snd->bitsPerSample = bitsPerSample;
    snd->sampleCount = (uint)length;
    snd->channelCount = channelCount;
    
    snd->sampleRate = sampleRate;
    snd->bytesPerSecond = bytesPerSecond;
    
    snd->data.p = malloc((bitsPerSample / 8) * length * channelCount);
    
    if (!snd->data.p)
        return 0;
    
    snd->format = kSndFormatRaw;
    return 1;

}

void Snd_Shutdown(Snd* snd)
{
    if (snd->data.p)
    {
        free(snd->data.p);
    }
}

void Snd_GenRand(Snd* snd, float amp)
{
    int i, j;
    for (i = 0; i < snd->sampleCount; ++i)
    {
        for (j = 0; j < snd->channelCount; ++j)
        {
            Snd_PackSample(snd, i * snd->channelCount, j, (rand() / (float)INT_MAX) * amp);
        }
    }
}

void Snd_GenSquare(Snd* snd, int frequency, float amp)
{
}

float Snd_UnpackSample(const Snd* snd, int frame, int channel)
{
    assert(snd);
    assert(frame >= 0 && frame < snd->sampleCount);
    
    if (channel >= snd->channelCount)
    {
        channel = snd->channelCount - 1;
    }
    
    uint index = (frame * snd->channelCount) + channel;
    
    switch (snd->bitsPerSample)
    {
        case 8:
            return snd->data.p8[index] / (float)CHAR_MAX;
        case 16:
            return snd->data.p16[index] / (float)SHRT_MAX;
        case 32:
            return snd->data.p32[index];
        default:
            assert(0);
            break;
    }

    return 0.0f;
}

void Snd_PackSample(Snd* snd, int frame, int channel, float val)
{
    assert(snd);
    assert(frame >= 0 && frame < snd->sampleCount);
    
    if (channel >= snd->channelCount)
    {
        channel = snd->channelCount - 1;
    }
    
    uint index = (frame * snd->channelCount) + channel;
    
    switch (snd->bitsPerSample)
    {
        case 8:
            snd->data.p8[index] = val * (float)CHAR_MAX;
            break;
        case 16:
            snd->data.p16[index] = val * (float)SHRT_MAX;
            break;
        case 32:
            snd->data.p32[index] = val;
            break;
        default:
            assert(0);
            break;
    }
}

uint Snd_SampleBufferLength(const Snd* snd)
{
    return snd->sampleCount * (snd->bitsPerSample / 8);
}

typedef struct
{
    char riff[4];      // 'RIFF'
    uint32_t chunkSize;
    char wav[4];      // 'WAVE'
} WavHeader;

typedef struct
{
    char format[4];
    uint32_t chunkDataSize;
    uint16_t compressionCode;
    uint16_t channelCount;
    uint32_t sampleRate;
    uint32_t bytesPerSecond;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} WavChunkFormat;

typedef struct
{
    char dataID[4];
    uint32_t chunkSize;
    
} WavDataChunk;

/* https://ccrma.stanford.edu/courses/422/projects/WaveFormat/  */
/* http://www.sonicspot.com/guide/wavefiles.html */

int Snd_FromWAV(Snd* snd, FILE* file)
{
    WavHeader header;
    WavChunkFormat format;
    WavDataChunk dataChunk;
    
    fread(&header, sizeof(WavHeader), 1, file);
    fread(&format, sizeof(WavChunkFormat), 1, file);
    fread(&dataChunk, sizeof(WavDataChunk), 1, file);
    
    assert(format.compressionCode == 1);
    
    size_t length = (dataChunk.chunkSize / format.channelCount) / (format.bitsPerSample / 8);
    
    Snd_Init(snd,
             kSndFormatRaw,
             length,
             format.channelCount,
             format.sampleRate,
             format.bytesPerSecond,
             format.bitsPerSample);
    
    fread(snd->data.p, dataChunk.chunkSize, 1, file);
    return 1;
}

int Snd_FromPath(Snd* sound, const char* path)
{
    FILE* file = fopen(path, "rb");
    
    if (!file) { return 0; }
    
    int status = 0;
    
    if (strcmp(Filepath_Extension(path), "wav") == 0)
    {
        status = Snd_FromWAV(sound, file);
    }
#if SND_OGG_VORBIS
    else if (strcmp(Filepath_Extension(path), "ogg") == 0)
    {
        status = Snd_FromOGG(sound, file);
    }
#endif
    
    fclose(file);
    
    if (status)
    {
        return 1;
    }
    
    return 0;
}


