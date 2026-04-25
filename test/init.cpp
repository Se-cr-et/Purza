#include <vector>
#include <unordered_map>
#include <iostream>

class FLatVectorStore
{
private:
    int dimension; //dimension is fixed for all the vectors
    std::vector<float> vectors; //all vectors are stored in 1d array for cache locality
    std::vector<long long> ids; //stores the id for the vector at ith pos
    std::unordered_map<long long, long long> id_to_pos; //maps id to the pos of the vector for fast retrieval while checking existence of a particular id and overwriting
public:
    FLatVectorStore(int dim) : dimension(dim) {}
    bool insert(long long id, const std::vector<float>& vec)
    {
        if (vec.size() != dimension)
            return false;
        auto it = id_to_pos.find(id);
        if (it != id_to_pos.end()) //overwrite existing vector
        {
            long long index = it->second * dimension; //to calculate the starting index of the vector at ith pos
            for (int i = 0; i < dimension; i++)
            {
                vectors[i+index] = vec[i];
            }
        }
        else //inserting new vector
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
};
