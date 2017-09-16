CXX = g++
LD = g++
PROTOC = protoc

OBJS = main.o debug.pb.o inputfile.o outputfile.o processmsg.o

CFLAGS = -I./include
LDFLAGS = -lprotobuf

%.o: %.cc
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

all: decode

decode: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

main.o: debug.pb.h

debug.pb.cc: debug.proto
	$(PROTOC) --cpp_out=./ $<

debug.pb.h: debug.proto
	$(PROTOC) --cpp_out=./ $<

clean:
	rm -f decode $(OBJS) debug.pb.cc debug.pb.h
