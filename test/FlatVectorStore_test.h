#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <cstring>
#include "../src/FlatVectorStore.h"
#include "../src/Parser.h"
using namespace std;
class FVS_test
{
private:
    void print_vector(const vector<float> &v)
    {
        for (int i = 0; i < v.size(); i++)
        {
            cout << v[i] << " ";
        }
        cout << endl;
    }
public:
    void test_insert()
    {
        cout << "---Testing Insert---" << endl;
        FLatVectorStore store(5);
        vector<float> v1 {1,1,1,1,1};
        vector<float> v2 {1,1,1,1,1};
        vector<float> v3 {1.5,2.5,3.5,4.5,5.5};
        vector<float> v4 {-1.1,-0.5,7.9,-10.0,0};
        cout << "Inserting: " << endl;
        print_vector(v1);
        print_vector(v2);
        print_vector(v3);
        print_vector(v4);
        store.insert(1, v1);
        store.insert(2, v2);
        store.insert(3, v3);
        store.insert(4, v4);
        cout << "Vector Store:" << endl;
        store.print_store();
        cout << "Overwriting id 4 with: " << endl;
        print_vector(v1);
        store.insert(4,v1);
        cout << "Vector Store:" << endl;
        store.print_store();
    }
    void test_brute_force()
    {
        cout << "---Testing Brute Force---" << endl;
        FLatVectorStore v(4);
        std::vector<float> v1{0.1, 0.2, 75, 5};
        std::vector<float> v2{0.15, 0.25, 0.35, 0.45};
        std::vector<float> v3{81, 6, 12, 0.2};
        std::vector<float> v4{0.85, 0.15, 15, 0.25};
        std::vector<float> v5{10, 0.5, 8, 0.5};

        v.insert(1, v1);
        v.insert(2, v2);
        v.insert(3, v3);
        v.insert(4, v4);
        v.insert(5, v5);

        v.print_store();

        std::cout << std::endl;

        std::vector<float> t{0.12, 0.22, 0.32, 0.42};
        std::vector<std::vector<float>> k_vectors;
        std::vector<std::pair<long long, float>> ids_dis;
        v.k_nearest(3, t, k_vectors, ids_dis);
        cout << "3 nearest to target: ";
        print_vector(t);
        for (int i = 0; i < ids_dis.size(); i++)
        {
            std::cout << ids_dis[i].first << ":" << ids_dis[i].second << ": ";
            print_vector(k_vectors[i]);
        }
    }
    void test_IVF()
    {
        cout << "---Testing IVF with 100 Random Vectors---" << endl;
        int dim = 10;
        FLatVectorStore store(dim);
        srand(42);
        cout << "Generating 100 random vectors..." << endl;
        for (int i = 0; i < 100; i++) {
            vector<float> v(dim);
            for (int d = 0; d < dim; d++) {
                v[d] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
            }
            store.insert(i, v);
        }

        cout << "Clustering vectors into groups..." << endl;
        store.llyods_algorithm();

        vector<float> target(dim);
        for (int d = 0; d < dim; d++) {
            target[d] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
        }
        int k = 5;
        int nprobe = 2;
        cout << "Searching for top " << k << " results using nprobe=" << nprobe << "..." << endl;
        int scanned = 0;
        vector<vector<float>> results = store.IVF(nprobe, k, target, scanned);
        cout << "IVF Search Results:" << endl;
        for (size_t i = 0; i < results[0].size(); i++) {
            cout << "Rank " << i + 1 << " | ID: " << (long long)results[0][i]
                 << " | Distance Sq: " << results[1][i] << endl;
        }
    }
    void test_IVF_custom()
    {
        int dim = 4;
        FLatVectorStore store(dim);
        vector<float> v1 {1,2,3,4};
        vector<float> v2 {2,3,4,5};
        vector<float> v3 {0.1, 0.4, 1.4, 3.5};

        store.insert(0, v1);
        store.insert(1, v2);
        store.insert(2, v3);
        store.llyods_algorithm();

        vector<float> target {0.1, 0.2, 0.3, 0.4};
        int k = 2;
        int nprobe = 2;
        cout << "Searching for top " << k << " results using nprobe=" << nprobe << "..." << endl;
        int scanned = 0;
        vector<vector<float>> results = store.IVF(nprobe, k, target, scanned);
        cout << "IVF Search Results:" << endl;
        for (size_t i = 0; i < results[0].size(); i++) {
            cout << "Rank " << i + 1 << " | ID: " << (long long)results[0][i]
                 << " | Distance Sq: " << results[1][i] << endl;
        }
    }
    void run_db_console() {
        const int dim = 4;
        FLatVectorStore db(dim);
        Parser p;
        string input_line;

        cout << "--- Vector DB Console Initialized (Dim: " << dim << ") ---" << endl;
        cout << "Commands: ADD, SEARCH, BUILD, STATS, SAVE, LOAD, QUIT" << endl;

        while (true) {
            cout << "\n> ";
            if (!getline(cin, input_line)) break;
            if (input_line.empty()) continue;

            stringstream ss(input_line);
            string command;
            ss >> command;

            // Normalize command to uppercase
            for (auto &c : command) c = toupper(c);

            if (command == "ADD") {
                string remaining;
                getline(ss, remaining);

                long long id = -1;
                vector<float> vec;

                if (!remaining.empty()) {
                    p.parse_add(remaining.data(), id, vec);
                }

                if (vec.size() != dim || id == -1) {
                    cout << "Error: Invalid Format or Dimension mismatch (Expected " << dim << ")." << endl;
                    continue;
                }
                db.insert(id, vec);
                cout << "OK" << endl;
            }
            else if (command == "SEARCH") {
                string remaining;
                getline(ss, remaining);

                vector<float> v;
                string mode;
                int k = -1;
                int nprobe = -1;

                if (!remaining.empty()) {
                    p.parse_search(remaining.data(), dim, v, mode, k, nprobe);
                }

                if (v.size() != dim) {
                    cout << "Error: Search vector dimension mismatch." << endl;
                    continue;
                }

                if (mode == "BRUTE") {
                    vector<vector<float>> k_vectors;
                    vector<pair<long long, float>> ids_dis;
                    db.k_nearest(k, v, k_vectors, ids_dis);

                    for (size_t i = 0; i < ids_dis.size(); i++) {
                        cout << ids_dis[i].first << "  [Dist: " << ids_dis[i].second << "]\t";
                        for (float val : k_vectors[i]) cout << val << " ";
                        cout << endl;
                    }
                    // Brute force scans the entire DB (number of vectors, not total float count)
                    cout << "(" << ids_dis.size() << " results, mode=" << mode << ", scanned=" << db.get_db_size() / dim << ")" << endl;
                }
                else if (mode == "IVF") {
                    if (nprobe == -1) {
                        cout << "Error: nprobe required for IVF." << endl;
                        continue;
                    }
                    if (!db.get_ivf_built()) {
                        cout << "Error: IVF index not built. Run BUILD first." << endl;
                        continue;
                    }

                    int scanned = 0;
                    auto id_dis = db.IVF(nprobe, k, v, scanned);

                    // Check if id_dis has the expected two rows (IDs and Distances) and is not empty
                    if (!id_dis.empty() && !id_dis[0].empty()) {
                        for (size_t i = 0; i < id_dis[0].size(); i++) {
                            long long found_id = (long long)id_dis[0][i];
                            cout << found_id << "  [Dist: " << id_dis[1][i] << "]\t";
                            auto vec = db.get_vec_id(found_id);
                            for (float val : vec) cout << val << " ";
                            cout << endl;
                        }
                        cout << "(" << id_dis[0].size() << " results, mode=" << mode << " nprobe=" << nprobe << ", scanned=" << scanned << ")" << endl;
                    } else {
                        cout << "(0 results, mode=" << mode << " nprobe=" << nprobe << ", scanned=" << scanned << ")" << endl;
                    }
                }
            }
            else if (command == "BUILD") {
                if (db.get_db_size() == 0) {
                    cout << "Error: Cannot build index on empty database." << endl;
                    continue;
                }

                auto start = chrono::high_resolution_clock::now();
                db.llyods_algorithm();
                auto end = chrono::high_resolution_clock::now();

                chrono::duration<double> elapsed = end - start;
                cout << "\tVectors:\t" << db.get_db_size() / dim << endl;
                cout << "\tClusters:\t" << db.get_no_clusters() << endl;
                cout << "\tIterations:\t" << db.get_iterations() << endl;
                cout << "\tDone in " << elapsed.count() << " seconds." << endl;
            }
            else if (command == "STATS") {
                cout << "Dimension:\t" << db.get_dim() << endl;
                cout << "Total vectors:\t" << db.get_db_size() / dim << endl;
                cout << "Index Built:\t" << (db.get_ivf_built() ? "Yes" : "No") << endl;
                cout << "Clusters:\t" << db.get_no_clusters() << endl;

                auto cs = db.get_cluster_sizes();
                cout << "Cluster Sizes: ";
                for (size_t i = 0; i < cs.size(); i++) {
                    cout << cs[i] << (i == cs.size() - 1 ? "" : ", ");
                }
                cout << endl;
            }
            else if (command == "SAVE" || command == "LOAD") {
                cout << "System: Implementation for " << command << " pending..." << endl;
            }
            else if (command == "QUIT" || command == "EXIT") {
                cout << "Exiting..." << endl;
                break;
            }
            else {
                cout << "Unknown command: " << command << endl;
            }
        }
    }
};