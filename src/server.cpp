#include <iostream>     // Basic Input/Output Library
#include <cstring>      // For Character-Strings
#include <netinet/in.h> // Not complete sure yet
#include <sys/socket.h> // Not completely sure yet
#include <unistd.h>     // Not completely sure yet
#include <thread>
#include <vector>
#include <sstream>
#include <chrono>
#include <mutex>
#include "FlatVectorStore.h"
#include "Parser.h"
using namespace std;

bool init_flag = true;
mutex mtx;

class Threader
{
    vector<thread> Threads;
    int num_of_threads;

public:
    Threader() : num_of_threads(0) {}
    void insert(int (*func)(int, Threader &, FLatVectorStore &), int value, Threader &T, FLatVectorStore &db)
    {
        Threads.push_back(thread(func, value, ref(T), ref(db)));
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

int communication(int ClientSocket, Threader &T, FLatVectorStore &db)
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
            mtx.lock();
            const int s = strlen(buffer) - strlen(command);
            char sub_buffer[1024] = {0};
            strncpy(sub_buffer, buffer + strlen(command) + 1, s);
            long long id;
            vector<float> vector;
            p.parse_add(sub_buffer, id, vector);
            // empty ADD
            if (vector.size() != dim || id == -1)
            {
                const char *msg = "Invalid Format\n";
                send(ClientSocket, msg, strlen(msg), 0);
                continue;
            }
            // [IMPLEMENT ADD FUNCTIONALITY HERE]
            // vector variable contains the vector, with the first index containing id
            db.insert(id, vector);
            const char *msg = "OK\n";
            send(ClientSocket, msg, strlen(msg), 0);
            mtx.unlock();
        }
        else if (!(strncmp(command, "SEARCH", 6)))
        {
            const int s = strlen(buffer) - strlen(command);
            char sub_buffer[1024] = {0};
            strncpy(sub_buffer, buffer + strlen(command) + 1, s);
            vector<float> v;
            string mode;
            int k;
            int nprobe;
            p.parse_search(sub_buffer, dim, v, mode, k, nprobe);
            if (v.size() != dim || !(mode == "BRUTE" || mode == "IVF") || k == -1)
            {
                const char *msg = "Invalid Format\n";
                send(ClientSocket, msg, strlen(msg), 0);
                continue;
            }
            if (mode == "BRUTE")
            {
                char *reply[1024];
                std::vector<std::vector<float>> k_vectors;
                std::vector<std::pair<long long, float>> ids_dis;
                db.k_nearest(k, v, k_vectors, ids_dis);
                stringstream ss;
                ss << '\n';
                for (int i = 0; i < k_vectors.size(); i++)
                {
                    ss << ids_dis[i].first << "  " << ids_dis[i].second << "\t";
                    for (int j = 0; j < k_vectors[i].size(); j++)
                    {
                        ss << k_vectors[i][j] << ' ';
                    }
                    ss << '\n';
                }
                ss << '(' << k << " results, mode=" << mode << ", scanned=" << db.get_db_size() << ")\n";
                ss << '\0';
                string str = ss.str();
                const char *msg = str.c_str();
                send(ClientSocket, msg, strlen(msg), 0);
            }
            if (mode == "IVF")
            {
                if (nprobe == -1)
                {
                    const char *msg = "Invalid Format\n";
                    send(ClientSocket, msg, strlen(msg), 0);
                    continue;
                }
                int scanned = 0;
                auto id_dis = db.IVF(nprobe, k, v, scanned);
                stringstream ss;
                for (int i = 0; i < id_dis[0].size(); i++)
                {
                    ss << '\n';
                    ss << id_dis[0][i] << "  " << id_dis[1][i] << "\t";
                    auto vec = db.get_vec_id(id_dis[0][i]);
                    for (int j = 0; j < vec.size(); j++)
                    {
                        ss << vec[j] << " ";
                    }
                }
                ss << '(' << id_dis[0].size() << " results, mode=" << mode << " nprobe=" << nprobe << ", scanned=" << scanned << ")\n";
                ss << '\0';
                string str = ss.str();
                const char *msg = str.c_str();
                send(ClientSocket, msg, strlen(msg), 0);
            }
        }
        else if (!(strncmp(command, "BUILD", 5)))
        {
            mtx.lock();
            auto start = std::chrono::high_resolution_clock::now();
            db.llyods_algorithm();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            stringstream ss;
            ss << "\tvectors:\t" << db.get_db_size() << '\n';
            ss << "\tclusters:\t" << db.get_no_clusters() << '\n';
            ss << "\titerations:\t" << db.get_iterations() << '\n';
            ss << "\tdone in " << elapsed.count() << " seconds.\n";
            string str = ss.str();
            const char *m = str.c_str();
            send(ClientSocket, m, strlen(m), 0);
            mtx.unlock();
        }
        else if (!(strncmp(command, "STATS", 5)))
        {
            stringstream ss;
            ss << "dimension:\t" << db.get_dim() << '\n';
            ss << "total vectors:\t" << db.get_db_size() << '\n';
            ss << "Index Built:\t" << db.get_ivf_built() ? "yes\n" : "no\n";
            ss << "clusters:\t" << db.get_no_clusters() << '\n';
            auto cs = db.get_cluster_sizes();
            for (int i = 0; i < cs.size(); i++)
            {
                ss << cs[i] << ", ";
            }
            ss << "\n";
            string str = ss.str();
            const char *m = str.c_str();
            send(ClientSocket, m, strlen(m), 0);
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
            const char *msg = "\n";
            send(ClientSocket, msg, strlen(msg), 0);
        }
        else
        {
            const char *msg = "No Attached Message\n";
            send(ClientSocket, msg, strlen(msg), 0);
            // For Errors
        }
        cout << "Client " << ClientSocket - 3 << ": " << buffer << endl;
    }

    T.decrement();
    close(ClientSocket);
    return -1;
}

void accept_cli(int Server, int max, Threader &Thread, FLatVectorStore &db)
{
    while (true)
    {
        int ClientSocket = accept(Server, nullptr, nullptr);
        cout << "Connected with " << ClientSocket - 3 << endl;

        // Has max connections been created ?
        if (!(Thread.size() == max))
        {
            // Create a thread for every new connection
            Thread.insert(communication, ClientSocket, Thread, ref(db));
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

    thread connections(accept_cli, ServerSocket, max, ref(Threads), ref(db));
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
