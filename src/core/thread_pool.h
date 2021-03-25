#pragma once
#include<functional>
#include <string>
#include <iostream>
// #include<intrin.h>
#include<list>
#include<atomic>
#include<mutex>
#include<chrono>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include "flowSQ.h"

namespace anyflow{
typedef std::function<void(void)> Task;
class flow_thread_pool{
public:
    class flow_cpuinfo{
    public:
        inline flow_cpuinfo():logic_core(0),hard_core(0){
            char vendor[12];
            unsigned regs[4];
            cpuID(0, regs);
            ((unsigned *)vendor)[0] = regs[1]; // EBX
            ((unsigned *)vendor)[1] = regs[3]; // EDX
            ((unsigned *)vendor)[2] = regs[2]; // ECX
            cpu_Vendor = std::string(vendor, 12);

            cpuID(1, regs);
            logic_core = (regs[1] >> 16) & 0xff; // EBX[23:16]

            if (cpu_Vendor == "GenuineIntel") {
              // Get DCP cache info
              cpuID(4, regs);
              hard_core = ((regs[0] >> 26) & 0x3f) + 1; // EAX[31:26] + 1
            } else if (cpu_Vendor == "AuthenticAMD") {
              // Get NC: Number of CPU cores - 1
              cpuID(0x80000008, regs);
              hard_core = ((unsigned)(regs[2] & 0xff)) + 1; // ECX[7:0] + 1
            }
        }
        int logic_core;
        int hard_core;
        std::string cpu_Vendor;
    private:
        inline void cpuID(unsigned i, unsigned regs[4]) {
            #ifdef _WIN32
                __cpuid((int *)regs, (int)i);

            #else
            asm volatile
                ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
                : "a" (i), "c" (0));
            // ECX is set to zero for CPUID function 4
            #endif
        }
    };
private:
    class std_impl_thread_pool {
    private:
        class ThreadWorker {
        private:
            int m_id;
            std_impl_thread_pool * m_pool;
        public:
            ThreadWorker(std_impl_thread_pool * pool, const int id)
            : m_pool(pool), m_id(id) {
            }

            void operator()() {
            std::function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown) {
                {
                std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                if (m_pool->m_queue.empty()) {
                    m_pool->m_conditional_lock.wait(lock);
                }
                dequeued = m_pool->m_queue.dequeue(func);
                }
                if (dequeued) {
                func();
                }
            }
            }
        };

    bool m_shutdown;
    flowSQ<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_conditional_mutex;
    std::condition_variable m_conditional_lock;
    public:
    std_impl_thread_pool(const int n_threads)
        : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {
        init();
    }
    ~std_impl_thread_pool(){this->shutdown();}

    std_impl_thread_pool(const std_impl_thread_pool &) = delete;
    std_impl_thread_pool(std_impl_thread_pool &&) = delete;

    std_impl_thread_pool & operator=(const std_impl_thread_pool &) = delete;
    std_impl_thread_pool & operator=(std_impl_thread_pool &&) = delete;

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
        m_threads[i] = std::thread(ThreadWorker(this, i));
        }
    }

    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();
        for (int i = 0; i < m_threads.size(); ++i) {
        if(m_threads[i].joinable()) {
            m_threads[i].join();
        }
        }
    }

    template<typename F, typename...Args>
    void schedule(F&& f, Args&&... args){
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)(); 
        };
        m_queue.enqueue(wrapper_func);
        m_conditional_lock.notify_one();
        task_ptr->get_future();
        return ;
    }
    };

    typedef std_impl_thread_pool impl_thread_pool;
    // typedef boost::threadpool::pool impl_thread_pool;

    inline flow_thread_pool(){
        if(!_pool){
            flow_cpuinfo cpuinfo;
            auto thread_num = cpuinfo.hard_core;
            m_cpuCount = thread_num;
            _pool = std::make_unique<impl_thread_pool>(thread_num);
        }
    }
    int m_cpuCount;
public:
    inline void schedule(Task task) {
        return _pool->schedule(task);
    }
    inline int getCpuCount(){
        return m_cpuCount;
    }
    inline virtual ~flow_thread_pool(){
    }
    inline static flow_thread_pool& GetInstance() {
        static flow_thread_pool __instance;
        return __instance;
    }
    inline void wait(int i = 0){
        return;
    }
private:
    std::unique_ptr<impl_thread_pool> _pool;
};

}