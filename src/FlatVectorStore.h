#include <vector>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <algorithm>
#include <cmath>
using namespace std;

// The flat vector store class stores vectors of a fixed dimension Din a flat array
// This is done so for cache locality and efficient storage
// The vector of ids is kept to maintain the relative position of that vector in the array
// The hashmap is kept for fast access to get the vector of a particular id, as it maps id to relative pos
class FLatVectorStore
{
private:
    int dimension;
    std::vector<float> vectors;
    std::vector<long long> ids;                         // stores the id for the vector at ith pos
    std::unordered_map<long long, long long> id_to_pos; // maps id to the pos of the vector

    vector<float> centroids;                            // stores centroid coordinates
    vector<vector<int>> vectors_cluster_id;             // outer vector is clusters, inner vector stores vector index

    float distance_sq(const float *a, const float *b)
    {
        // distance function uses euclidean distance but doesnt take underroot as relative values stay the same
        float sum = 0;
        for (int i = 0; i < dimension; i++)
        {
            float diff = a[i] - b[i];
            sum += diff * diff;
        }
        return sum;
    }
    struct pair_cmp_lesser // pair first contains id, pair second contains distance
    {
        bool operator()(const std::pair<long long, float> &p1, std::pair<long long, float> &p2)
        {
            return p1.second < p2.second;
        }
    };

    struct cmp_dist{
        long long id;
        float dist;

        cmp_dist(long long i, float d) : id(i), dist(d){}

