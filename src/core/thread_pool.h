#pragma once
#include<functional>

namespace anyflow {
typedef std::function<void(void)> Task;
class  anyflow_thread_pool{
public:
private:
    anyflow_thread_pool();
    int m_cpuCount;
public:
    bool schedule(Task task);
    int getCpuCount();
    ~anyflow_thread_pool();
    static anyflow_thread_pool& GetInstance();
    void wait(int i = 0);
};
}