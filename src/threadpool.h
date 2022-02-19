#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;
int MAX_THREADS=10;
int MAX_TASKS=100;

typedef function<void()> Task;

class Threadpool
{
private:
    vector<thread> threads;
    queue<Task> m_tasks;
    int max_threads;
    int max_tasks;
    mutex m_lock;
    condition_variable has_task;
    bool run_flag;
    static Threadpool * pool;
    static mutex * mtx;
public:
    Threadpool();
    bool init_pool(Task t);
    static Threadpool * get_pool();
    bool assign_task(Task task);
    ~Threadpool();
private:
    bool init_threads();
    void run_task();//take the first task in queue and excute



};

#endif