#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <algorithm>
#include <cmath>
#include <limits.h>
#include <unordered_set>
using namespace std;

// The flat vector store class stores vectors of a fixed dimension Din a flat array
// This is done so for cache locality and efficient storage
// The vector of ids is kept to maintain the relative position of that vector in the array
// The hashmap is kept for fast access to get the vector of a particular id, as it maps id to relative pos
class FLatVectorStore
{
private:
    int dimension;
    int iterations;
    std::vector<float> vectors;
    std::vector<float> norm_vectors;
    std::vector<long long> ids;                         // stores the id for the vector at ith pos
    std::unordered_map<long long, long long> id_to_pos; // maps id to the pos of the vector
    bool is_cosine;

    vector<float> centroids;                      // stores centroid coordinates
    vector<vector<long long>> vectors_cluster_id; // outer vector is clusters, inner vector stores vector index
    friend bool save_snapshot(FLatVectorStore& store);
    friend bool load_snapshot(FLatVectorStore& store);

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
    float cosine_sim(const float *a, const float *b)
    {
        float dot = 0;
        for (int i = 0; i < dimension; i++)
        {
            dot += a[i] * b[i];
        }
        return 1 - dot;
    }
    struct pair_cmp_lesser // pair first contains id, pair second contains distance
    {
        bool operator()(const std::pair<long long, float> &p1, std::pair<long long, float> &p2)
        {
            return p1.second < p2.second;
        }
    };
    struct cmp_dist
    {
        long long id;
        float dist;

        cmp_dist(long long i, float d) : id(i), dist(d) {}

