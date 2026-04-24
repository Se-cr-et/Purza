#include <iostream>
#include <thread>
#include <mutex>
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

int main()
{
    thread t1(inc);
    thread t2(inc);

    t1.join();
    t2.join();

    cout << "Number after execution: " << number << endl;
}