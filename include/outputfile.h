/*
 * outputfile.h
 *
 *      Author: sheayun
 */

#ifndef INCLUDE_OUTPUTFILE_H_
#define INCLUDE_OUTPUTFILE_H_

#include <string>


class OutputFile {
    bool isWav;
    FILE* fp;
    int bytesWritten;
private:
    void FloatS16ToS16(const float* src, size_t size, int16_t* dest);
public:
    OutputFile(const char* name);
    ~OutputFile();
    void printLine(const char* line);
    void writeSamples(const int16_t* samples, size_t numSamples);
    void writeSamples(const float* samples, size_t numSamples);
    void resetWavHeader(int numChannels, int sampleRate);
    void adjustWavHeader(void);
};

#endif /* INCLUDE_OUTPUTFILE_H_ */
