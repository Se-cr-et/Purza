#include <vector>
#include <unordered_map>
#include <iostream>

class FLatVectorStore
{
private:
    int dimension;
    std::vector<float> vectors;
    std::vector<long long> ids; //stores the id for the vector at ith pos
    std::unordered_map<long long, long long> id_to_pos; //maps id to the pos of the vector
public:
    FLatVectorStore(int dim) : dimension(dim) {}
    bool insert(long long id, const std::vector<float>& vec)
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
};
