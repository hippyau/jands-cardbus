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

//#include <linux/can.h>
//#include <linux/can/raw.h>
#include "serial.h"

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
#include "tinyxml2.h"
#include <algorithm> 
#include <cctype>
#include <locale>


  
Serial port;
typedef struct {
	uint8_t mode;
	uint8_t analog_channel;
	uint64_t supported_modes;
	uint32_t value;
} pin_t;
pin_t pin_info[128];
std::string firmata_name;
unsigned int rx_count, tx_count;
int parse_count;
int parse_command_len;
uint8_t parse_buf[4096];

uint8_t dport_in = 0;

#define MODE_INPUT    0x00
#define MODE_OUTPUT   0x01
#define MODE_ANALOG   0x02
#define MODE_PWM      0x03
#define MODE_SERVO    0x04
#define MODE_SHIFT    0x05
#define MODE_I2C      0x06
#define MODE_ONEWIRE  0x07
#define MODE_STEPPER  0x08
#define MODE_ENCODER  0x09
#define MODE_SERIAL   0x0A
#define MODE_PULLUP   0x0B
#define MODE_IGNORE   0x7F

#define START_SYSEX             0xF0 // start a MIDI Sysex message
#define END_SYSEX               0xF7 // end a MIDI Sysex message
#define PIN_MODE_QUERY          0x72 // ask for current and supported pin modes
#define PIN_MODE_RESPONSE       0x73 // reply with current and supported pin modes
#define PIN_STATE_QUERY         0x6D
#define PIN_STATE_RESPONSE      0x6E
#define CAPABILITY_QUERY        0x6B
#define CAPABILITY_RESPONSE     0x6C
#define ANALOG_MAPPING_QUERY    0x69
#define ANALOG_MAPPING_RESPONSE 0x6A
#define REPORT_FIRMWARE         0x79 // report name and version of the firmware


void init_firmata_data(void)
{
	for (int i=0; i < 128; i++) {
		pin_info[i].mode = 255;
		pin_info[i].analog_channel = 127;
		pin_info[i].supported_modes = 0;
		pin_info[i].value = 0;
	}
	tx_count = rx_count = 0;
	firmata_name = "";	
}




// // set pin to MODE_OUTPUT, MODE_INPUT, MODE_ANALOG, MODE_PWM, MODE_SERVO
// void comPinModeChage(uint8_t pin, uint8_t mode)
// {
//     if (pin < 0 || pin > 127)
//         return;
//     //	wxChoice *ch = (wxChoice *)FindWindowById(id, scroll);
//     //	wxString sel = ch->GetStringSelection();
//     printf("Mode Change, pin=%d, mode=%d", pin, mode);
//     //	printf("Mode = %s\n", (const char *)sel);
//     //	int mode = 255;
//     // if (sel.IsSameAs("Input")) mode = MODE_INPUT;
//     // if (sel.IsSameAs("Pullup")) mode = MODE_PULLUP;
//     // if (sel.IsSameAs("Output")) mode = MODE_OUTPUT;
//     // if (sel.IsSameAs("Analog")) mode = MODE_ANALOG;
//     // if (sel.IsSameAs("PWM")) mode = MODE_PWM;
//     // if (sel.IsSameAs("Servo")) mode = MODE_SERVO;
//     if (mode != pin_info[pin].mode)
//     {
//         // send the mode change message
//         uint8_t buf[4];
//         buf[0] = 0xF4;
//         buf[1] = pin;
//         buf[2] = mode;
//         port.Write(buf, 3);
//         tx_count += 3;
//         pin_info[pin].mode = mode;
//         pin_info[pin].value = 0;
//     }
// }



