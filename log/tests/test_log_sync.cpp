#include <chrono>
#include <vector>
#include <atomic>

#include "../log.h"
#include "../pr.h"

using namespace std;

const int g_item_num = 11000;
const int g_t_num = 5;
atomic<int> g_t_cnt = 0;

void log_sync_test()
{
    long long tid = tid_to_ll(this_thread::get_id());
    PR_INFO("[threa_%d]tid is: %lld\n", g_t_cnt.fetch_add(1), tid);
    for(int i=0; i<g_item_num; i++)
    {
        LOG_INFO("[tid:%lld] log sync test: %d\n", tid, i);
    }
}

int main()
{

    Logger::get_instance()->init("./log_sync.txt");

    vector<thread> threads;
    
    PR_INFO("sync log test started\n");
    PR_INFO("start to log\n");
    auto start = chrono::steady_clock::now();

    for(int i=0; i<g_t_num; i++)
    {
        threads.emplace_back(log_sync_test);       
    }
    for(int i=0; i<g_t_num; i++)
    {
        threads[i].join();   
    }

    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    PR_INFO("end logging\n"); 
    PR_INFO("totally write %d items into files\n", g_t_num * g_item_num);
    PR_INFO("costed time: %d ms\n" ,static_cast<int>(duration.count()));
    return 0;
}