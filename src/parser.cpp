#include <iostream>
#include <vector>
using namespace std;

void parser()
{
    string word, mode;
    vector<int> vec;
    int num, k, id;

    cin >> word;

    if (word == "SEARCH")
    {
        cin >> id;

        while (cin >> num)
        {
            vec.push_back(num);
        }
    }
    else if (word == "ADD")
    {
        while (cin >> num)
        {
            vec.push_back(num);
        }

        cin >> k;
        cin >> mode;
    }
    else if (word == "STATS")
    {
        // stats logic
    }
    else if (word == "QUIT")
    {
        // disconnect the client
    }
    else
    {
        cout << "Invalid command" << endl;
    }
}