void comSetup(){

	std::string name = "/dev/ttyACM0";
	port.Close();
    init_firmata_data();
	printf("com_setup: name = %s\n", (const char *)name.c_str());

	//if (id == 9000) return; // none

	if (!port.Open(name)){
        printf("%s",port.error_message().c_str());
    }
	port.Set_baud(57600);
	if (port.Is_open()) {
        
		printf("port is open\n");
		firmata_name = "";
		rx_count = tx_count = 0;
		parse_count = 0;
		parse_command_len = 0;
        sleep(0.5);		
		/* 
		The startup strategy is to open the port and immediately
		send the REPORT_FIRMWARE message.  When we receive the
		firmware name reply, then we know the board is ready to
		communicate.

		For boards like Arduino which use DTR to reset, they may
		reboot the moment the port opens.  They will not hear this
		REPORT_FIRMWARE message, but when they finish booting up
		they will send the firmware message.

		For boards that do not reboot when the port opens, they
		will hear this REPORT_FIRMWARE request and send the
		response.  If this REPORT_FIRMWARE request isn't sent,
		these boards will not automatically send this info.

		Arduino boards that reboot on DTR will act like a board
		that does not reboot, if DTR is not raised when the
		port opens.  This program attempts to avoid raising
		DTR on windows.  (is this possible on Linux and Mac OS-X?)

		Either way, when we hear the REPORT_FIRMWARE reply, we
		know the board is alive and ready to communicate.
		*/
		// uint8_t buf[3];
		// buf[0] = START_SYSEX;
		// buf[1] = REPORT_FIRMWARE; // read firmata name & version
		// buf[2] = END_SYSEX;        
		// port.Write(buf, 3);
		// tx_count += 3;	        	

        // for (int cnt = 0; cnt < 32; cnt++){     
        //    comPinModeChage(cnt,MODE_OUTPUT);   
        // } 

	} else {
		printf("error opening port\n");
	}
}



void comDoMessage(void)
{
	uint8_t cmd = (parse_buf[0] & 0xF0);

	printf("message, %d bytes, %02X\n", parse_count, parse_buf[0]);

	if (cmd == 0xE0 && parse_count == 3) {
		int analog_ch = (parse_buf[0] & 0x0F);
		int analog_val = parse_buf[1] | (parse_buf[2] << 7);
		for (int pin=0; pin<128; pin++) {
			if (pin_info[pin].analog_channel == analog_ch) {
				pin_info[pin].value = analog_val;
				printf("pin %d is A%d = %d\n", pin, analog_ch, analog_val);
				//wxStaticText *text = (wxStaticText *)
				//  FindWindowById(5000 + pin, scroll);
				//if (text) {
				//	wxString val;
				//	val.Printf("A%d: %d", analog_ch, analog_val);
				//	text->SetLabel(val);
				//}
				return;
			}
		}
		return;
	}

	if (cmd == 0x90 && parse_count == 3) {
		int port_num = (parse_buf[0] & 0x0F);
		int port_val = parse_buf[1] | (parse_buf[2] << 7);
		int pin = port_num * 8;
		printf("port_num = %d, port_val = %d\n", port_num, port_val);
        if (port_num == 0) dport_in = port_val;

		for (int mask=1; mask & 0xFF; mask <<= 1, pin++) {
			if (pin_info[pin].mode == MODE_INPUT || pin_info[pin].mode == MODE_PULLUP) {
				uint32_t val = (port_val & mask) ? 1 : 0;
				if (pin_info[pin].value != val) {
					printf("pin %d is %d\n", pin, val);
					//wxStaticText *text = (wxStaticText *)
					//  FindWindowById(5000 + pin, scroll);
					//if (text) text->SetLabel(val ? "High" : "Low");
					pin_info[pin].value = val;
				}
			}
		}
		return;
	}


	if (parse_buf[0] == START_SYSEX && parse_buf[parse_count-1] == END_SYSEX) {
		// Sysex message
		if (parse_buf[1] == REPORT_FIRMWARE) {
			char name[140];
			int len=0;
			for (int i=4; i < parse_count-2; i+=2) {
				name[len++] = (parse_buf[i] & 0x7F)
				  | ((parse_buf[i+1] & 0x7F) << 7);
			}
			name[len++] = '-';
			name[len++] = parse_buf[2] + '0';
			name[len++] = '.';
			name[len++] = parse_buf[3] + '0';
			name[len++] = 0;
			firmata_name = name;
			// query the board's capabilities only after hearing the
			// REPORT_FIRMWARE message.  For boards that reset when
			// the port open (eg, Arduino with reset=DTR), they are
			// not ready to communicate for some time, so the only
			// way to reliably query their capabilities is to wait
			// until the REPORT_FIRMWARE message is heard.
			uint8_t buf[80];
			len=0;
			buf[len++] = START_SYSEX;
			buf[len++] = ANALOG_MAPPING_QUERY; // read analog to pin # info
			buf[len++] = END_SYSEX;
			buf[len++] = START_SYSEX;
			buf[len++] = CAPABILITY_QUERY; // read capabilities
			buf[len++] = END_SYSEX;
			for (int i=0; i<16; i++) {
				buf[len++] = 0xC0 | i;  // report analog
				buf[len++] = 1;
				buf[len++] = 0xD0 | i;  // report digital
				buf[len++] = 1;
			}
			port.Write(buf, len);
			tx_count += len;
		} else if (parse_buf[1] == CAPABILITY_RESPONSE) {
			int pin, i, n;
			for (pin=0; pin < 128; pin++) {
				pin_info[pin].supported_modes = 0;
			}
			for (i=2, n=0, pin=0; i<parse_count; i++) {
				if (parse_buf[i] == 127) {
					pin++;
					n = 0;
					continue;
				}
				if (n == 0) {
					// first byte is supported mode
					pin_info[pin].supported_modes |= (1<<parse_buf[i]);
				}
				n = n ^ 1;
			}
			// send a state query for for every pin with any modes
			for (pin=0; pin < 128; pin++) {
				uint8_t buf[512];
				int len=0;
				if (pin_info[pin].supported_modes) {
					buf[len++] = START_SYSEX;
					buf[len++] = PIN_STATE_QUERY;
					buf[len++] = pin;
					buf[len++] = END_SYSEX;
				}
				port.Write(buf, len);
				tx_count += len;
			}
		} else if (parse_buf[1] == ANALOG_MAPPING_RESPONSE) {
			int pin=0;
			for (int i=2; i<parse_count-1; i++) {
				pin_info[pin].analog_channel = parse_buf[i];
				pin++;
			}
			return;
		} else if (parse_buf[1] == PIN_STATE_RESPONSE && parse_count >= 6) {
			int pin = parse_buf[2];
			pin_info[pin].mode = parse_buf[3];
			pin_info[pin].value = parse_buf[4];
			if (parse_count > 6) pin_info[pin].value |= (parse_buf[5] << 7);
			if (parse_count > 7) pin_info[pin].value |= (parse_buf[6] << 14);
			//add_pin(pin); -- TO DO
		}
		return;
	}
}







