#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;


int main(){
    cout << "Cow" << endl;
    cout << "Buffalo" << endl;

    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(8080);
    ServerAddress.sin_addr.s_addr = INADDR_ANY;

    bind(ServerSocket, (struct sockaddr*)& ServerAddress, sizeof(ServerAddress));
    cout << "Listening..." << endl;
    listen(ServerSocket, 5);

    int ClientSocket = accept(ServerSocket, nullptr, nullptr);
    cout << "Connected with: " << ClientSocket << endl;

    while (true){
        char buffer[1024] = {0};
        cout << "At the top" << endl;
        recv(ClientSocket, buffer, sizeof(buffer), 0);
        if (sizeof(buffer) == 0){
            cout << "Unable to recieve messages" << endl;
        }
        cout << "Message from Client: " << buffer << endl;
    }

    close(ServerSocket);
    return 0;
}