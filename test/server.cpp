#include <iostream>          // Basic Input/Output Library
#include <cstring>           // For Character-Strings
#include <netinet/in.h>      // Not complete sure yet
#include <sys/socket.h>      // Not completely sure yet
#include <unistd.h>          // Not completely sure yet
using namespace std;


int main(){

    // Creating a server socket
    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Initializing the Server Address and Type
    sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(8080);
    ServerAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding the socket we created and the address we initialized
    bind(ServerSocket, (struct sockaddr*)& ServerAddress, sizeof(ServerAddress));
    cout << "Listening..." << endl;

    // Waits/Listens for connections
    listen(ServerSocket, 5);

    // Once a connection request arrives, accept it
    // and store it in ClientSocket
    int ClientSocket = accept(ServerSocket, nullptr, nullptr);
    cout << "Connected with: " << ClientSocket << endl;

    while (true){
        // Holds the message recieved from the Client, max 1024 char size
        char buffer[1024] = {0};

        // Recieves message from client
        recv(ClientSocket, buffer, sizeof(buffer), 0);

        // If no message, exit loop
        if (strlen(buffer) == 0){
            break;
        }
        cout << "Message from Client: " << buffer << endl;
    }

    cout << "Connection has been closed" << endl;

    // Close the ServersSocket
    close(ServerSocket);
    return 0;
}