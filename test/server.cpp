#include <iostream>          // Basic Input/Output Library
#include <cstring>           // For Character-Strings
#include <netinet/in.h>      // Not complete sure yet
#include <sys/socket.h>      // Not completely sure yet
#include <unistd.h>          // Not completely sure yet
#include <thread>
#include <vector>
using namespace std;

bool init_flag = true;

class Threader {
    vector<thread> Threads;
    int num_of_threads;
public:
    Threader(): num_of_threads(0) {}
    void insert(int (*func)(int, Threader&), int value, Threader& T){
        Threads.push_back(thread(func, value, ref(T)));
        num_of_threads++;
    }
    int size(){
        return num_of_threads;
    }
    void decrement(){
        num_of_threads--;
    }
    ~Threader() {
        vector<thread*>::iterator it;
        for (int i = 0; i < num_of_threads; i++){
            Threads.at(i).join();
        }
    }
};

int communication(int ClientSocket,Threader& T){
    while (true){
        // Holds the message recieved from the Client, max 1024 char size
        char buffer[1024] = {0};
        char command[6] = {0};

        // Recieves message from client
        recv(ClientSocket, buffer, sizeof(buffer), 0);

        // If no message, exit loop
        if (strlen(buffer) == 0){
            break;
        }
        

        // Sends message to the client
        int index = strcspn(buffer," ");
        strncpy(command, buffer, index);
        if (!(index == strlen(buffer))){
            send(ClientSocket, command, strlen(command), 0);
        }
        cout << "Message from Client " << ClientSocket << ": " << buffer << endl;
    }

    T.decrement();
    close(ClientSocket);
    return -1;
}


void accept_cli(int Server, int max, Threader& Thread){
    while (true){
        int ClientSocket = accept(Server, nullptr, nullptr);
        cout << "Connected with: " << ClientSocket << endl;

        // Has max connections been created ?
        if (!(Thread.size() == max)){
            // Create a thread for every new connection
            Thread.insert(communication, ClientSocket, Thread);
            init_flag  = false;

            // Detach, so that the thread stops on its own

            // Store the thread in the vector
        }
        else{
            cout << "Max Connections Reached" << endl;
        }
    }
}

int main(){

    // Creating a server socket
    int max = 5;
    int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    Threader Threads;

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

    thread connections(accept_cli, ServerSocket, max, ref(Threads));
    while (true){
        // Accept connections from the Queue [Listen function holds the Queue]
        if (!(init_flag)){
            if (Threads.size() == 0){
                break;
            }
        }

    }


    cout << "Connection has been closed" << endl;

    // Close the ServersSocket
    close(ServerSocket);
    return 0;
}
