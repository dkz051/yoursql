CC = g++
CPPFLAGS = -std=c++14 -g
LINKFLAGS = -lpthread
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:%.cpp=%.o)
TARGET = main

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) $(LINKFLAGS) -o $(TARGET)

%.o: %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) $< -o $@ -c

.PHONY: clean
clean:
	rm -f *.o main
