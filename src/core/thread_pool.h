#pragma once
#include<functional>
#include<memory>
#include<boost/bind.hpp>
#include<boost/asio/thread_pool.hpp>
#include<boost/threadpool.hpp>
#include<thread>

namespace anyflow {
typedef boost::threadpool::pool thread_pool;
typedef std::function<void(void)> Task;
class anyflow_thread_pool{
public:
private:
    inline anyflow_thread_pool(){
        if(!_pool){
            auto thread_num = boost::asio::detail::default_thread_pool_size();
            m_cpuCount = thread_num;
            // _pool = std::make_unique<thread_pool>(thread_num);
            _pool = std::make_unique<thread_pool>(100);
        }
    }
    int m_cpuCount;
public:
    inline bool schedule(Task task) {
        return _pool->schedule(task);
    }
    inline int getCpuCount(){
        return m_cpuCount;
    }
    inline virtual ~anyflow_thread_pool(){}
    inline static anyflow_thread_pool& GetInstance() {
        static anyflow_thread_pool __instance;
        return __instance;
    }
    inline void wait(int i = 0){
        _pool->wait(i);
    }
private:
    std::unique_ptr<thread_pool> _pool;
};
}