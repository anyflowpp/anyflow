#pragma once
#include<memory>
#include<vector>
#include<mutex>
#include<condition_variable>
#include"thread_pool.h"
#include<list>
#include<functional>

namespace anyflow {
template<class T>
class Node{
public:
    typedef std::function<void (std::shared_ptr<T>)> callback_type;
    void setNext(std::shared_ptr<Node> next){
        this->next_node = next;
    }
    virtual ~Node(){}
    virtual void releaseThread(){}
    Node():m_input_max(2),m_input_count(0){
    }
    Node(callback_type func):m_cb_func(func),m_input_count(0){
    }
    void setCallBack(callback_type func){
        this->m_cb_func=func;
    }
    virtual void setInput(std::shared_ptr<T> input){
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_input_count>= m_input_max){
            m_cond.wait(lock);
        }
        m_input_count++;
        auto& g = anyflow_thread_pool::GetInstance();
        g.schedule(std::bind(&Node::thread_loop,this,input));
    }
    int m_input_max;
protected:
    void thread_loop(std::shared_ptr<T> input){
        auto output = this->NodeProcess(input);
        if(m_cb_func){
            m_cb_func(output);
        }
        if(next_node)
            this->next_node->setInput(output);
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_input_count--;
            m_cond.notify_one();
        }
    };
    virtual std::shared_ptr<T> NodeProcess(std::shared_ptr<T> input)= 0;
    std::shared_ptr<T> m_output;
    int m_input_count;
    std::shared_ptr<Node> next_node;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    callback_type m_cb_func;
};
} // namespace anyflow