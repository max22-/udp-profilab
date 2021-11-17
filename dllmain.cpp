#include "dll.h"
#include <winsock2.h>
#include <stdarg.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")


#define UI_WINDOWS
#define UI_IMPLEMENTATION
extern "C" {
#include "luigi.h"
}


const unsigned char inputs = 16;
const unsigned char outputs = 16;
const unsigned short PORT = 1234;
enum {
  PUSER_INPUT,
  PUSER_OUTPUT,
  PUSER_TIMESTAMP,
  PUSER_IP0,
  PUSER_IP1,
  PUSER_IP2,
  PUSER_IP3
};
double SEND_INTERVAL = 5; // seconds
SOCKET s;
struct sockaddr_in sin;

  
const unsigned char CLK = 0;
const unsigned char RST = 1;
//Bezeichnung für User-variabelen
const unsigned char CLK_OLD = 0;
const unsigned char RST_OLD = 1;
const unsigned char COUNT = 2;

#define STRING_IP(string, puser) { sprintf(string, "%d.%d.%d.%d", (int)puser[PUSER_IP0], (int)puser[PUSER_IP1], (int)puser[PUSER_IP2], (int)puser[PUSER_IP3]); }

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
  char host_ip[256];
  
  STRING_IP(host_ip, PUser);
  
  WSAStartup(MAKEWORD(2,2),&wsa);
  s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  memset((char *) &sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);
  sin.sin_addr.S_un.S_addr = inet_addr(host_ip);
  
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

static UITextbox *textbox;

int SaveIP(UIElement *button, UIMessage message, int di, void *dp) {
  int ip[4];
  double *PUser = (double*)button->cp;
  
  if (message == UI_MSG_CLICKED) {
    sscanf(textbox->string, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    PUser[PUSER_IP0] = ip[0];
    PUser[PUSER_IP1] = ip[1];
    PUser[PUSER_IP2] = ip[2];
    PUser[PUSER_IP3] = ip[3];
    //debug("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  }
  
  return 0;
}

DLLEXPORT void _stdcall CConfigure(double *PUser)
{
  UILabel *label;
  UIButton *button;
  char buffer[256];

         
  UIInitialise();
  
  UIWindow *window = UIWindowCreate(0, 0, "Configuration", 320, 170);
  UIPanel *panel = UIPanelCreate(&window->e, UI_PANEL_GRAY | UI_PANEL_MEDIUM_SPACING);
  label = UILabelCreate(&panel->e, 0, "Adresse IP ESP32 :", -1);
  textbox = UITextboxCreate(&panel->e, 0);

  STRING_IP(buffer, PUser);
  UITextboxClear(textbox, false);
  UITextboxReplace(textbox, buffer, strlen(buffer), false);
  button = UIButtonCreate(&panel->e, 0, "Enregistrer", -1);
  button->e.messageUser = SaveIP;
  button->e.cp = PUser;
  UIMessageLoop();
  DestroyWindow(window->hwnd);
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
