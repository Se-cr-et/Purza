#include <vector>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <algorithm>

//The flat vector store class stores vectors of a fixed dimension Din a flat array
//This is done so for cache locality and efficient storage
//The vector of ids is kept to maintain the relative position of that vector in the array
//The hashmap is kept for fast access to get the vector of a particular id, as it maps id to relative pos
class FLatVectorStore
{
private:
    int dimension;
    std::vector<float> vectors;
    std::vector<long long> ids; //stores the id for the vector at ith pos
    std::unordered_map<long long, long long> id_to_pos; //maps id to the pos of the vector
    float distance_sq ( const float* a , const float* b) {
        //distance function uses euclidean distance but doesnt take underroot as relative values stay the same
        float sum = 0;
        for ( int i = 0; i < dimension ; i++) {
            float diff = a[i] - b[i];
            sum += diff * diff ;
        }
        return sum ;
    }
    struct pair_cmp_lesser //pair first contains id, pair second contains distance
    {
        bool operator()(const std::pair<long long, float>& p1, std::pair<long long, float>& p2)
        {
            return p1.second < p2.second;
        }
    };

public:
    FLatVectorStore(const int dim) : dimension(dim) {}
    bool insert(const long long id, const std::vector<float>& vec)
    {
        if (vec.size() != dimension)
            return false;
        auto it = id_to_pos.find(id);
        if (it != id_to_pos.end())
        {
            long long index = it->second * dimension;
            for (int i = 0; i < dimension; i++)
            {
                vectors[i+index] = vec[i];
            }
        }
        else
        {
            long long pos = ids.size();
            id_to_pos[id] = pos;
            ids.push_back(id);
            for (int i = 0; i < dimension; i++)
            {
                vectors.push_back(vec[i]);
            }
        }
        return true;
    }
    void print_store() //print all the vectors with their corresponding ids in the terminal
    {
        for (int i = 0; i < ids.size(); i++)
        {
            std::cout << ids[i] << ": ";
            for (int j = 0; j < dimension; j++)
            {
                std::cout << vectors[(i*dimension)+j] << " ";
            }
            std::cout << std::endl;
        }
    }
    std::vector<float> get_vec_id(const long long id) //returns a vector corresponding to the given id. if no vector exists returns origin
    {
        auto it = id_to_pos.find(id);
        if (it == id_to_pos.end())
            return std::vector<float> (dimension, 0);
        else
        {
            std::vector<float> v(dimension, 0);
            for (int i = 0; i < dimension; i++)
            {
                v[i] = vectors[(it->second*dimension)+i];
            }
            return v;
        }

    }
    int get_dim()
    {
        return dimension;
    }
    bool k_nearest(const int k, const std::vector<float>& target,
        std::vector<std::vector<float>>& k_vectors, std::vector<std::pair<long long, float>>& ids_dis)
    {
        k_vectors.clear();
        ids_dis.clear();
        if (ids.size() < k || target.size() != dimension)
        {
            return false; //return false if k greater than total vectors or if target of invalid size
        }
        std::priority_queue<std::pair<long long, float>,
        std::vector<std::pair<long long, float>>, pair_cmp_lesser> pq;
        const float* v = vectors.data();
        int i;
        //populate the pq with first k pairs
        const float* t = target.data();
        for (i = 0; i < k; i++)
        {
            float dist = distance_sq(v+(i*dimension), t);
            pq.push(std::pair<long long, float>(ids[i], dist));
        }
        for (; i < ids.size(); i++)
        {
            float dist = distance_sq(v+(i*dimension), t);
            if (dist < pq.top().second) {
                pq.pop();
                pq.push({ids[i], dist});
            }
        }
        //push the k nearest in ids_dis and sort it in ascending order
        while (!pq.empty())
        {
            std::pair<long long, float> temp = pq.top();
            pq.pop();
            ids_dis.push_back(temp);
        }
        std::sort(ids_dis.begin(), ids_dis.end(), pair_cmp_lesser());
        //push the vectors in k_vectors
        for (int j = 0; j < ids_dis.size(); j++)
        {
            k_vectors.push_back(std::vector<float>());
            long long pos = (id_to_pos.find(ids_dis[j].first))->second;
            for (int k = 0; k < dimension; k++)
            {
                k_vectors[j].push_back(vectors[(pos*dimension)+k]);
            }
        }
        return true;
    }
};
