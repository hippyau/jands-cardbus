// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_sdl_opengl3/ folder**
// See imgui_impl_sdl.cpp for details.


#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <errno.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <regex>
#include <map>
#include <iterator>

#include <thread>
#include <mutex>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui/imgui_memory_editor.h"

//#include "tinyxml2.h"

#include <algorithm> 
#include <cctype>
#include <locale>


// OSC
#include "osc/OscOutboundPacketStream.h"

#include "ip/UdpSocket.h"
#include "ip/PacketListener.h"

#define OSCADDRESS "192.168.1.33"
#define OSCOUTPORT 3819
#define OSC_BUFFER_SIZE 1024

UdpTransmitSocket * transmitSocket;



using namespace std;

                        

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}



// utils to turn ints and chars into hex strings
template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789abcdef";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return "0x" + rc;
}


// make a 10101100 string...
// not threadsafe
const char* FormatBinary(const uint8_t* buf, int width) 
{
    IM_ASSERT(width <= 64);
    size_t out_n = 0;
    
    static char out_buf[64 + 8 + 1];
    int n = width / 8;
    for (int j = n - 1; j >= 0; --j)
    {
        for (int i = 0; i < 8; ++i)
            out_buf[out_n++] = (buf[j] & (1 << (7 - i))) ? '1' : '0';
        out_buf[out_n++] = ' ';
    }
    IM_ASSERT(out_n < IM_ARRAYSIZE(out_buf));
    out_buf[out_n] = 0;
    return out_buf;
}







// /* telnet */
// const char *HOST = "192.168.1.33";
// const unsigned PORT = 5501;
// const int BUFLEN = 256;

// class FGFSSocket {
// public:
// 	FGFSSocket(const char *name, unsigned port);
// 	~FGFSSocket();

// 	int		write(const char *msg, ...);
// 	const char	*read(void);
// 	inline void	flush(void);
// 	void		settimeout(unsigned t) { _timeout = t; }

// private:
// 	int		close(void);

// 	int		_sock;
// 	bool		_connected;
// 	unsigned	_timeout;
// 	char		_buffer[BUFLEN];
// };

// FGFSSocket::FGFSSocket(const char *hostname = HOST, unsigned port = PORT) :
// 	_sock(-1),
// 	_connected(false),
// 	_timeout(1)
// {
// 	_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
// 	if (_sock < 0)
// 		throw("FGFSSocket/socket");

// 	struct hostent *hostinfo;
// 	hostinfo = gethostbyname(hostname);
// 	if (!hostinfo) {
// 		close();
// 		throw("FGFSSocket/gethostbyname: unknown host");
// 	}

// 	struct sockaddr_in serv_addr;
// 	serv_addr.sin_family = AF_INET;
// 	serv_addr.sin_port = htons(port);
// 	serv_addr.sin_addr = *(struct in_addr *)hostinfo->h_addr;

// 	if (connect(_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
// 		close();
// 		throw("FGFSSocket/connect");
// 	}
// 	_connected = true;
// 	try {
// 		write("data");
// 	} catch (...) {
// 		close();
// 		throw;
// 	}
// }


// FGFSSocket::~FGFSSocket()
// {
// 	close();
// }


// int FGFSSocket::close(void)
// {
// 	if (_connected)
// 		write("quit");
// 	if (_sock < 0)
// 		return 0;
// 	int ret = ::close(_sock);
// 	_sock = -1;
// 	return ret;
// }


// int FGFSSocket::write(const char *msg, ...)
// {
// 	va_list va;
// 	ssize_t len;
// 	char buf[BUFLEN];
// 	fd_set fd;
// 	struct timeval tv;

// 	FD_ZERO(&fd);
// 	FD_SET(_sock, &fd);
// 	tv.tv_sec = _timeout;
// 	tv.tv_usec = 0;
// 	if (!select(FD_SETSIZE, 0, &fd, 0, &tv))
// 		throw("FGFSSocket::write/select: timeout exceeded");

// 	va_start(va, msg);
// 	vsnprintf(buf, BUFLEN - 2, msg, va);
// 	va_end(va);
// 	std::cout << "SEND: " << buf << std::endl;
// 	strcat(buf, "\015\012");

// 	len = ::write(_sock, buf, strlen(buf));
// 	if (len < 0)
// 		throw("FGFSSocket::write");
// 	return len;
// }


// const char *FGFSSocket::read(void)
// {
// 	char *p;
// 	fd_set fd;
// 	struct timeval tv;
// 	ssize_t len;

