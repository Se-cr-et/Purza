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
};
