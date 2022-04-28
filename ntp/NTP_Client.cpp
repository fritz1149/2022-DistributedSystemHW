#include "NTP_Client.h"

bool NTP_Client::init(){
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);

    if(!createSocket() || !setSocketFeatures())
        return false;
    // createSocket();
    return true;
}

int NTP_Client::getState(){
    return state;
}

bool NTP_Client::createSocket(){
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET)
        return false;

    state = SocketCreated;
    return true;
}

bool NTP_Client::setSocketFeatures(){
    // 非阻塞模式
	// unsigned long mode = 1;
	// if(ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
	// 	return false;

    // 设定超时时间500ms，代替原来的500ms后执行非阻塞recv的策略
    // 
    int timeout = 500;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int)) == SOCKET_ERROR)
        return false;

    state = SocketEdited;
	return true;
}

bool NTP_Client::setNTPServerAddr(char *url){
    hostent *host = gethostbyname(url);
    if(host == nullptr)
        return false;
    NTPServerAddr.sin_addr = *(struct in_addr*)host->h_addr_list[0];
	NTPServerAddr.sin_family = AF_INET;
	NTPServerAddr.sin_port = htons(NTP_Port); 

    state = NTPServerAddrSet;
    return true;
}

// 利用windows的100纳秒级精度的API获取时间
// 这个时间是从1601年1月1号开始的
inline void get_now(time_t *ret){
    FILETIME time;
    GetSystemTimeAsFileTime(&time);
    *ret = (time_t)time.dwLowDateTime +
                       (((time_t)time.dwHighDateTime) << 32);
    return;
}

bool NTP_Client::sendNTPRequest(){
	memset(sendBuffer, 0, NTP_PACKET_SIZE);
	sendBuffer[0] = 0xE3;

    // 取得当前时间为t1
    get_now(&t1);

	if(sendto(sock, (const char*)sendBuffer, sizeof(sendBuffer), 0,
        (SOCKADDR*)&NTPServerAddr, sizeof(NTPServerAddr)) == INVALID_SOCKET)
        return false;
    state = RequestSent;
    return true;
}

time_t bytes2time(byte *a){
    time_t ret = 0;
    for(int i = 0; i < 8; i++)
        ret = (ret << 8) + a[i];
    return ret;
}

time_t time_change(byte *a){
    time_t high = 0, low0 = 0;
    for(int i = 0; i < 4; i++)
        high = (high << 8) + a[i];
    high *= 10000000;

    for(int i = 0; i < 4; i++)
        low0 = (low0 << 8) + a[i + 4];
    low0 *= 10000000;

    double low = 1.0 * low0;
    time_t factor = 1LL << 32;
    low /= factor;
    
    high += (int)low;
    return high;
}


struct NTP_Timestamp_Data {		
	unsigned long UnixTime;	// (100 nano)Seconds since 1970 (secsSince1900 - seventyYears)
	unsigned long Hour;
	unsigned long Minute;
	unsigned long Second;
    unsigned long MilliSecond;
    unsigned long MicroSecond;
    NTP_Timestamp_Data(time_t raw_time){
        UnixTime = raw_time / 10000000;
        Hour = (UnixTime % 86400L) / 3600;
        Minute = (UnixTime % 3600) /60;
        Second = UnixTime % 60;
        MilliSecond = (raw_time % 10000000) / 10000;
        MicroSecond = (raw_time % 10000) / 10;
    }
    void output(){
        printf("\nHour: %ld, Minute: %ld, Second: %ld, MilliSecond: %ld, MicroSecond: %ld\n\n", 
            Hour, Minute, Second, MilliSecond, MicroSecond);
    }
};

bool NTP_Client::receiveNTPResponse(){
    state = NTPServerAddrSet;
    if(recvfrom(sock, (char *)receiveBuffer, 
        NTP_PACKET_SIZE, 0, NULL, NULL) == SOCKET_ERROR)
        return false;
    // 取得当前时间为t4
    get_now(&t4);

    // 统一转换成以100纳秒为单位的，unix标准下时间（从1970年1月1号开始）
    t2 = time_change(receiveBuffer + 32) - NTP_TO_UNIX;
    t3 = time_change(receiveBuffer + 40) - NTP_TO_UNIX;
    t1 -= WIN_TO_UNIX;
    t4 -= WIN_TO_UNIX;
    
    // printf("t1:%llx, t2:%llx, t3:%llx, t4:%llx\n", t1, t2, t3, t4);
    
    time_t offset = (t2 + t3 - t1 - t4) >> 1;
    time_t now;
    get_now(&now);
    now += offset;
    now -= WIN_TO_UNIX;
    // printf("now: %llx, offset: %llx\n", now, offset);

    NTP_Timestamp_Data now2 = NTP_Timestamp_Data(now);
    now2.output();

    return true;
}