/*
 * outputfile.cc
 *
 *      Author: sheayun
 */

#include <stdio.h>
#include <limits>


#include <outputfile.h>

typedef std::numeric_limits<int16_t> limits_int16;


OutputFile::OutputFile(const char* fileName)
{
    isWav = false;

    if (!strcmp(fileName + strlen(fileName) - 4, ".wav"))
        isWav = true;

    fp = fopen(fileName, "wb");
    if (!fp)
    {
        fprintf(stderr, "File open error: %s\n", fileName);
    }

    bytesWritten = 0;
}

OutputFile::~OutputFile()
{
    if (this->isWav)
        this->adjustWavHeader();

    if (fp)
    {
        fclose (fp);
    }
}

void OutputFile::printLine(const char* line)
{
    fprintf(fp, "%s\n", line);
}


void OutputFile::writeSamples(const int16_t* samples, size_t numSamples)
{
    int written;
    int done;

    done = 0;
    while (done < numSamples)
    {
        written = fwrite(samples, sizeof(*samples), numSamples, fp);
        done += written;
        if (ferror (fp))
        {
            fprintf(stderr, "File writing error [%d:%d]\n",
                    done, written);
            break;
        }
    }

    bytesWritten += done * sizeof(*samples);
}


void OutputFile::writeSamples(const float* samples, size_t numSamples)
{
    static const size_t kChunkSize = 4096 / sizeof(uint16_t);
    for (size_t i = 0; i < numSamples; i += kChunkSize)
    {
        int16_t iSamples[kChunkSize];
        size_t chunk = std::min(kChunkSize, numSamples - i);
        FloatS16ToS16(samples + i, chunk, iSamples);
        writeSamples(iSamples, chunk);
    }
}


void OutputFile::FloatS16ToS16(const float* src, size_t size, int16_t* dest)
{
    static const float kMaxRound = limits_int16::max() - 0.5f;
    static const float kMinRound = limits_int16::min() + 0.5f;
    float v;

    for (size_t i = 0; i < size; i++)
    {
        v = src[i];
        if (v > 0)
        {
            dest[i] = v >= kMaxRound ? limits_int16::max()
                                       :
                                       static_cast<int16_t>(v + 0.5f);
        }
        else
        {
            dest[i] = v <= kMinRound ? limits_int16::min()
                                       :
                                       static_cast<int16_t>(v - 0.5f);
        }
    }
}

void OutputFile::resetWavHeader(int numChannels, int sampleRate)
{
    unsigned char wavHeader[] =
            {
                    0x52, 0x49, 0x46, 0x46, /* RIFF */
                    0x00, 0x00, 0x00, 0x00, /* RIFF chunk size */
                    0x57, 0x41, 0x56, 0x45, /* WAVE */
                    0x66, 0x6d, 0x74, 0x20, /* fmt */
                    0x10, 0x00, 0x00, 0x00, /* chunk size = 16 */
                    0x01, 0x00, /* format tag */
                    0x00, 0x00, /* channels */
                    0x00, 0x00, 0x00, 0x00, /* samples/sec */
                    0x00, 0x00, 0x00, 0x00, /* avg B/s */
                    0x00, 0x00, /* block align */
                    0x10, 0x00, /* bits/sample = 16 */
                    0x64, 0x61, 0x74, 0x61, /* data */
                    0x00, 0x00, 0x00, 0x00 /* data chunk size */
            };

    // number of channels
    wavHeader[22] = (unsigned char) (numChannels % 256);
    wavHeader[23] = (unsigned char) ((numChannels >> 8) % 256);

    // samples per second
    wavHeader[24] = (unsigned char) (sampleRate % 256);
    wavHeader[25] = (unsigned char) ((sampleRate >> 8) % 256);
    wavHeader[26] = (unsigned char) ((sampleRate >> 16) % 256);
    wavHeader[27] = (unsigned char) ((sampleRate >> 24) % 256);

    // average bytes per second
    int avgBPS = sampleRate * 2 * numChannels;
    wavHeader[28] = (unsigned char) (avgBPS % 256);
    wavHeader[29] = (unsigned char) ((avgBPS >> 8) % 256);
    wavHeader[30] = (unsigned char) ((avgBPS >> 16) % 256);
    wavHeader[31] = (unsigned char) ((avgBPS >> 24) % 256);

    // block align
    int blkAlign = numChannels * 2;
    wavHeader[32] = (unsigned char) (blkAlign % 256);
    wavHeader[33] = (unsigned char) ((blkAlign >> 8) % 256);

    fseek(fp, 0, SEEK_SET);
    fwrite((void *) wavHeader, 1, sizeof(wavHeader), fp);
}


void OutputFile::adjustWavHeader(void)
{
    unsigned char bytes[4];
    unsigned int dataChunkSize = bytesWritten;
    unsigned int riffChunkSize = dataChunkSize + 36;

    bytes[0] = (unsigned char) (dataChunkSize % 256);
    bytes[1] = (unsigned char) ((dataChunkSize >> 8) % 256);
    bytes[2] = (unsigned char) ((dataChunkSize >> 16) % 256);
    bytes[3] = (unsigned char) ((dataChunkSize >> 24) % 256);
    fseek(fp, 40, SEEK_SET);
    fwrite((void *) bytes, 1, sizeof(bytes), fp);

    bytes[0] = (unsigned char) (riffChunkSize % 256);
    bytes[1] = (unsigned char) ((riffChunkSize >> 8) % 256);
    bytes[2] = (unsigned char) ((riffChunkSize >> 16) % 256);
    bytes[3] = (unsigned char) ((riffChunkSize >> 24) % 256);
    fseek(fp, 4, SEEK_SET);
    fwrite((void *) bytes, 1, sizeof(bytes), fp);
}
