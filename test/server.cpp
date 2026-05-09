#include <iostream>          // Basic Input/Output Library
#include <cstring>           // For Character-Strings
#include <netinet/in.h>      // Not complete sure yet
#include <sys/socket.h>      // Not completely sure yet
#include <unistd.h>          // Not completely sure yet
#include <thread>
#include <vector>
using namespace std;


int communication(int ClientSocket){
    while (true){
        // Holds the message recieved from the Client, max 1024 char size
        char buffer[1024] = {0};

        // Recieves message from client
        recv(ClientSocket, buffer, sizeof(buffer), 0);

        // If no message, exit loop
        if (strlen(buffer) == 0){
            break;
        }
        cout << "Message from Client " << ClientSocket << ": " << buffer << endl;
    }
    return -1;
}


int main(){

    // Creating a server socket
    int max = 5;
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
    listen(ServerSocket, max);

    int i = 0;
    vector<thread*> Threads;
    while (true){
        // Accept connections from the Queue [Listen function holds the Queue]
        int ClientSocket = accept(ServerSocket, nullptr, nullptr);
        cout << "Connected with: " << ClientSocket << endl;

        // Has max connections been created ?
        if (i <= max){
            // Create a thread for every new connection
            thread T(communication, ClientSocket);

            // Detach, so that the thread stops on its own
            T.detach();

            // Store the thread in the vector
            Threads.push_back(&T);
            i++;
        }
        else{
            cout << "Max Connections Reached" << endl;
        }

    }


    cout << "Connection has been closed" << endl;

    // Close the ServersSocket
    close(ServerSocket);
    return 0;
}
