#include <iostream>     // Basic Input/Output Library
#include <cstring>      // For Character-Strings
#include <netinet/in.h> // Not complete sure yet
#include <sys/socket.h> // Not completely sure yet
#include <unistd.h>     // Not completely sure yet
#include <thread>
#include <vector>
#include <sstream>
#include "FlatVectorStore.h"
#include "Parser.h"
using namespace std;

bool init_flag = true;

class Threader
{
    vector<thread> Threads;
    int num_of_threads;

public:
    Threader() : num_of_threads(0) {}
    void insert(int (*func)(int, Threader &), int value, Threader &T)
    {
        Threads.push_back(thread(func, value, ref(T)));
        num_of_threads++;
    }
    int size()
    {
        return num_of_threads;
    }
    void decrement()
    {
        num_of_threads--;
    }
    ~Threader()
    {
        vector<thread *>::iterator it;
        for (int i = 0; i < num_of_threads; i++)
        {
            Threads.at(i).join();
        }
    }
};

int communication(int ClientSocket, Threader &T, FLatVectorStore& db)
{
    Parser p;
    int dim = db.get_dim();
    while (true)
    {
        // Holds the message recieved from the Client, max 1024 char size
        char buffer[1024] = {0};
        char command[7] = {0};

        // Recieves message from client
        recv(ClientSocket, buffer, sizeof(buffer), 0);

        // If no message, exit loop
        if (strlen(buffer) == 0)
        {
            break;
        }

        // Sends message to the client
        int cmd_index = strcspn(buffer, " ");
        strncpy(command, buffer, cmd_index);


        if (!(strncmp(command, "ADD", 3)))
        {
            const int size = strlen(buffer) - strlen(command);
            char sub_buffer[size+1] = {0};
            strncpy(sub_buffer, buffer + strlen(command) + 1, size);
            long long id; vector<float> vector;
            p.parse_add(sub_buffer, id, vector);
            //empty ADD
            if (vector.size() != dim || id == -1)
            {
                const char* msg = "Invalid Format\n";
                send(ClientSocket, msg, strlen(msg), 0);
                continue;
            }
            // [IMPLEMENT ADD FUNCTIONALITY HERE]
            // vector variable contains the vector, with the first index containing id
            db.insert(id, vector);
            const char* msg = "OK\n";
            send(ClientSocket, msg, strlen(msg), 0);
        }
        else if (!(strncmp(command, "SEARCH", 6)))
        {
            const int size = strlen(buffer) - strlen(command);
            char sub_buffer[size] = {0};
            strncpy(sub_buffer, buffer + strlen(command) + 1, size);
            vector<float> v; string mode; int k; int nprobe;
            p.parse_search(sub_buffer, dim, v, mode, k, nprobe);
            if (v.size() != dim || !(mode == "BRUTE" || mode == "IVF") || k == -1)
            {
                const char* msg = "Invalid Format\n";
                send(ClientSocket, msg, strlen(msg), 0);
                continue;
            }
            if (mode == "BRUTE")
            {
                char* reply[1024];
                std::vector<std::vector<float>> k_vectors;
                std::vector<std::pair<long long, float>> ids_dis;
                db.k_nearest(3, v, k_vectors, ids_dis);
                stringstream ss;
                ss << '\n';
                for (int i = 0; i < ids_dis.size(); i++)
                {
                    ss << ids_dis[i].first << ' ' << ids_dis[i].second << ' ';
                    for (int j = 0; j < v.size(); j++)
                    {
                        ss << v[j] << ' ';
                    }
                    ss << '\n';
                }
                ss << '(' << k << " results, mode=" << mode << ", scanned=" << db.get_db_size() << ")\n";
                ss << '\0';
                char* msg = ss.c_str();
                send(ClientSocket, msg, strlen(msg), 0);
            }
            if (mode == "IVF")
            {
                if (nprobe == -1)
                {
                    const char* msg = "Invalid Format\n";
                    send(ClientSocket, msg, strlen(msg), 0);
                    continue;
                }
            }
        }
        else if (!(strncmp(command, "BUILD", 5)))
        {
            const char* msg = "Building IVF Index\n";
            send(ClientSocket, msg, strlen(msg), 0);
            stringstream ss;
            ss << "vectors:\t" << db.get_db_size();

        }
        else if (!(strncmp(command, "STATS", 5)))
        {
            // [IMPLEMENT STATS FUNCTIONALITY HERE]
            cout << "IN STATS" << endl;
        }
        else if (!(strncmp(command, "SAVE", 4)))
        {
            // [IMPLEMENT SAVE FUNCTIONALITY HERE]
            cout << "IN SAVE" << endl;
        }
        else if (!(strncmp(command, "LOAD", 4)))
        {
            // [IMPLEMENT LOAD FUNCTIONALITY HERE]
            cout << "IN LOAD" << endl;
        }
        else if (!(strncmp(command, "QUIT", 4)))
        {
            // [IMPLEMENT QUIT FUNCTIONALITY HERE]
            cout << "IN QUIT" << endl;
        }
        else
        {
            // For Errors
        }

        if (!(cmd_index == strlen(buffer)))
        {
            send(ClientSocket, command, strlen(command), 0);
        }
        cout << "Client " << ClientSocket - 3 << ": " << buffer << endl;
    }

    T.decrement();
    close(ClientSocket);
    return -1;
}

void accept_cli(int Server, int max, Threader &Thread, FLatVectorStore& db)
{
    while (true)
    {
        int ClientSocket = accept(Server, nullptr, nullptr);
        cout << "Connected with: " << ClientSocket << endl;

        // Has max connections been created ?
        if (!(Thread.size() == max))
        {
            // Create a thread for every new connection
            Thread.insert(communication, ClientSocket, Thread);
            init_flag = false;
        }
        else
        {
            cout << "Max Connections Reached" << endl;
        }
    }
}

int main()
{
    const int dim = 4;
    FLatVectorStore db(dim);
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
    bind(ServerSocket, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
    cout << "Listening..." << endl;

    // Waits/Listens for connections
    listen(ServerSocket, max);

    thread connections(accept_cli, ServerSocket, max, ref(Threads));
    while (true)
    {
        // Accept connections from the Queue [Listen function holds the Queue]
        if (!(init_flag))
        {
            if (Threads.size() == 0)
            {
                break;
            }
        }
    }

    cout << "Connection has been closed" << endl;

    // Close the ServersSocket
    close(ServerSocket);
    return 0;
}
