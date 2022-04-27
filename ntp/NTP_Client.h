#include <winsock2.h>
#include <cstdio>
#define NTP_Port 123
#define NTP_PACKET_SIZE 48
#define WIN_TO_UNIX 116444736000000000LL //1601与1970的时间间隔
#define NTP_TO_UNIX 22089888000000000LL //1900与1970的时间间隔

enum NTP_Client_State{
    Uninitialized,
    SocketCreated,
    SocketEdited,
    NTPServerAddrSet,
    RequestSent
};
class NTP_Client{
    public:
        bool init();
        int getState();
        bool setNTPServerAddr(char*);
        bool sendNTPRequest();
        bool receiveNTPResponse();
    private:
        SOCKET sock;
        SOCKADDR_IN NTPServerAddr;
        NTP_Client_State state = Uninitialized;
        bool createSocket();
        bool setSocketFeatures();
        byte sendBuffer[NTP_PACKET_SIZE];
        byte receiveBuffer[NTP_PACKET_SIZE];
        time_t t1;
        time_t t2;
        time_t t3;
        time_t t4;
};