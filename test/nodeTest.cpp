#include<stdlib.h>
#include<functional>
#include<string.h>
#include<gtest/gtest.h>
#include"core/node.h"
#include<thread>
#include"spdlog_wrap.h"
#include<chrono>
#include<functional>
using namespace anyflow;
class context{
public:
    context(){a=0;}
    int a;
    int b;
    float c;
};

class cNode:public flow<context>::Node{
public:
    std::shared_ptr<context> NodeProcess(std::shared_ptr<context> input,void* ctx,std::shared_ptr<Node_Info> info){
        input->a++;
        input->a++;
        return input;
    }
};

void back(std::shared_ptr<context> i,std::shared_ptr<Node_Info> f){
    logi("terminal result:{}",i->a);
}

TEST(NodeTest,T1){
    setLogLevel(spdlog::level::debug);
    auto n1 = std::make_shared<cNode>();
    auto n2 = std::make_shared<cNode>();
    auto n3 = std::make_shared<cNode>();
    n1->setNext(n2);
    n2->setNext(n3);
    logd("");
    auto c1 = std::make_shared<context>();
    auto f1 = std::make_shared<Node_Info>();
    n1->setInput(c1,f1);
    logd("");
    c1 = std::make_shared<context>();
    n1->setInput(c1,f1);
    logd("");
    c1 = std::make_shared<context>();
    n1->setInput(c1,f1);
    logd("");
    c1 = std::make_shared<context>();
    n1->setInput(c1,f1);
    logd("");
    c1 = std::make_shared<context>();
    n1->setInput(c1,f1);
    logd("");
    c1 = std::make_shared<context>();
    n1->setInput(c1,f1);
    logd("");
    auto &g = flow_thread_pool::GetInstance();
    logd("");
    logi("g.join");
    n1->setInput(c1,f1);
    logi("g.join");
    g.wait();
    logd("");
    logi("g.join");
};