void comParse(const uint8_t *buf, int len)
{
	const uint8_t *p, *end;

	p = buf;
	end = p + len;
	for (p = buf; p < end; p++) {
		uint8_t msn = *p & 0xF0;
		if (msn == 0xE0 || msn == 0x90 || *p == 0xF9) {
			parse_command_len = 3;
			parse_count = 0;
		} else if (msn == 0xC0 || msn == 0xD0) {
			parse_command_len = 2;
			parse_count = 0;
		} else if (*p == START_SYSEX) {
			parse_count = 0;
			parse_command_len = sizeof(parse_buf);
		} else if (*p == END_SYSEX) {
			parse_command_len = parse_count + 1;
		} else if (*p & 0x80) {
			parse_command_len = 1;
			parse_count = 0;
		}
		if (parse_count < (int)sizeof(parse_buf)) {
			parse_buf[parse_count++] = *p;
		}
		if (parse_count == parse_command_len) {
			comDoMessage();
			parse_count = parse_command_len = 0;
		}
	}
}



// void comUpdate(void)
// {
// 	uint8_t buf[1024];
// 	int r;
    

// 	//printf("Idle event\n");
// 	r = port.Input_wait(40);
// 	if (r > 0) {
//         printf("rx=%d\n",r);
// 		r = port.Read(&buf[0], sizeof(buf));
// 		if (r < 0) {
// 			// error
// 			return;
// 		}
// 		if (r > 0) {
// 			// parse
// 			rx_count += r;
// 			for (int i=0; i < r; i++) {
// 				printf("%02X ", buf[i]);
// 			}
// 			//printf("\n");
// 			comParse(buf, r);			
// 		}
// 	} else if (r < 0) {
// 		return;
// 	}	
// }



void comUpdate(void)
{
	uint8_t buf[1024];
	int r;
  
	//printf("Idle event\n");
	r = port.Input_wait(0);

	if (r > 0) {
        printf("rx=%d\n",r);
		r = port.Read(&buf[0], 1);
		if (r < 0) {
			// error
			return;
		}
		if (r > 0) {
		  	dport_in = buf[0];	
              rx_count += 1;
		}
	} else if (r < 0) {
		return;
	}	
}


// // set pin 1 or 0
// void comSetPin(uint8_t pin, bool val)
// {
//     if (pin < 0 || pin > 127)
//         return;
//      printf("Mode Change, pin=%d, val=%d", pin, val);
 
