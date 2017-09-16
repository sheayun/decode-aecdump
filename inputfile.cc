/*
 * inputfile.cc
 *
 *      Author: sheayun
 */

#include <stdio.h>

#include <inputfile.h>

InputFile::InputFile(const char* fileName)
{
    fp = fopen(fileName, "rb");
    if (!fp)
    {
        fprintf(stderr, "File open error: %s\n", fileName);
    }
}

InputFile::~InputFile()
{
    if (fp)
    {
        fclose(fp);
    }
}

bool InputFile::readMessage(MessageLite* msg)
{
    std::string s;
    std::unique_ptr<uint8_t[]> bytes;
    size_t size = readMessageBytesFromFile(fp, &bytes);

    if (!size)
        return false;

    msg->Clear();
    return msg->ParseFromArray(bytes.get(), size);
}

size_t InputFile::readMessageBytesFromFile(FILE* file,
        std::unique_ptr<uint8_t[]>* bytes)
{
    int32_t size = 0;
    if (fread(&size, sizeof(size), 1, file) != 1)
        return 0;
    if (size <= 0)
        return 0;

    bytes->reset(new uint8_t[size]);
    return fread(bytes->get(), sizeof((*bytes)[0]), size, file);
}
