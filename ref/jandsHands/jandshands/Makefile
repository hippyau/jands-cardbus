#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL
#

CXX = g++
#CXX = clang++

EXE = jandshands
SOURCES = main.cpp tinyxml2.cpp serial.cpp
SOURCES += osc/OscTypes.cpp osc/OscReceivedElements.cpp osc/OscPrintReceivedElements.cpp osc/OscOutboundPacketStream.cpp 
SOURCES += ip/posix/UdpSocket.cpp ip/IpEndpointName.cpp ip/posix/NetworkingUtils.cpp 
SOURCES += imgui/examples/imgui_impl_sdl.cpp imgui/examples/imgui_impl_opengl2.cpp  
SOURCES += imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp 


OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS =-DTIXML_USE_TICPP -DLINUX -Iimgui -Iimgui/examples -Iosc -Iip
CXXFLAGS += -g -Wall -Wformat
LIBS = -lpthread 

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"

	LIBS += -lGL -ldl `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

	CXXFLAGS += `pkg-config --cflags sdl2`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:osc/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:ip/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:ip/posix/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


%.o:imgui/examples/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
