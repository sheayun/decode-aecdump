/*
 * main.cc
 *
 *      Author: sheayun
 */


#include <stdio.h>

#include "debug.pb.h"
#include <inputfile.h>
#include <outputfile.h>
#include <processmsg.h>

namespace webrtc {

using audioproc::Event;

int processInputFile(const char* fileName);


int doMain(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: decode <input_file>\n");
		return 1;
	}

	return processInputFile(argv[1]);
}


int processInputFile(const char* fileName)
{
	InputFile* inputFile = new InputFile(fileName);

	Event eventMsg;

    OutputFile* settingsFile = new OutputFile("settings.txt");
    ProcessMsg* proc = new ProcessMsg();
	while (inputFile->readMessage(&eventMsg))
	{
        int r;

		if (eventMsg.type() == Event::CONFIG)
		{
            r = proc->processConfig(eventMsg);
            if (r)
                return r;
		}
		else if (eventMsg.type() == Event::INIT)
		{
            r = proc->processInit(eventMsg, settingsFile);
            if (r)
                return r;
		}
		else if (eventMsg.type() == Event::STREAM)
		{
            r = proc->processStream(eventMsg);
            if (r)
                return r;
		}
		else if (eventMsg.type() == Event::REVERSE_STREAM)
		{
            r = proc->processReverseStream(eventMsg);
            if (r)
                return r;
		}
	}

	delete inputFile;
    delete settingsFile;
    delete proc;

	return 0;
}

}


int main(int argc, char* argv[])
{
	return webrtc::doMain(argc, argv);
}
