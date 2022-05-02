#include "NTP_Client.h"

char url[20][20];
int n;
bool end;

// 返回选择的服务器编号
void display_menu(){
    printf("Please chooose one from following URLs as the NTP Server used in this experiment\n");
    printf("And enter the chosne URL's number in the console.\n");
    for(int i = 0; i <= n; i++)
        printf("%d. %s\n", i, url[i]);
}
bool choose_url(int *choice){
    scanf("%d", choice);
    // printf("%d\n", *choice == n);
    if(*choice == n){
        end = 1;
        return 1;
    }
    return (*choice >= 0 && *choice < n);
}
int main(){
    NTP_Client client;
    if(!client.init()){
        printf("Failed to init client!\nPlease check your Internet status or check your computer.\n");
        return 0;
    }
    // printf("%d\n", client.getState());

    freopen("NTP_Servers.txt", "r", stdin);
    while(scanf("%s", url[n++]) != EOF);
    freopen("CON", "r", stdin);
    n--;
    sprintf(url[n], "%s", "quit.");
    
    while(1){
        int choice = 0;
        do{
            display_menu();
        }while(!choose_url(&choice));

        if(end)
            break;

        if(!client.setNTPServerAddr(url[choice])){
            printf("Chosen URL cannot be resolved!\nPlease choose anothor URL or check your Internet status.\n\n");
            continue;
        }
        if(!client.sendNTPRequest()){
            printf("Failed to send NTP request!\nPlease choose anothor URL or check your Internet status.\n\n");
            continue;
        }
        if(!client.receiveNTPResponse()){
            printf("Failed to receive NTP response!\nPlease choose anothor URL or check your Internet status.\n\n");
            continue;
        }
    }
    return 0;
}