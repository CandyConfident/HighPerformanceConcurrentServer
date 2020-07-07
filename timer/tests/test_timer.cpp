#include <atomic>

#include "timer.h"
#include "log.h"
#include "pr.h"

using namespace std;

void test_repeated_func(chrono::time_point<chrono::steady_clock> t1, int num)
{
    static int cnt = num;
    long long tid = tid_to_ll(this_thread::get_id());

    auto t2 = chrono::steady_clock::now();
    int tm_diff = static_cast<int>( chrono::duration<double, milli>(t2-t1).count() );
    LOG_INFO("[tid:%lld] hello, repeated_func! total repeated_cnt = %d, left cnt: %d, time diff is %d ms\n",
                                                                     tid, num, --cnt, tm_diff);    
}

void test_run_at_func(chrono::time_point<chrono::steady_clock> t1, int ms)
{
    long long tid = tid_to_ll(this_thread::get_id());

    auto t2 = chrono::steady_clock::now();
    int tm_diff = static_cast<int>( chrono::duration<double, milli>(t2-t1).count() );
    LOG_INFO("[tid:%lld] hello, run_at_func ! run at %d ms, time diff is %d ms\n", tid, ms, tm_diff);   
}

class period_cls
{
public:
    void operator()(chrono::time_point<chrono::steady_clock> t1, int a)
    {
        long long tid = tid_to_ll(this_thread::get_id());

        auto t2 = chrono::steady_clock::now();
        int tm_diff = static_cast<int>( chrono::duration<double, milli>(t2-t1).count() );
        LOG_INFO("[tid:%lld] hello, period_cls_func with param %d! period_cls_run_cnt = %d, time diff is %d ms\n", 
                                        tid, a, period_cls_cnt.load(), tm_diff);
        period_cls_cnt.fetch_add(1);
        return;
    }
    period_cls() = default;
    period_cls(const period_cls&){}
    period_cls(period_cls&&){}
private:
    atomic<int> period_cls_cnt = 0;
};

void test_timer() {

    long long tid = tid_to_ll(this_thread::get_id());

    Timer t;
    t.run();

    /* test repeated */
    auto t1 = std::chrono::steady_clock::now();
    int repeated_cnt = 5;
    int repeated_timeout_ms = 800;
    LOG_INFO("[tid:%lld] start to test repeated run, repeated cnt is %d, timeout is %d ms\n", tid, repeated_cnt, repeated_timeout_ms);
    auto repeated_id = t.run_repeated(repeated_timeout_ms, repeated_cnt, test_repeated_func, t1, repeated_cnt);

    /* test run at certain time */
    int run_at_ms = 1250;
    t1 = std::chrono::steady_clock::now();
    auto now = chrono::high_resolution_clock::now();
    LOG_INFO("[tid:%lld] start to test run at certain time, run at %d ms after now\n", tid, run_at_ms); 
    auto certion_id = t.run_at(now + std::chrono::milliseconds(run_at_ms), test_run_at_func, t1, run_at_ms);

    /* test run once after certain time */
    int run_after_ms = 2150;
    t1 = chrono::steady_clock::now();
    LOG_INFO("[tid:%lld] start to test run once after certain time, run after %d ms\n", tid, run_after_ms);
    auto run_after_lamda = [t1, run_after_ms]{
                                            long long tid = tid_to_ll(this_thread::get_id());
                                            auto t2 = chrono::steady_clock::now();
                                            int tm_diff = static_cast<int>( chrono::duration<double, milli>(t2-t1).count() );
                                            LOG_INFO("[tid:%lld] hello, run_after_once_lambda_func! time diff is %d ms\n", 
                                                                                                tid, tm_diff);
                                        };
    auto once_id = t.run_after(run_after_ms, false, run_after_lamda);

    /* test run after certain time periodically */
    int run_after_ms_period = 500;
    period_cls p_cls;
    int p_param = 99;
    t1 = chrono::steady_clock::now();
    LOG_INFO("[tid:%lld] start to run after certain time periodically, period is %d ms\n", tid, run_after_ms_period);
    auto period_id = t.run_after(run_after_ms_period, true, [t1, &p_cls](int val){ p_cls(t1, val); }, p_param);
    // auto period_id = t.run_after(run_after_ms_period, true, bind(period_cls(), placeholders::_1), p_param);
    

    this_thread::sleep_for(chrono::seconds(5));
    /* test cancel */
    t.cancel(period_id);
    LOG_INFO("[tid:%lld] cancel periodically running\n", tid);
}

int main()
{
    Logger::get_instance()->init(NULL);
    test_timer();
    return 0;
}