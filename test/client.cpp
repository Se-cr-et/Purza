#include <iostream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int main(){
    int ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(8080);
    ServerAddress.sin_addr.s_addr = INADDR_ANY;
    connect(ClientSocket, (struct sockaddr*)& ServerAddress, sizeof(ServerAddress));

    while (true){
        int check = 0;
        char message[1024] = {0};
        cout << message << " " << "2" << endl;
        cin.get(message, 1024);
        cin.ignore();
        check = send(ClientSocket, message, strlen(message), 0);
    }

    close(ClientSocket);
    return 0;
}