#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
using namespace std;

mutex mtx;
int number = 0;

// remove the thread locking and unlocking part to see the racing condition faced when mutex is not used.
void inc()
{
    mtx.lock(); // lock the thread
    for (int i = 0; i < 1000000; i++)
        number++;
    mtx.unlock(); // release the lock
}

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
int main()
{
    thread t1(inc);
    thread t2(inc);

    t1.join();
    t2.join();

    cout << "Number after execution: " << number << endl;
}