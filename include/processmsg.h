/*
 * processmsg.h
 *
 *      Author: sheayun
 */

#ifndef INCLUDE_PROCESSMSG_H_
#define INCLUDE_PROCESSMSG_H_

#define MAX_LINE    256


#include "../debug.pb.h"
#include <outputfile.h>

namespace webrtc {
using audioproc::Event;
using audioproc::Init;
using audioproc::Stream;
using audioproc::ReverseStream;


class ProcessMsg {
    int frameCnt;
    int inputSampleRate;
    int outputSampleRate;
    int reverseSampleRate;
    int inputChannels;
    int outputChannels;
    int reverseChannels;
    size_t inputSamplesPerChannel;
    size_t outputSamplesPerChannel;
    size_t reverseSamplesPerChannel;
    OutputFile *inputWavFile;
    OutputFile *outputWavFile;
    OutputFile *reverseWavFile;
private:
    template<typename T>
    void Interleave(const T* const * deinterleaved,
            size_t samplesPerChannel, size_t numChannels,
            T* interleaved);
    void WriteIntData(const int16_t* data, size_t length,
            OutputFile* file);
    void WriteFloatData(const float* const * data,
            size_t samplesPerChannel, size_t numChannels,
            OutputFile* file);
public:
    ProcessMsg();
    ~ProcessMsg();
    int processConfig(Event &eventMsg);
    int processInit(Event &eventMsg, OutputFile* file);
    int processStream(Event &eventMsg);
    int processReverseStream(Event &eventMsg);
};

}

#endif /* INCLUDE_PROCESSMSG_H_ */