//     // send the pin message
//         uint8_t buf[4];
//         buf[0] = 0xF5;
//         buf[1] = pin;
//         buf[2] = val;
//         port.Write(buf, 3);
//         tx_count += 3;
//         //pin_info[pin].mode = mode;
//         //pin_info[pin].value = 0;
    
// }




// // set pins 0-7 as input or output
// void setDataBusDirection(bool rw){
//     for (int cnt = 0; cnt < 8; cnt++){
//         if (rw) {
//             comPinModeChage(cnt,MODE_OUTPUT);
//         } else {
//             comPinModeChage(cnt,MODE_INPUT);
//         }
//     } 
// }



// // write a byte to the databus
// // makes the bus go output
// void writeDataBusData(uint8_t data)
// { // setDataBusDirection(1); // set pins to outputs  
//     uint8_t buf[3];
//     buf[0] = 0x90 | 0;
//     buf[1] = data & 0x7F;
//     buf[2] = (data >> 7) & 0x7F;
//     port.Write(buf, 3);
//     tx_count += 3;
// }

// // write a byte to the control bus signals port
// void writeBusControl(uint8_t data)
// {    
//     uint8_t buf[3];
//     buf[0] = 0x90 | 1;
//     buf[1] = data & 0x7F;
//     buf[2] = (data >> 7) & 0x7F;
//     port.Write(buf, 3);
//     tx_count += 3;
// }



// write one bus and one control byte...
void write_duino(uint8_t datab, uint8_t datac)
{    
    uint8_t buf[2];
    buf[0] = datab;
    buf[1] = datac & 0x7F;   
    port.Write(buf, 2);   
    tx_count += 2;
}




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



// OSC
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

#define OSCADDRESS "192.168.1.33"
#define OSCOUTPORT 3819
#define OSC_BUFFER_SIZE 1024

UdpTransmitSocket * transmitSocket;






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


using namespace tinyxml2;
using namespace std;

// storage
// takes in filename
// returns 1 on error
int store_xml(std::string outfile)
{
    FILE * pFile;
    pFile = fopen (outfile.c_str(),"w");
    if (pFile!=NULL)
    {

    XMLPrinter printer (pFile);
    printer.OpenElement( "firmata-file" );
    printer.PushAttribute( "ver", "1.0" );

    printer.PushComment("Leonardo");
            
   
    printer.CloseElement(); // ma-can-map-file
        
    fclose (pFile);
    return 0;
    } 
 
    perror("Could not open XML output file");
    return 1;
}


// load map with data from XML file
int load_xml(std::string infile)
{

    XMLDocument doc;
    XMLError err = doc.LoadFile(infile.c_str());

    if (err == XML_SUCCESS)
    {

        XMLNode *root = doc.FirstChildElement("firmata-file");
        if (root == nullptr)
            return false;
       
        return XML_SUCCESS;
    }

    perror("Could not open XML input");
    return err; // failed
}


int * memview = NULL;
int memviewsize = 0;
static MemoryEditor mem_edit1;
  

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

    printf("CANF> %s %f\n",param, value);

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







