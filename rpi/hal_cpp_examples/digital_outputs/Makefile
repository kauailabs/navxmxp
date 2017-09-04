SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS=$(SOURCES:.cpp=)

CFLAGS+=-MMD
CXXFLAGS+=-I/usr/local/include/vmxpi -O0 -g3 -Wall -fmessage-length=0 -std=c++11 -g -rdynamic -MMD -MP -L/usr/local/lib/vmxpi -lvmxpi_hal_cpp -lrt -lpthread

all: $(BINS)

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)

-include $(DEPS)