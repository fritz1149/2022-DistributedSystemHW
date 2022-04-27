#include "NTP_Client.h"

int main(){
    NTP_Client client;
    client.init();
    // printf("%d\n", client.getState());

    freopen("NTP_Servers.txt", "r", stdin);
    char url[20];
    scanf("%s", url);
    client.setNTPServerAddr(url);
    client.sendNTPRequest();
    client.receiveNTPResponse();
    return 0;
}