void com_rx_thread(void)
{
    printf(" rx thread started\n");

    while (!quit)
    {
      //  nbytes = read(s, &frame, sizeof(struct can_frame));      
        //if (nbytes <= 0)
       // {
            //perror("Read");
//            continue;
        //}
//        else
//        {
//        }  
     sleep(1); 
    }

    printf("rx thread exited.\n");
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



// Main code
int main(int argc, char* argv[])
{
   auto start = chrono::steady_clock::now();

 // setup OSC
    transmitSocket = new UdpTransmitSocket ( IpEndpointName( OSCADDRESS, OSCOUTPORT ) );

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    comSetup();


    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("jandshands", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, window_flags);
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

    // Our state

    ImVec4 clear_color = ImVec4(0.0f, 0.00f, 0.60f, 1.00f);


    static bool show_demo_window = true;

    static uint8_t in_d = 0xaa; // data in
    static uint8_t in_r = 0x55; // data to send out
    static uint8_t out_d = 0x00; // control signals
    uint8_t old_out_d = 0x00;
    uint8_t old_in_r = 0x00;


    // start the RX thread
//    std::thread com_rx_thread_h(canrx_thread);

    
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


      
       // _gCOMRX_mutex.lock();

ImGui::Begin("PIN Data");
            in_d = dport_in;


            ImGui::Columns(2);
            ImGui::Text("Data Bus --> %dd %sh (%sb)",in_r, n2hexstr(in_r).c_str(), FormatBinary(&in_r,8));
            ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(255,0,0,255));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(32,0,0,255));
            if (ImGui::RadioButton("D0",in_r & 1)) in_r ^= 1;
            if (ImGui::RadioButton("D1",in_r & (1<<1))) in_r ^= (1<<1);
            if (ImGui::RadioButton("D2",in_r & (1<<2))) in_r ^= (1<<2);
            if (ImGui::RadioButton("D3",in_r & (1<<3))) in_r ^= (1<<3);
            if (ImGui::RadioButton("D4",in_r & (1<<4))) in_r ^= (1<<4);
            if (ImGui::RadioButton("D5",in_r & (1<<5))) in_r ^= (1<<5);
            if (ImGui::RadioButton("D6",in_r & (1<<6))) in_r ^= (1<<6);
            if (ImGui::RadioButton("D7",in_r & (1<<7))) in_r ^= (1<<7);
            if (old_in_r != in_r) {
                //writeDataBusData(in_r);
                write_duino(in_r,out_d);
                old_in_r = in_r;
            }
            
            ImGui::PopStyleColor(2);
            ImGui::NextColumn();

            ImGui::Text("Data Bus <-- %dd %sh (%sb)",in_d, n2hexstr(in_d).c_str(), FormatBinary(&in_d,8));
            ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0,255,0,255));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0,32,0,255));            
            ImGui::RadioButton("I0",in_d & 1);
            ImGui::RadioButton("I1",in_d & (1<<1));
            ImGui::RadioButton("I2",in_d & (1<<2));
            ImGui::RadioButton("I3",in_d & (1<<3));
            ImGui::RadioButton("I4",in_d & (1<<4));
            ImGui::RadioButton("I5",in_d & (1<<5));
            ImGui::RadioButton("I6",in_d & (1<<6));
            ImGui::RadioButton("I7",in_d & (1<<7));
            ImGui::PopStyleColor(2);
            ImGui::NextColumn();                        
            ImGui::Columns(1);

            ImGui::NewLine();

            ImGui::Text("Bus Signals -> %dd %sh (%sb)",out_d, n2hexstr(out_d).c_str(), FormatBinary(&out_d,8));
          
           ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(255,0,0,255));
           ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(32,0,0,255));
           
            
            if (ImGui::RadioButton("DIR (Master)",out_d & 1)){
                out_d ^= 1;  
                
                // if ((out_d & 1)) {
                //     writeDataBusData(in_r);                  
                // }
                // setDataBusDirection(out_d & 1); // change direction
                    

            };
            if (ImGui::RadioButton("BUF (Master)",out_d & (1<<1))){
                out_d ^= (1<<1);  
                          
            };
            ImGui::NewLine();

           
            if (ImGui::RadioButton("MUX",out_d & (1<<2))){
                out_d ^= (1<<2);  
                          
            };
            if (ImGui::RadioButton("ALE",out_d & (1<<3))){
                out_d ^= (1<<3);                            
            };
            if (ImGui::RadioButton("DS",out_d & (1<<4))){
                out_d ^= (1<<4);  
                          
            };
            if (ImGui::RadioButton("RW",out_d & (1<<5))){
                out_d ^= (1<<5);                            
            };
            
            ImGui::PopStyleColor(2);

            ImGui::NewLine();

            if (old_out_d != out_d) {
                write_duino(in_r,out_d);
                old_out_d = out_d;
            }

            

        // list of PIN's  
     /*    for (std::map<unsigned int, t_candata>::iterator it = _gCANinput.begin(); it != _gCANinput.end(); it++)
        {
            ImGui::PushID(it->first);
            if (ImGui::Button("->")){
                //t_candata * pData = NULL;   
                //pData = & it->second ;
                //memview = (int*)pData;
                //memviewsize = it->second.len * 3;             
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::Text("0x%03X [%d] ", it->first, it->second.len);
            
            ImGui::SameLine();
            for (int i = 0; i < it->second.len; i++)
            {
              if (init) {
                // ImGui::Text("%02X ", it->second.data[i]);
                // if (ImGui::IsItemHovered()){
                // ImGui::BeginTooltip();
                // // XOR table
                // for (int cntro = 0; cntro < 255 ; cntro++){
                //     ImGui::Text("%x:%x ",cntro,it->second.data[i] ^ cntro);
                //     if (cntro % 16 == 0) ImGui::NewLine();
                //     ImGui::SameLine();
                // }
                // ImGui::EndTooltip();
                // }

              }                
              else { 
                if (it->second.chgf[i] > 0){
                 ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, it->second.chgf[i] * 2 ,0,255));
                }

                ImGui::Text("%02X ", it->second.xord[i]);

               if (it->second.chgf[i] > 0){
                 ImGui::PopStyleColor();
                 it->second.chgf[i] -= 1;
                }

              }
                
              if (ImGui::IsItemClicked())
               {
              //  pEditID = &it->second;
              //  pEditByte = i;
                
              //  ImGui::OpenPopup("Edit Byte");
               }

               if (i != it->second.len-1)
                 ImGui::SameLine();
            }
            
        } */


