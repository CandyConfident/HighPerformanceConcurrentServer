#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>
#include <assert.h>

#define  THREADPOOL_MAX_NUM 64
//#define  THREADPOOL_AUTO_GROW

using namespace std;

class Threadpool
{
public:
	typedef function<void()> Task;

	inline Threadpool(unsigned short size = 4) { 
        assert(size <= THREADPOOL_MAX_NUM);
        add_thread(size); 
    }
	inline ~Threadpool()
	{
		tp_run=false;
		tp_task_cv.notify_all();
		for (thread& thread : tp_pool) {
			if(thread.joinable())
				thread.join(); 
		}
	}

	template<class F, class... Args>
	decltype(auto) post_task(F&& f, Args&&... args)
	{
		if (!tp_run)
			throw runtime_error("post_task on Threadpool has been stopped.");

		using return_type = typename std::result_of_t<F(Args...)>;
		auto task = make_shared<packaged_task<return_type()>>(
			bind(forward<F>(f), forward<Args>(args)...)
		);
		future<return_type> res = task->get_future();
		{
			lock_guard<mutex> lock{ tp_lock };
			tp_tasks.emplace([task](){
				(*task)();
			});
		}
#ifdef THREADPOOL_AUTO_GROW
		if (tp_idl_tnum < 1 && tp_pool.size() < THREADPOOL_MAX_NUM)
			add_thread(1);
#endif
		tp_task_cv.notify_one(); 

		return res;
	}


	int idl_thread_cnt() { return tp_idl_tnum; }

	int thread_cnt() { return tp_pool.size(); }

#ifndef THREADPOOL_AUTO_GROW
private:
#endif

	void add_thread(unsigned short size)
	{
		for (; tp_pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
		{
			tp_pool.emplace_back( [this]{
				while (tp_run)
				{
					Task task;
					{
						unique_lock<mutex> lock{ tp_lock };
						tp_task_cv.wait(lock, [this]{
								return !tp_run || !tp_tasks.empty();
						});
						if (!tp_run && tp_tasks.empty())
							return;
						task = move(tp_tasks.front());
						tp_tasks.pop();
					}
					tp_idl_tnum--;
					task();
					tp_idl_tnum++;
				}
			});
			tp_idl_tnum++;
		}
	}

private:
    Threadpool(const Threadpool &) = delete;

    Threadpool(Threadpool &&) = delete;

    Threadpool & operator=(const Threadpool &) = delete;

    Threadpool & operator=(Threadpool &&) = delete;

	vector<thread> tp_pool; 
	queue<Task> tp_tasks;
	mutex tp_lock;
	condition_variable tp_task_cv;
	atomic<bool> tp_run{ true };
	atomic<int>  tp_idl_tnum{ 0 };
};

#endif