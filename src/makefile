CC = g++
CPPFLAGS = -std=c++14 -O2
LINKFLAGS = -lpthread
ALLSOURCES = $(wildcard *.cpp)
ALLOBJECTS = $(ALLSOURCES:%.cpp=%.o)
HEADERS = $(wildcard *.h)
MAKEFILE = makefile

SOURCES = $(filter-out main-1.cpp, $(ALLSOURCES))
OBJECTS = $(filter-out main-1.o, $(ALLOBJECTS))
TARGET = main2

OLDSOURCES = $(filter-out main.cpp, $(ALLSOURCES))
OLDOBJECTS = $(filter-out main.o, $(ALLOBJECTS))
OLDTARGET = main1

.PHONY: all
all: $(TARGET) $(OLDTARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) $(LINKFLAGS) -o $(TARGET)

$(OLDTARGET): $(OLDOBJECTS) $(HEADERS)
	$(CC) $(OLDOBJECTS) $(LINKFLAGS) -o $(OLDTARGET)

%.o: %.cpp $(HEADERS) $(MAKEFILE)
	$(CC) $(CPPFLAGS) $< -o $@ -c

.PHONY: clean
clean:
	rm -f *.o main1 main2
