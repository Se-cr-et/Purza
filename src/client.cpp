#include <iostream>          // Basic Input/Output Library
#include <cstring>           // For Character-Strings
#include <netinet/in.h>      // Not complete sure yet
#include <sys/socket.h>      // Not completely sure yet
#include <unistd.h>          // Not completely sure yet
using namespace std;

int main(){
    // Creating a client socket
    int ClientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Initializing the Server Address and Type to connect to
    sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(8080);
    ServerAddress.sin_addr.s_addr = INADDR_ANY;

    // Connect to the Server Address using the client socket we created
    connect(ClientSocket, (struct sockaddr*)& ServerAddress, sizeof(ServerAddress));

    while (true){
        // Holds the message to be sent to server, max 1024 char size
        char snd_message[1024] = {0};
        char rcv_message[1024] = {0};

        cin.get(snd_message, 1024); cin.ignore();

        // Send the message to server
        send(ClientSocket, snd_message, strlen(snd_message), 0);

        // Recieve message from the server
        recv(ClientSocket, rcv_message, sizeof(rcv_message), 0);

        // If no message, exit loop
        if (!(strncmp(rcv_message, "\n", 1))){
            break;
        }
        else{
            cout << rcv_message << endl;
        }
    }

    cout << "Connection has been closed" << endl;

    // Close the Client Socket
    close(ClientSocket);
    return 0;
}