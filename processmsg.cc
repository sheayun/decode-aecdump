/*
 * processmsg.cc
 *
 *      Author: sheayun
 */

#include <stdio.h>

#include <processmsg.h>


using namespace webrtc;


ProcessMsg::ProcessMsg()
{
    frameCnt = 0;
    inputSampleRate = 0;
    outputSampleRate = 0;
    reverseSampleRate = 0;
    inputChannels = 0;
    outputChannels = 0;
    reverseChannels = 0;
    inputSamplesPerChannel = 0;
    outputSamplesPerChannel = 0;
    reverseSamplesPerChannel = 0;
    inputWavFile = NULL;
    outputWavFile = NULL;
    reverseWavFile = NULL;
}


ProcessMsg::~ProcessMsg()
{
    if (inputWavFile)
        delete inputWavFile;

    if (outputWavFile)
        delete outputWavFile;

    if (reverseWavFile)
        delete reverseWavFile;
}


int ProcessMsg::processConfig(Event &eventMsg)
{
    return 0;
}


int ProcessMsg::processInit(Event &eventMsg, OutputFile* file)
{
    char line[MAX_LINE];
    char fileName[MAX_LINE];

    if (!eventMsg.has_init())
    {
        fprintf(stderr, "Corrupt input file: Init missing.\n");
        return 1;
    }

    const Init msg = eventMsg.init();
    sprintf(line, "Init at frame %d", frameCnt);
    file->printLine(line);

    inputSampleRate = msg.sample_rate();
    sprintf(line, "  Input sample rate: %d", inputSampleRate);
    file->printLine(line);

    outputSampleRate = msg.output_sample_rate();
    sprintf(line, "  Output sample rate: %d", outputSampleRate);
    file->printLine(line);

    reverseSampleRate = msg.reverse_sample_rate();
    sprintf(line, "  Reverse sample rate: %d", reverseSampleRate);
    file->printLine(line);

    inputChannels = msg.num_input_channels();
    sprintf(line, "  Input channels: %d", inputChannels);
    file->printLine(line);

    outputChannels = msg.num_output_channels();
    sprintf(line, "  Output channels: %d", outputChannels);
    file->printLine(line);

    reverseChannels = msg.num_reverse_channels();
    sprintf(line, "  Reverse channels: %d", reverseChannels);
    file->printLine(line);

    file->printLine("");

    if (reverseSampleRate == 0)
        reverseSampleRate = inputSampleRate;
    if (outputSampleRate == 0)
        outputSampleRate = inputSampleRate;

    inputSamplesPerChannel = static_cast<size_t>(inputSampleRate / 100);
    outputSamplesPerChannel = static_cast<size_t>(outputSampleRate / 100);
    reverseSamplesPerChannel = static_cast<size_t>(reverseSampleRate / 100);

    sprintf(fileName, "input%d.wav", frameCnt);
    if (inputWavFile)
        delete inputWavFile;
    inputWavFile = new OutputFile(fileName);
    inputWavFile->resetWavHeader(inputChannels, inputSampleRate);

    sprintf(fileName, "ref_out%d.wav", frameCnt);
    if (outputWavFile)
        delete outputWavFile;
    outputWavFile = new OutputFile(fileName);
    outputWavFile->resetWavHeader(outputChannels, outputSampleRate);

    sprintf(fileName, "reverse%d.wav", frameCnt);
    if (reverseWavFile)
        delete reverseWavFile;
    reverseWavFile = new OutputFile(fileName);
    reverseWavFile->resetWavHeader(reverseChannels, reverseSampleRate);

    return 0;
}


int ProcessMsg::processStream(Event &eventMsg)
{
    frameCnt++;

    if (!eventMsg.has_stream())
    {
        fprintf(stderr, "Corrupt input file: Stream missing.\n");
        return 1;
    }

    const Stream msg = eventMsg.stream();

    if (msg.has_input_data())
    {
        const int16_t* ptr =
                reinterpret_cast<const int16_t*>(msg.input_data().data());
        WriteIntData(ptr, inputChannels * inputSamplesPerChannel,
                inputWavFile);
    }
    else if (msg.input_channel_size() > 0)
    {
        std::unique_ptr<const float*[]> data(new const float*[inputChannels]);
        for (size_t i = 0; i < inputChannels; i++)
        {
            data[i] =
                    reinterpret_cast<const float*>(msg.input_channel(i).data());
        }
        WriteFloatData(data.get(), inputSamplesPerChannel,
                inputChannels, inputWavFile);
    }

    if (msg.has_output_data())
    {
        const int16_t* ptr =
                reinterpret_cast<const int16_t*>(msg.output_data().data());
        WriteIntData(ptr, outputChannels * outputSamplesPerChannel,
                outputWavFile);
    }
    else if (msg.output_channel_size() > 0)
    {
        std::unique_ptr<const float*[]> data(new const float*[outputChannels]);
        for (size_t i = 0; i < outputChannels; i++)
        {
            data[i] =
                    reinterpret_cast<const float*>(msg.output_channel(i).data());
        }
        WriteFloatData(data.get(), outputSamplesPerChannel,
                outputChannels, outputWavFile);
    }

    return 0;
}


int ProcessMsg::processReverseStream(Event &eventMsg)
{
    if (!eventMsg.has_reverse_stream())
    {
        fprintf(stderr, "Corrupt input file: ReverseStream missing.\n");
        return 1;
    }

    const ReverseStream msg = eventMsg.reverse_stream();

    if (msg.has_data())
    {
        const int16_t* ptr =
                reinterpret_cast<const int16_t*>(msg.data().data());
        WriteIntData(ptr, reverseChannels * reverseSamplesPerChannel,
                reverseWavFile);
    }
    else if (msg.channel_size() > 0)
    {
        std::unique_ptr<const float*[]> data(new const float*[reverseChannels]);
        for (size_t i = 0; i < reverseChannels; i++)
        {
            data[i] = reinterpret_cast<const float*>(msg.channel(i).data());
        }
        WriteFloatData(data.get(), reverseSamplesPerChannel,
                reverseChannels, reverseWavFile);
    }

    return 0;
}


template<typename T>
void ProcessMsg::Interleave(const T* const * deinterleaved,
        size_t samplesPerChannel,
        size_t numChannels, T* interleaved)
{
    for (size_t i = 0; i < numChannels; ++i)
    {
        const T* channel = deinterleaved[i];
        size_t interleaved_idx = i;
        for (size_t j = 0; j < samplesPerChannel; ++j)
        {
            interleaved[interleaved_idx] = channel[j];
            interleaved_idx += numChannels;
        }
    }
}


void ProcessMsg::WriteIntData(const int16_t* data, size_t length,
        OutputFile* file)
{
    file->writeSamples(data, length);
}


void ProcessMsg::WriteFloatData(const float* const * data,
        size_t samplesPerChannel,
        size_t numChannels, OutputFile* file)
{
    size_t length = numChannels * samplesPerChannel;

    std::unique_ptr<float[]> buffer(new float[length]);
    Interleave(data, samplesPerChannel, numChannels, buffer.get());

    for (size_t i = 0; i < length; i++)
    {
        buffer[i] =
                buffer[i] > 0 ?
                        buffer[i] * std::numeric_limits<int16_t>::max() :
                        -buffer[i] * std::numeric_limits<int16_t>::min();
    }

    file->writeSamples(buffer.get(), length);
}
