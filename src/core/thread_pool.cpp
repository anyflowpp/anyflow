#include"thread_pool.h"
#include"thread_pool.h"
#include<memory>
#include<boost/bind.hpp>
#include<boost/asio/thread_pool.hpp>
#include<boost/threadpool.hpp>
#include<thread>


using namespace anyflow;

typedef boost::threadpool::pool thread_pool;

static std::unique_ptr<thread_pool> _pool;

anyflow_thread_pool::anyflow_thread_pool(){
    if(!_pool){
        auto thread_num = boost::asio::detail::default_thread_pool_size();
        m_cpuCount = thread_num;
        // _pool = std::make_unique<thread_pool>(thread_num);
        _pool = std::make_unique<thread_pool>(100);
    }
}

bool anyflow_thread_pool::schedule(Task task) {
    return _pool->schedule(task);
}

anyflow_thread_pool::~anyflow_thread_pool(){
    _pool.reset();
}

anyflow_thread_pool& anyflow_thread_pool::GetInstance() {
    static anyflow_thread_pool __instance;
    return __instance;
}

void anyflow_thread_pool::wait(int i){
    _pool->wait(i);
}

int anyflow_thread_pool::getCpuCount(){
    return m_cpuCount;
}