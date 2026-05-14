#pragma once

#include "../src/FlatVectorStore.h"
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
        vector<vector<float>> results = store.IVF(nprobe, k, target);
        cout << "IVF Search Results:" << endl;
        for (size_t i = 0; i < results[0].size(); i++) {
            cout << "Rank " << i + 1 << " | ID: " << (long long)results[0][i]
                 << " | Distance Sq: " << results[1][i] << endl;
        }
    }
};