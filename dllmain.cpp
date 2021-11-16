#include "dll.h"
#include <winsock2.h>
#include <stdarg.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")


const unsigned char inputs = 16;
const unsigned char outputs = 16;
const char *HOST = "192.168.4.1";
const unsigned short PORT = 1234;
const unsigned char PUSER_INPUT = 0;
const unsigned char PUSER_OUTPUT = 1;
const unsigned char PUSER_TIMESTAMP = 2;
double SEND_INTERVAL = 5; // seconds
SOCKET s;
struct sockaddr_in sin;

  
const unsigned char CLK = 0;
const unsigned char RST = 1;
//Bezeichnung für User-variabelen
const unsigned char CLK_OLD = 0;
const unsigned char RST_OLD = 1;
const unsigned char COUNT = 2;

void debug(const char *fmt, ...)
{
  va_list va;
  char buffer[1024];
  va_start(va, fmt);
  vsnprintf(buffer, 1024, fmt, va);
  va_end(va);
  MessageBox(NULL,TEXT(buffer),TEXT("Debug"), MB_OK);
}

int data_ready(SOCKET socket, long ms)
{
    struct timeval timeout;
    struct fd_set fds;

    timeout.tv_sec = 0;

    timeout.tv_usec = ms*1000;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);

    // Possible return values:

    // -1: error occurred

    // 0: timed out

    // > 0: data ready to be read

    return select(0, &fds, 0, 0, &timeout);

}

DLLEXPORT unsigned char _stdcall NumInputs()
{
  return inputs;
}

DLLEXPORT unsigned char _stdcall NumOutputs()
{
  return outputs;
}

DLLEXPORT void _stdcall GetInputName(unsigned char channel, unsigned char *name)
{
  sprintf((char*)name, "I%d", channel);
}


DLLEXPORT void _stdcall GetOutputName(unsigned char channel, unsigned char *name)
{
    sprintf((char*)name, "O%d", channel);
}


DLLEXPORT void _stdcall CSimStart(double *PInput, double *POutput, double *PUser)
{
	WSADATA wsa;

	WSAStartup(MAKEWORD(2,2),&wsa);
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset((char *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.S_un.S_addr = inet_addr(HOST);
	
	PUser[PUSER_INPUT] = 0;
	PUser[PUSER_OUTPUT] = 0;

    for(int i = 0; i < outputs; i++)
        POutput[i] = 0;
    
    
    time_t timestamp;
    time(&timestamp);
    PUser[PUSER_TIMESTAMP] = (double)timestamp;
}


DLLEXPORT void _stdcall CCalculate(double *PInput, double *POutput, double *PUser)
{
  unsigned short input = 0, output = 0;
  int slen = sizeof(sin);
  time_t timestamp;
  double elapsed_time;

  time(&timestamp);
  elapsed_time = difftime(timestamp, (time_t)PUser[PUSER_TIMESTAMP]);
  
  for(int i = 0; i < 16; i++)
    input |= (PInput[i] > 2.5) << i;

  if((input != (unsigned short)PUser[PUSER_INPUT]) || elapsed_time > SEND_INTERVAL) {
    sendto(s, (char*)&input, 2 , 0 , (struct sockaddr *) &sin, slen);
    PUser[PUSER_INPUT] = (double) input;
    PUser[PUSER_TIMESTAMP] = (double)timestamp;
  }
  
  if(data_ready(s, 0) > 0) {
    if(recvfrom(s, (char*)&output, 2, 0, (struct sockaddr *) &sin, &slen) != SOCKET_ERROR) {
      for(int i = 0; i < 16; i++) {
        if(output & (1 << i))
          POutput[i] = 5.0;
        else
          POutput[i] = 0.0;
      }
    }
  }

}

DLLEXPORT void _stdcall CSimStop(double *PInput, double *POutput, double *PUser)
{
    closesocket(s);
	WSACleanup();
}
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szComdLine, int iCmdShow);  

DLLEXPORT void _stdcall CConfigure(double *PUser)
{
  MessageBox(NULL,TEXT("nichts zu configurieren"),TEXT("Config"), MB_OK);
}

BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      break;
      
    case DLL_PROCESS_DETACH:
      break;
      
    case DLL_THREAD_ATTACH:
      break;
      
    case DLL_THREAD_DETACH:
      break;
    }
  
  /* Returns TRUE on success, FALSE on failure */
  return TRUE;
}