        bool operator<(const cmp_dist& Other) const{
            return dist < Other.dist;
        }
    };


public:
    FLatVectorStore(const int dim) : dimension(dim) {}
    bool insert(const long long id, const std::vector<float> &vec)
    {
        if (vec.size() != dimension)
            return false;
        auto it = id_to_pos.find(id);
        if (it != id_to_pos.end())
        {
            long long index = it->second * dimension;
            for (int i = 0; i < dimension; i++)
            {
                vectors[i + index] = vec[i];
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
    void print_store() // print all the vectors with their corresponding ids in the terminal
    {
        for (int i = 0; i < ids.size(); i++)
        {
            std::cout << ids[i] << ": ";
            for (int j = 0; j < dimension; j++)
            {
                std::cout << vectors[(i * dimension) + j] << " ";
            }
            std::cout << std::endl;
        }
    }
    std::vector<float> get_vec_id(const long long id) // returns a vector corresponding to the given id. if no vector exists returns origin
    {
        auto it = id_to_pos.find(id);
        if (it == id_to_pos.end())
            return std::vector<float>(dimension, 0);
        else
        {
            std::vector<float> v(dimension, 0);
            for (int i = 0; i < dimension; i++)
            {
                v[i] = vectors[(it->second * dimension) + i];
            }
            return v;
        }
    }
    int get_dim()
    {
        return dimension;
    }
    bool k_nearest(const int k, const std::vector<float> &target,
                   std::vector<std::vector<float>> &k_vectors, std::vector<std::pair<long long, float>> &ids_dis)
    {
        k_vectors.clear();
        ids_dis.clear();
        if (ids.size() < k || target.size() != dimension)
        {
            return false; // return false if k greater than total vectors or if target of invalid size
        }
        std::priority_queue<std::pair<long long, float>,
                            std::vector<std::pair<long long, float>>, pair_cmp_lesser>
            pq;
        const float *v = vectors.data();
        int i;
        // populate the pq with first k pairs
        const float *t = target.data();
        for (i = 0; i < k; i++)
        {
            float dist = distance_sq(v + (i * dimension), t);
            pq.push(std::pair<long long, float>(ids[i], dist));
        }
        for (; i < ids.size(); i++)
        {
            float dist = distance_sq(v + (i * dimension), t);
            if (dist < pq.top().second)
            {
                pq.pop();
                pq.push({ids[i], dist});
            }
        }
        // push the k nearest in ids_dis and sort it in ascending order
        while (!pq.empty())
        {
            std::pair<long long, float> temp = pq.top();
            pq.pop();
            ids_dis.push_back(temp);
        }
        std::sort(ids_dis.begin(), ids_dis.end(), pair_cmp_lesser());
        // push the vectors in k_vectors
        for (int j = 0; j < ids_dis.size(); j++)
        {
            k_vectors.push_back(std::vector<float>());
            long long pos = (id_to_pos.find(ids_dis[j].first))->second;
            for (int k = 0; k < dimension; k++)
            {
                k_vectors[j].push_back(vectors[(pos * dimension) + k]);
            }
        }
        return true;
    }
    vector<float> llyods_algorithm()
    {
        cout << "I was here 1" << endl;
        int number = ids.size();
        int k = sqrt(number);
        centroids = vector<float>(k*dimension);    // stores centroid coordinates
        vector<int> vector_cluster_id(number, -1); // stores the cluster index of vectors
        cout << "I was here 2" << endl;

        // picking k initial centroids
        for (int i = 0; i < k; i++)
        {
            int random_position = rand() % number;
            for (int j = 0; j < dimension; j++)
            {
                centroids[i * dimension + j] = vectors[random_position * dimension + j];
            }
        }
        cout << "I was here 3" << endl;
        const float *v = vectors.data();

        for (int iteration = 0; iteration < 50; iteration++)
        {
            bool update_happened = false;

            // Assignment step
            for (int i = 0; i < number; i++)
            {
                int min_distance = __INT_MAX__;
                int cluster_id = 0;

                for (int j = 0; j < k; j++)
                {
                    const float *c = centroids.data();
                    float distance = distance_sq(c + (j * dimension), v + (i * dimension));

                    if (distance < min_distance) // updating distance and cluster id
                    {
                        min_distance = distance;
                        cluster_id = j;
                    }
                }

                if (vector_cluster_id[i] != cluster_id) // updating cluster id of the vector i to the one it is closest to
                {
                    vector_cluster_id[i] = cluster_id;
                    update_happened = true;
                }
            }

            // convergence check
            if (!update_happened)
                break;

            // Recompute Centrouds
            vector<int> sum(k * dimension, 0);  // stores sum of centroid coordinates
            vector<int> count_of_vectors(k, 0); // stores number of vectors in ith cluster

            // computing sum of all the clusters
            for (int j = 0; j < number; j++)
            {
                int cluster = vector_cluster_id[j];
                count_of_vectors[cluster]++;

                for (int dim = 0; dim < dimension; dim++)
                {
                    sum[cluster * dimension + dim] += vectors[j * dimension + dim];
                }
            }

            // updating centroid coordinates by dividing the sum of cluster coordinates with the number of vectors in that cluster
            for (int j = 0; j < k; j++)
            {
                if (count_of_vectors[j] == 0)
                    continue;

                for (int dim = 0; dim < dimension; dim++)
                {
                    centroids[j * dimension + dim] = sum[j * dimension + dim] / count_of_vectors[j];
                }
            }
        }
        cout << "I was here 4" << endl;

        vectors_cluster_id = vector<vector<int>>(k);
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < number; j++)
            {
                if (i == vector_cluster_id[j])
                {
                    vectors_cluster_id[i].push_back(j); // storing integer index of a vector
                }
            }
        }
        cout << "I was here 5" << endl;
        return centroids;
    }


    vector<vector<float>> IVF(int nprobe, int k, vector<float> target){
        priority_queue<cmp_dist> Max_Centroid;
        int K = vectors_cluster_id.size(); // Number of Centroids

        float *c = centroids.data();
        for (int i = 0; i < K; i++){
            float *t = target.data();
            float distance = distance_sq(t, c);
            c += dimension;
            if (Max_Centroid.size() <= nprobe){
                Max_Centroid.push(cmp_dist(i,distance));
            }
            else{
                Max_Centroid.pop();
                Max_Centroid.push(cmp_dist(i,distance));
            }
        }
        Max_Centroid.pop();

        priority_queue<cmp_dist> Top_k;
        for (int i = 0; i < nprobe; i++){
            int CentroidID = Max_Centroid.top().id;
            int ClusterCount = vectors_cluster_id[CentroidID].size();
            float *v = vectors.data();
            float *t = target.data();
            Max_Centroid.pop();
            for (int j = 0; j < ClusterCount; j++){
                int VectorID = vectors_cluster_id[CentroidID][j];
                int offset = dimension*VectorID;
                float distance = distance_sq(t, v+offset);
                if (Top_k.size() <= k){
                    Top_k.push(cmp_dist(VectorID,distance));
                }
                else{
                    Top_k.pop();
                    Top_k.push(cmp_dist(VectorID,distance));
                }
            }
        }
        Top_k.pop();


        vector<vector<float>> nearest(2);

        for (int i = 0; i < k; i++){
            nearest[0].push_back(Top_k.top().id);
            nearest[1].push_back(Top_k.top().dist);
            Top_k.pop();
        }

        return nearest;
    }


    void Cluster(){
        for (int i = 0; i < vectors_cluster_id.size(); i++)
        {
            cout << "Cluster " << i << " coordinates : " << endl;
            for (int j = 0; j < dimension; j++)
            {
                cout << centroids[(i * dimension) + j] << " " << endl;
            }
            cout << endl;

            cout << "Cluster vectors ID: " << endl;
            for (int l = 0; l < vectors_cluster_id[i].size(); l++)
            {
                cout << vectors_cluster_id[i][l] << " ";
            }
            cout << endl;
        }
    }
};
