/*
 * inputfile.h
 *
 *      Author: sheayun
 */

#ifndef INPUTFILE_H_
#define INPUTFILE_H_

#include <memory>
#include <string>
#include <google/protobuf/message_lite.h>

using namespace google::protobuf;

class InputFile {
    FILE* fp;
private:
    size_t readMessageBytesFromFile(FILE* file,
            std::unique_ptr<uint8_t[]>* bytes);
public:
    InputFile(const char* fileName);
    ~InputFile();
    bool readMessage(MessageLite* msg);
};

#endif /* INPUTFILE_H_ */