// 	FD_ZERO(&fd);
// 	FD_SET(_sock, &fd);
// 	tv.tv_sec = _timeout;
// 	tv.tv_usec = 0;
// 	if (!select(FD_SETSIZE, &fd, 0, 0, &tv)) {
// 		if (_timeout == 0)
// 			return 0;
// 		else
// 			throw("FGFSSocket::read/select: timeout exceeded");
// 	}

// 	len = ::read(_sock, _buffer, BUFLEN - 1);
// 	if (len < 0)
// 		throw("FGFSSocket::read/read");
// 	if (len == 0)
// 		return 0;

// 	for (p = &_buffer[len - 1]; p >= _buffer; p--)
// 		if (*p != '\015' && *p != '\012')
// 			break;
// 	*++p = '\0';
// 	return strlen(_buffer) ? _buffer : 0;
// }


// inline void FGFSSocket::flush(void)
// {
// 	int i = _timeout;
// 	_timeout = 0;
// 	while (read())
// 		;
// 	_timeout = i;
// }



// FGFSSocket * fs = NULL;





std::mutex _gCANRX_mutex; // protect the map







char * memview = NULL;
int memviewsize = 0;
static MemoryEditor mem_edit1;
    
int s, i; 
int nbytes;
struct sockaddr_can addr;
struct ifreq ifr;
struct can_frame frame;	

bool quit = false;

bool init = false;






void sendosc(const char * param, int value){
       
    char buffer[OSC_BUFFER_SIZE];
    osc::OutboundPacketStream p( buffer, OSC_BUFFER_SIZE );

    printf("OSC> %s %d\n",param, value);

    p << osc::BeginBundleImmediate

        << osc::BeginMessage( param ) 
            << value 
        << osc::EndMessage
  //      << osc::BeginMessage( "/test2" ) 
  //          << true << 24 << (float)10.8 << "world" 
  //      << osc::EndMessage    
    << osc::EndBundle;
    transmitSocket->Send( p.Data(), p.Size() );
}


void sendosc_f(const char * param, float value){      
    char buffer[OSC_BUFFER_SIZE];
    osc::OutboundPacketStream p( buffer, OSC_BUFFER_SIZE );

    printf("CANF> %s %f\n",param);

    p << osc::BeginBundleImmediate

        << osc::BeginMessage( param ) 
            << value 
        << osc::EndMessage

  //      << osc::BeginMessage( "/test2" ) 
  //          << true << 24 << (float)10.8 << "world" 
  //      << osc::EndMessage
    
    << osc::EndBundle;
    transmitSocket->Send( p.Data(), p.Size() );
}



uint8_t Faders[64];
uint8_t Buttons[13];
uint8_t Wheels[3];



enum Keys {

    FLASH0 = 0,
    FLASH1,
    FLASH2,
    FLASH3,
    FLASH4,
    FLASH5,
    FLASH6,
    FLASH7
};



class EmptyPacketListener : public PacketListener {
public:
	virtual void ProcessPacket( const char *data, int size, 
			const IpEndpointName& remoteEndpoint )
	{
        (void) remoteEndpoint; // suppress unused parameter warning

		std::cout << data << size << endl;
        
	}
};



void rx_thread(){

    // open UDP port 8888, listen for incoming JCB0 Packets

    EmptyPacketListener listener;
    IpEndpointName t;
    t.address = IpEndpointName::ANY_ADDRESS;
    t.port = 8888;
    UdpListeningReceiveSocket s(t, &listener );
  
    char buffer[128];
    char * buf = buffer;

    memview = buf;
    memviewsize = 128;

while (!quit){


     int rx = s.ReceiveFrom( t, buf, 128);
     memviewsize = rx;
   
    //  check is a JCB0 packet
    if ((buffer[0] == 'J') && (buffer[1] == 'C') && (buffer[2] == 'B') && (buffer[3] == '0')){

        // faders x 64 x 8-bit bytes
        // 01..24 preset card 1
        // 25..48 preset card 2
        // 49..56 assign card 1
        // 57..64 master card                 
        for (int cnt = 0 ; cnt < 64 ; cnt++){
            Faders[cnt] = buffer[cnt+4]; // skip header
        }

        // buttons x 13 bytes
        // preset 1 = 2 bytes -> 12 buttons
        // preset 2 = 2 bytes -> 12 buttons
        // assign 1 = 1 bytes -> 8 buttons
        // palette = 3 bytes -> 20 buttons
        // master = 5 bytes -> 37 buttons
        for (int cnt = 0 ; cnt < 13 ; cnt++){
            Buttons[cnt] = buffer[cnt+64+4]; // skip faders and header 
        }

        
        // wheels x 3
        for (int cnt = 0 ; cnt < 3 ; cnt++){
            Wheels[cnt] = buffer[cnt+64+4+13]; // skip faders and header and buttons
        }

    }
    else {
        printf("Not a JCB0 Packet!\n");        
    }

}


}





