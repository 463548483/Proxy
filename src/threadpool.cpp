#include "threadpool.h"
#include <iostream>
#include <memory>
#define _CRTDBG_MAP_ALLOC


using namespace std;
int MAX_THREADS=10;
int MAX_TASKS=100;

Threadpool * Threadpool::pool=NULL;
mutex * Threadpool::mtx=new mutex();

Threadpool::Threadpool():max_threads(MAX_THREADS),max_tasks(MAX_TASKS),run_flag(true){
    if (threads.size()==0){
        unique_lock<mutex> lck(m_lock);
        if (threads.size()==0){
            init_threads();
        }
    }
}

Threadpool * Threadpool::get_pool(){
    if(pool==NULL){
        unique_lock<mutex> lck(*mtx);
        if (pool==NULL){
            pool=new Threadpool();
        }
    }
    return pool;
}



void Threadpool::run_task(){
    while(run_flag==true||!m_tasks.empty()){
        function<void()> task;
        unique_lock<mutex> lck(m_lock);
        while(m_tasks.empty()){
            has_task.wait(lck);
        }
        task=m_tasks.front();
        m_tasks.pop();
        task();
    }
}

bool Threadpool::init_threads(){
    for (int i=0;i<max_threads;i++){
        threads.push_back(thread(&Threadpool::run_task,this));
    }
    return true;
}

bool Threadpool::assign_task(function<void()> t){
    {
        unique_lock<mutex> lck(m_lock);
        if (m_tasks.size()<(unsigned int)max_tasks){
            m_tasks.push(t);
        }
        else{
            return false;
        }
    }
    has_task.notify_one();
    return true;
}

Threadpool::~Threadpool(){
    {
        unique_lock<mutex> ul(m_lock);
        run_flag=false;
    }
    has_task.notify_all();
    for (auto &t:threads){
        t.join();
    }
    threads.clear();
}
