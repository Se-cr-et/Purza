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
    std::vector<float> v1(4, 1);
    std::vector<float> v2(4, 2);
    std::vector<float> v3(4, 3);
    std::vector<float> v4(4, 4);

    v.insert(1, v1);
    v.insert(2, v2);
    v.insert(3, v3);
    v.insert(4, v4);

    v.print_store();

    std::cout << std::endl;

    print_vector(v.get_vec_id(1));
    print_vector(v.get_vec_id(2));
    print_vector(v.get_vec_id(3));
    print_vector(v.get_vec_id(4));
    print_vector(v.get_vec_id(5));

    return 0;
}