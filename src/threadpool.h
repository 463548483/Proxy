#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;



class Threadpool
{
private:
    vector<std::thread> threads;
    queue<function<void()> > m_tasks;
    int max_threads;
    int max_tasks;
    mutex m_lock;
    condition_variable has_task;
    bool run_flag;
    static Threadpool * pool;
    static mutex * mtx;
public:
    Threadpool();
    bool init_pool(function<void()> t);
    static Threadpool * get_pool();
    bool assign_task(function<void()> task);
    ~Threadpool();
private:
    bool init_threads();
    void run_task();//take the first task in queue and excute



};

#endif