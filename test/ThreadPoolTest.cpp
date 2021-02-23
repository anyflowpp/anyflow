#include<stdlib.h>
#include<functional>
#include<string.h>
#include<gtest/gtest.h>
#include"core/thread_pool.h"
#include<thread>
#include"spdlog_wrap.h"
#include<chrono>
#include<functional>

void task(){
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

void task1(int i){
    std::this_thread::sleep_for(std::chrono::seconds(i));
}

TEST(threadpool,t1){
    auto& tp = anyflow::flow_thread_pool::GetInstance();
    int i = 9;
    auto fn = std::bind(&task1,i);
    logi("begin thread");
    tp.schedule(task);
    tp.schedule(fn);
    tp.schedule(fn);
    tp.schedule(fn);
    tp.schedule(fn);
    tp.wait();
    logi("end");
};