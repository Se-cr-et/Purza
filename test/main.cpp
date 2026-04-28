#include "init.h"

void print_vector(const std::vector<float>& v)
{
    for (int i = 0; i < v.size(); i++)
    {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}


int main()
{
    FLatVectorStore v(4);
    std::vector<float> v1 {0.1, 0.2, 0.3, 0.4};
    std::vector<float> v2 {0.15, 0.25, 0.35, 0.45};
    std::vector<float> v3 {0.9, 0.1, 0.1, 0.2};
    std::vector<float> v4 {0.85, 0.15, 0.05, 0.25};
    std::vector<float> v5 {0.5, 0.5, 0.5, 0.5};


    v.insert(1, v1);
    v.insert(2, v2);
    v.insert(3, v3);
    v.insert(4, v4);
    v.insert(5, v5);

    v.print_store();

    std::cout << std::endl;

    std::vector<float> t {0.12, 0.22, 0.32, 0.42};
    std::vector<std::vector<float>> k_vectors;
    std::vector<std::pair<long long, float>> ids_dis;
    v.k_nearest(3, t, k_vectors, ids_dis);

    for (int i = 0; i < ids_dis.size(); i++)
    {
        std::cout << ids_dis[i].first << ":" << ids_dis[i].second << ": ";
        print_vector(k_vectors[i]);
    }

    return 0;
}