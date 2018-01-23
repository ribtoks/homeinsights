
# Defines the RPI variable which is needed by rc-switch/RCSwitch.h
CXXFLAGS=-DRPI

all: tempserver

tempserver: ../../../vendors/rc-switch/RCSwitch.o tempserver.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lsqlite3

clean:
	$(RM) ../../../vendors/rc-switch/*.o *.o tempserver