        bool operator<(const cmp_dist &Other) const
        {
            return dist < Other.dist;
        }
    };
    bool ivf_built;

public:
    FLatVectorStore(const int dim) : dimension(dim), ivf_built(false), is_cosine(false) {}
    int get_db_size()
    {
        return vectors.size() / dimension;
    }
    int get_no_clusters()
    {
        return vectors_cluster_id.size();
    }
    int get_iterations()
    {
        return iterations;
    }
    bool get_ivf_built()
    {
        return ivf_built;
    }
    int get_dim()
    {
        return dimension;
    }
    vector<int> get_cluster_sizes()
    {
        vector<int> v;
        for (int i = 0; i < vectors_cluster_id.size(); i++)
        {
            v.push_back(vectors_cluster_id[i].size());
        }
        return v;
    }
    bool insert(const long long id, const std::vector<float> &vec)
    {
        if (vec.size() != dimension)
            return false;
        auto it = id_to_pos.find(id);
        long long vector_pos;
        bool is_update = (it != id_to_pos.end());
        float sum = 0;
        if (is_update)
        {
            vector_pos = it->second;
            long long idx = vector_pos * dimension;
            for (int i = 0; i < dimension; i++)
            {
                vectors[i + idx] = vec[i];
                sum += vec[i] * vec[i];
            }
            sum = sqrt(sum);
            if (sum > 0)
            {
                for (int i = 0; i < dimension; i++)
                {
                    norm_vectors[i + idx] = vec[i] / sum;
                }
            }
            else
            {
                for (int i = 0; i < dimension; i++)
                {
                    norm_vectors[i + idx] = 0;
                }
            }
        }
        else
        {
            vector_pos = ids.size();
            id_to_pos[id] = vector_pos;
            ids.push_back(id);
            for (int i = 0; i < dimension; i++)
            {
                vectors.push_back(vec[i]);
                sum += vec[i] * vec[i];
            }
            sum = sqrt(sum);
            if (sum > 0)
            {
                for (int i = 0; i < dimension; i++)
                {
                    norm_vectors.push_back(vec[i] / sum);
                }
            }
            else
            {
                for (int i = 0; i < dimension; i++)
                {
                    norm_vectors.push_back(0);
                }
            }
        }
        if (!ivf_built)
            return true;
        float closest_dist = numeric_limits<float>::max();
        int closest_centroid = 0;
        const float *v;
        if (is_cosine)
            v = norm_vectors.data() + (vector_pos * dimension);
        else
            v = vectors.data() + (vector_pos * dimension);
        const float *c = centroids.data();
        for (int i = 0; i < vectors_cluster_id.size(); i++)
        {
            float dist = 0;
            if (is_cosine)
                dist = cosine_sim(v, c + (i * dimension));
            else
                dist = distance_sq(v, c + (i * dimension));
            if (dist < closest_dist)
            {
                closest_dist = dist;
                closest_centroid = i;
            }
        }
        vectors_cluster_id[closest_centroid].push_back(vector_pos);
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
    bool k_nearest(const int k, std::vector<float> target,
                   std::vector<std::vector<float>> &k_vectors, std::vector<std::pair<long long, float>> &ids_dis)
    {
        if (is_cosine)
        {
            float sum = 0;
            for (int i = 0; i < dimension; i++)
            {
                sum += target[i] * target[i];
            }
            if (sum > 0)
            {
                sum = sqrt(sum);
                for (int i = 0; i < dimension; i++)
                {
                    target[i] /= sum;
                }
            }
        }
        k_vectors.clear();
        ids_dis.clear();
        if (ids.size() < k || target.size() != dimension)
        {
            return false; // return false if k greater than total vectors or if target of invalid size
        }
        std::priority_queue<std::pair<long long, float>,
                            std::vector<std::pair<long long, float>>, pair_cmp_lesser>
            pq;
        const float *v;
        if (is_cosine)
            v = norm_vectors.data();
        else
            v = vectors.data();
        int i;
        // populate the pq with first k pairs
        const float *t = target.data();
        for (i = 0; i < k; i++)
        {
            float dist = 0;
            if (is_cosine)
                dist = cosine_sim(v + (i * dimension), t);
            else
                dist = distance_sq(v + (i * dimension), t);
            pq.push(std::pair<long long, float>(ids[i], dist));
        }
        for (; i < ids.size(); i++)
        {
            float dist = 0;
            if (is_cosine)
                dist = cosine_sim(v + (i * dimension), t);
            else
                dist = distance_sq(v + (i * dimension), t);
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
        long long number = ids.size();
        if (number == 0)
        {
            ivf_built = false; // Cannot build IVF with 0 vectors
            return vector<float>();
        }

        ivf_built = true;
        long long k = sqrt(number);
        if (k == 0)
            k = 1;
        centroids = vector<float>(k * dimension);  // stores centroid coordinates
        vector<int> vector_cluster_id(number, -1); // stores the cluster index of vectors

        // picking k initial centroids
        unordered_set<int> picked;
        while (picked.size() < k)
        {
            long long random_position = rand() % number;
            if (picked.insert(random_position).second)
            {
                int i = picked.size() - 1;
                for (int j = 0; j < dimension; j++)
                {
                    if (is_cosine)
                        centroids[i * dimension + j] = norm_vectors[random_position * dimension + j];
                    else
                        centroids[i * dimension + j] = vectors[random_position * dimension + j];
                }
            }
        }

        const float *v;
        if (is_cosine)
            v = norm_vectors.data();
        else
            v = vectors.data();
        int iteration;
        for (iteration = 0; iteration < 50; iteration++)
        {
            bool update_happened = false;

            // Assignment step
            for (long long i = 0; i < number; i++)
            {
                float min_distance = numeric_limits<float>::max();
                int cluster_id = 0;

                for (int j = 0; j < k; j++)
                {
                    const float *c = centroids.data();
                    float distance = 0;
                    if (is_cosine)
                        distance = cosine_sim(c + (j * dimension), v + (i * dimension));
                    else
                        distance = distance_sq(c + (j * dimension), v + (i * dimension));

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

            // Recompute Centroids
            vector<float> sum(k * dimension, 0); // stores sum of centroid coordinates
            vector<int> count_of_vectors(k, 0);  // stores number of vectors in ith cluster

            // computing sum of all the clusters
            for (long long j = 0; j < number; j++)
            {
                int cluster = vector_cluster_id[j];
                count_of_vectors[cluster]++;

                for (int dim = 0; dim < dimension; dim++)
                {
                    if (is_cosine)
                        sum[cluster * dimension + dim] += norm_vectors[j * dimension + dim];
                    else
                        sum[cluster * dimension + dim] += vectors[j * dimension + dim];
                }
            }

            // updating centroid coordinates by dividing the sum of cluster coordinates with the number of vectors in that cluster
            for (int j = 0; j < k; j++)
            {
                if (count_of_vectors[j] == 0)
                    continue;

                float norm_sum = 0;
                for (int dim = 0; dim < dimension; dim++)
                {
                    centroids[j * dimension + dim] = sum[j * dimension + dim] / (float)count_of_vectors[j];
                    if (is_cosine)
                        norm_sum += centroids[j * dimension + dim] * centroids[j * dimension + dim];
                }
                // normalize
                if (is_cosine)
                {
                    if (norm_sum > 0)
                    {
                        norm_sum = sqrt(norm_sum);
                        for (int dim = 0; dim < dimension; dim++)
                        {
                            centroids[j * dimension + dim] /= norm_sum;
                        }
                    }
                }
            }
        }
        iterations = iteration;
        vectors_cluster_id = vector<vector<long long>>(k);
        for (long long i = 0; i < number; i++)
        {
            vectors_cluster_id[vector_cluster_id[i]].push_back(i); // storing integer index of a vector
        }
        return centroids;
    }
    vector<vector<float>> IVF(int nprobe, int k, vector<float> target, int &scanned)
    {
        scanned = 0;
        if (is_cosine)
        {
            float sum = 0;
            for (int i = 0; i < dimension; i++)
            {
                sum += target[i] * target[i];
            }
            // NEW: Check if sum > 0
            if (sum > 0.0f)
            {
                sum = sqrt(sum);
                for (int i = 0; i < dimension; i++)
                {
                    target[i] /= sum;
                }
            }
        }
        priority_queue<cmp_dist> Max_Centroid;
        int K = vectors_cluster_id.size();
        if (K == 0)
            return vector<vector<float>>();

        for (int i = 0; i < K; i++)
        {
            float distance = 0;
            if (is_cosine)
                distance = cosine_sim(target.data(), centroids.data() + (i * dimension));
            else
                distance = distance_sq(target.data(), centroids.data() + (i * dimension));
            Max_Centroid.push(cmp_dist(i, distance));
            if (Max_Centroid.size() > nprobe)
            {
                Max_Centroid.pop();
            }
        }
        priority_queue<cmp_dist> Top_k;
        while (!Max_Centroid.empty())
        {
            int CentroidID = Max_Centroid.top().id;
            Max_Centroid.pop();
            vector<long long> &cluster = vectors_cluster_id[CentroidID];
            scanned += cluster.size();
            for (int i = 0; i < cluster.size(); i++)
            {
                float distance = 0;
                if (is_cosine)
                    distance = cosine_sim(target.data(), norm_vectors.data() + (cluster[i] * dimension));
                else
                    distance = distance_sq(target.data(), vectors.data() + (cluster[i] * dimension));
                Top_k.push(cmp_dist(cluster[i], distance));
                if (Top_k.size() > k)
                {
                    Top_k.pop();
                }
            }
        }
        int num = Top_k.size();
        if (num == 0)
            return vector<vector<float>>();
        vector<vector<float>> nearest(2, vector<float>(num));
        for (int i = num - 1; i >= 0; i--)
        {
            long long pos = Top_k.top().id;
            nearest[0][i] = ids[pos];
            nearest[1][i] = Top_k.top().dist;
            cout << "DIST:" << Top_k.top().dist << endl;
            Top_k.pop();
        }
        return nearest;
    }
    void Cluster()
    {
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
                cout << ids[vectors_cluster_id[i][l]] << " ";
            }
            cout << endl;
        }
    }
};
