#ifndef __serial_h__
#define __serial_h__

#include <stdlib.h>
#include <stdint.h>
#include <string>

#define LINUX


#if defined(LINUX)
#include <termios.h>
#elif defined(MACOSX)
#include <termios.h>
#elif defined(WINDOWS)
#include <windows.h>
#endif

class Serial
{
public:
	Serial();
	~Serial();
	//wxArrayString port_list();
	int Open(const std::string& name);
	std::string error_message();
	int Set_baud(int baud);
	int Set_baud(const std::string& baud_str);
	int Read(void *ptr, int count);
	int Write(const void *ptr, int len);
	int Input_wait(int msec);
	void Input_discard(void);
	int Set_control(int dtr, int rts);
	void Output_flush();
	void Close(void);
	int Is_open(void);
	std::string get_name(void);
private:
	int port_is_open;
	std::string port_name;
	int baud_rate;
	std::string error_msg;
private:
#if defined(LINUX) || defined(MACOSX)
	int port_fd;
	struct termios settings_orig;
	struct termios settings;
#elif defined(WINDOWS)
	HANDLE port_handle;
	COMMCONFIG port_cfg_orig;
	COMMCONFIG port_cfg;
#endif
};

#endif // __serial_h__
