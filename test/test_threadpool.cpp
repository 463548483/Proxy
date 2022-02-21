#include <iostream>
#include "../src/threadpool.h"
#include <memory>
#define _CRTDBG_MAP_ALLOC


using namespace std;
int ticket = 0;

void showTicket(mutex* m){
            lock_guard<std::mutex> l(*m);
            //cout<< thread.get_id()<<endl;
            cout <<" show ticket: " << ticket++ << endl;
    
}

int main(){
    mutex *m = new mutex;
    int sum = 0;
    {
    Threadpool thread_pool;
    Threadpool* pool = thread_pool.get_pool();
    for(int i = 0; i < 200; i++){
        if(!pool->assign_task(bind(showTicket, m))){
            sum++;
        }
    }
    }
    cout << "not use task : "<< sum << endl;
    delete m;
    system("pause");
    return 0;
}
