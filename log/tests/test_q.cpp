#include <stdio.h>
#include <thread>
#include <assert.h>
#include <stdlib.h>
#include <atomic>
#include "../pr.h"
#include "../log_queue.h"

using namespace std;

const int g_size = 10;
typedef int qType;
auto g_q = buffer_queue<qType>(g_size, true);
atomic<int> g_p_cnt = 0;
atomic<int> g_c_cnt = 0;

void produer(int p_num)
{
    long long tid = tid_to_ll(this_thread::get_id());
    PR_INFO("[tid:%lld] %d elements should be produced\n", tid, p_num);
    int i = 0;
    while(i<p_num)
    {
        bool ret = g_q.push(i);
        if(!ret)
        {
            PR_INFO("***[tid:%lld] push fail, buffer queue is full\n", tid);
        }
        else
        {
            PR_INFO("+++[tid:%lld] produced no.%d element\n", tid, i+1);
            g_p_cnt.fetch_add(1);
            i++;
        }
        
    }
}

void consumer(int c_num)
{
    long long tid = tid_to_ll(this_thread::get_id());
    PR_INFO("[tid:%lld] %d elements should be consumed\n", tid, c_num);
    int temp;
    for(int i=0; i<c_num; i++)
    {
        g_q.pop(temp);
        PR_INFO("---[tid:%lld] consumed no.%d element\n", tid, i+1);
        g_c_cnt.fetch_add(1);
    }
}

void fill_q(buffer_queue<qType>& q, bool test=false)
{
    for(int i=0; i<q.get_capacity(); i++)
        q.push(static_cast<qType>(i));  

    if(test)
    {   
        qType f_val;
        q.front(f_val);
        assert(f_val==0);

        for(int i=0; i<q.get_capacity(); i++) 
        {
            q.pop(f_val);
            // PR_INFO("poped val:%d\n", f_val);
            assert(f_val==i);
            if(i==q.get_capacity()-1)
                break;
            q.front(f_val);
            assert(f_val==i+1);
            // PR_INFO("front val:%d\n", f_val);
        }      
    }  
}

int main()
{
    pr_set_level(PR_LEVEL_DEBUG);

    assert(g_q.is_empty());
    assert(g_q.get_capacity()==g_size);
    assert(g_q.get_rcnt()==0);
    assert(g_q.get_wcnt()==0);

    fill_q(g_q);
    assert(g_q.is_full());
    assert(g_q.get_rcnt()==0);
    assert(g_q.get_wcnt()==g_size);

    g_q.clear();
    assert(g_q.is_empty());
    assert(g_q.get_rcnt()==0);
    assert(g_q.get_wcnt()==0);

    fill_q(g_q, true);
    assert(g_q.get_rcnt()==g_size);
    assert(g_q.get_wcnt()==g_size);
    assert(g_q.is_empty());
    fill_q(g_q);
    assert(g_q.get_rcnt()==g_size);
    assert(g_q.get_wcnt()==g_size*2);

    g_q.clear();

    int temp;
    auto start = chrono::steady_clock::now();
    bool ret= g_q.pop(temp, 2000);  
    auto end = chrono::steady_clock::now(); 
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    PR_INFO("cond wait_for duration: %d ms\n" ,static_cast<int>(duration.count()));
    assert(!ret);

    PR_INFO("buffer queue function test passed!\n\n\n");

    g_q.clear();
    fill_q(g_q);
    int origin_num = g_q.get_size();
    assert(origin_num==g_size);
    PR_INFO("------------------------------------------\n");
    PR_INFO("buffer queue capacity is %d\n", g_size);
    PR_INFO("origin element num is %d\n", origin_num);
    PR_INFO("------------------------------------------\n");
    int p_n1 = 100;
    int p_n2 = 50;
    int c_n1 = 160;
    thread p1(produer, p_n1);
    thread p2(produer, p_n2);
    thread c1(consumer, c_n1);
 
    p1.join();
    p2.join();
    c1.join();

    PR_INFO("\n");
    PR_INFO("origin element num is %d\n", origin_num);
    PR_INFO("totally produced %d elements\n", g_p_cnt.load());
    PR_INFO("totally consumed %d elements\n", g_c_cnt.load());

    assert( g_q.get_size() == ( p_n1 + p_n2 + g_size - c_n1 ) );
    assert( g_q.get_rcnt() == ( c_n1 ) );
    assert( g_q.get_wcnt() == ( g_size + p_n1 + p_n2 ) );
    PR_INFO("buffer queue mutithread test passed!\n");

    return 0;
}