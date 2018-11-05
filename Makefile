DIRS=mqtt

#CXXFLAGS =  -std=c++11 -MMD -O0 -g -Wall -fmessage-length=0 -I .  -I /home/ds/workspace/apps/mqtt/mosquitto-1.5/lib
#CXXFLAGS = -O0 -g -Wall -fmessage-length=0 -I .  -I /home/ds/workspace/apps/mqtt/mosquitto-1.5/lib

CXXFLAGS =  -std=c++17 -MMD -O0 -g3 -ggdb -Wall -Wfatal-errors -fmessage-length=0 \
			-I . \
			-I ./serial \
			-I ./l2 \
			-I ./l3 \
			-I ./l4 \
			-I ./json \
			-I ./utils \
			-I ./mqtt \
			-I ./mosquitto \
			-I ./msg24hrStorage


OBJS =		bezel.o \
			config.o \
			serial/serial_kb.o \
			l2/L2Comm.o \
			l3/L3Comm.o \
			l3/L3CommEvents.o \
			l3/L3CommFastlan.o \
			l3/L3CommReadWriteCmd.o \
			l3/L3CommRqstResponse.o \
			l4/L4Comm.o \
			json/jsonInterface.o \
			utils/utilities.o \
			msg24hrStorage/msg24hrStorage.o
			
DEPS := $(OBJS:.o=.d)
-include $(DEPS)


LIBS += -pthread \
		-L ./mosquitto/cpp ./mosquitto/cpp/libmosquittopp.a \
		-L ./mosquitto    ./mosquitto/libmosquitto.a \
		-lrt

		
#		-L /home/ds/workspace/apps/zeromq-4.2.3/src/.libs -lzmq \

TARGET =	bezel

$(TARGET):	$(OBJS)
	$(MAKE) -C mosquitto
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -Xlinker -Map=bezel.map 


all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.d
	$(MAKE) -C mosquitto clean
	
	
