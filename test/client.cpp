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
        char message[1024] = {0};
        cout << "Your message: ";
        cin.get(message, 1024); cin.ignore();

        // If no message, exit loop
        if (strlen(message) == 0){
            break;
        }

        // Send the message to server
        send(ClientSocket, message, strlen(message), 0);
    }

    cout << "Connection has been closed" << endl;

    // Close the Client Socket
    close(ClientSocket);
    return 0;
}