// Main code
int main(int argc, char* argv[])
{
    auto start = chrono::steady_clock::now();

    // byte editor
    char * pEditID = NULL;
    
 // setup OSC
    transmitSocket = new UdpTransmitSocket ( IpEndpointName( OSCADDRESS, OSCOUTPORT ) );
 


    // Setup SDL 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Event4 CardBus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    ImVec4 clear_color = ImVec4(0.0f, 0.00f, 0.60f, 1.00f);

    std::thread rxthread (rx_thread);
    
    // Main loop
    bool done = false;
    while (!done)
    {

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();



        const float spacing = 4;
        ImGui::Begin("Preset 1");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
        ImGui::PushID("faderset1");

        for (int i = 0; i < 12; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::NewLine();

        for (int i = 12; i < 24; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopStyleVar(1);

        ImGui::End();

        ImGui::Begin("Preset 2");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
        ImGui::PushID("faderset2");

        for (int i = 24; i < 36; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::NewLine();

        for (int i = 36; i < 48; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::PopID();
        ImGui::PopStyleVar(1);
        ImGui::End();



      ImGui::Begin("Assign 1");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
        ImGui::PushID("faderset2");

        for (int i = 48; i < 56; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

       
        ImGui::PopID();
        ImGui::PopStyleVar(1);
        ImGui::End();


   ImGui::Begin("Master");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
        ImGui::PushID("faderset2");

        for (int i = 56; i < 64; i++)
        {
            ImGui::PushID(i);

            int tmp = Faders[i];
            if (ImGui::VSliderInt("##int", ImVec2(20, 160), &tmp, 0, 255))
            {
                // SendFader(i,Faders[i]);
            }
            ImGui::SameLine();
            ImGui::PopID();
        }

       
        ImGui::PopID();
        ImGui::PopStyleVar(1);
        ImGui::End();


// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
//   if (show_demo_window)
//       ImGui::ShowDemoWindow(&show_demo_window);

// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
{
    ImGui::Begin("Jands CardBus!"); // Create a window called "Hello, world!" and append into it.

    ImGui::Text("written by Hippy (2020)"); // Create a window called "Hello, world!" and append into it.
    ImGui::NewLine();

    // ImGui::Checkbox("Listen?",&g_listening);
    // ImGui::SameLine();
    // ImGui::Checkbox("Init",&init);
    // ImGui::SameLine();
    // ImGui::Checkbox("Poll?",&g_polling);
    // ImGui::SameLine();
    // if (ImGui::Button("SAVE")){
    //     store_xml(CanFileName);
    // }

    // ImGui::SameLine();
    // if (ImGui::Button("LOAD")){
    //     load_xml(CanFileName);
    // }

    // ImGui::InputText("Filename",(char*)CanFileName.c_str(),CanFileName.capacity());

    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
        }



        mem_edit1.DrawWindow("Packet", memview, memviewsize);        


        ImGui::Begin("I/O");

           if (ImGui::Button("OSC Send Test")) {
               std::string sendstr = "/button";
               sendosc(sendstr.c_str(),1);
           }

        //    if (ImGui::Button("Connect to FlightGear")) {
        //             // setup telnet to flightgear
        //     fs = new FGFSSocket (HOST, PORT);
        //     fs->flush();     
        //    }
           
        ImGui::End();


        // ImGui::Begin("XOR Table");

        // static char inphex[3];
        // static int inv;
        // ImGui::InputText("Input HEX",(char*)inphex,3,ImGuiInputTextFlags_CharsHexadecimal);
        // sscanf(inphex,"%x",&inv);

        // for (int cntro = 0; cntro < 255 ; cntro++){
        //             ImGui::Text("%x:%x ",cntro, inv ^ cntro);
        //             if (cntro % 16 == 0) ImGui::NewLine();
        //             ImGui::SameLine();
        //         }

        // ImGui::End();





        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

// can

    quit = true;

//    canrx_thread_h.join();
//    if (close(s) < 0) {
//		perror("CAN Close");
//	} else { 
//        printf("CAN Closed\n");
//    }



    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();



    return 0;
}