/*         if (ImGui::BeginPopupModal("Edit Byte", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
            {
                t_candata * pEditor = pEditID;
                uint8_t EditByte = pEditByte;
              //  static int bType = 0;

                ImGui::InputText("Byte Name", (char*)pEditor->bytenames[EditByte].c_str(),pEditor->bytenames[EditByte].capacity());
                ImGui::Combo("Byte Type",&pEditor->type[EditByte], ByteTypeNames, IM_ARRAYSIZE(ByteTypeNames));      
                ImGui::Separator();
                ImGui::Text("Init:"); ImGui::SameLine();
                ImGui::Text(FormatBinary(&pEditor->init[EditByte],8));
                ImGui::Text("Recv:"); ImGui::SameLine();
                ImGui::Text(FormatBinary(&pEditor->data[EditByte],8));
                ImGui::Text("Value:"); ImGui::SameLine();
                ImGui::Text(FormatBinary(&pEditor->xord[EditByte],8));              
                ImGui::Separator();

                // names of individual bits
                for (int cnti=0;cnti<8;cnti++){
                 
                 std::string v = "Bit "+ std::to_string(cnti) + " = ";
                 v += std::to_string( ((pEditor->xord[EditByte] >> cnti) & 0x01) ? 1 : 0 );
                 ImGui::Text((char*)v.c_str()); 
                 ImGui::SameLine();
                 ImGui::PushID(cnti);
                 ImGui::InputText("###bitname",(char*)pEditor->lbl[EditByte][cnti][0].c_str(),8);
                 ImGui::PopID();
                }

                if (ImGui::Button("Close", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            } */
ImGui::End();



        // ImGui::Begin("Faders");

        // const float spacing = 4;
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
        // ImGui::PushID("faderset1");
        // for (int i = 1; i < 16; i++)
        // {
        //     ImGui::PushID(i);

        //     if (ImGui::VSliderInt("##int", ImVec2(20, 160), &Faders[i], 0, 255)){
        //      //  SendFader(i,Faders[i]);
        //     }
        //     ImGui::SameLine();
        //     ImGui::PopID();
        // }

        // ImGui::PopID();
        // ImGui::PopStyleVar(1);

        // ImGui::End();





       
       
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {           
            ImGui::Begin("Firmata UI");                
                ImGui::Text("written by Hippy (2020)");            
                ImGui::NewLine();

                ImGui::Text("Port: %s",port.get_name().c_str()); 
                ImGui::SameLine();

if (ImGui::Button("Re-open")){
                    	comSetup();
                }

                if (ImGui::Button("Request Version")){
                    	uint8_t buf[3];
                        buf[0] = START_SYSEX;
                        buf[1] = REPORT_FIRMWARE; // read firmata name & version
                        buf[2] = END_SYSEX;
                        port.Write(buf, 3);                        
                        tx_count += 3;
                }

                ImGui::Text("Tx:%u Rx:%u",tx_count, rx_count);
                ImGui::NewLine();
                ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


        // memory editor
        mem_edit1.DrawWindow("Packet", memview, memviewsize);
        

        ImGui::Begin("I/O");
           if (ImGui::Button("OSC Send Test")) {
               std::string sendstr = "/button";
               sendosc(sendstr.c_str(),1);
           }



        ImGui::End();


       




        // Polling
        //        auto now = chrono::steady_clock::now();
        //        if (chrono::duration_cast<chrono::milliseconds>(now-start).count() > 20){

//         start = chrono::steady_clock::now();
//        }

  comUpdate();  // handle firmata RX

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }


    quit = true;

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();



    return 0;
}
