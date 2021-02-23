#pragma once
#include<memory>
#include"json/json.h"
#include<list>
#include"flowcfg.h"
#include"node_info.h"
#include"thread_pool.h"
#include<vector>
#include<map>

#define FLOW_REGISTER_NODE(t,name,genfunc) static flow<t>::_NodeJsonParseInsertType s__insertTag##__LINE__(name,genfunc); 

namespace anyflow{
template<class T>
class flow{
public:
    class Node;
    typedef std::shared_ptr<T> flow_data_ptr;
    typedef std::shared_ptr<Node> RootNode_ptr;
    typedef std::function<void (flow_data_ptr, node_info_ptr)> callback_type;
    typedef std::shared_ptr<flow> flow_ptr;
    typedef std::weak_ptr<flow> flow_w_ptr;
    typedef std::vector<flow_data_ptr> vec_flow_data_ptr;
    typedef std::function<RootNode_ptr(const Json::Value&)> genfunc; 

    typedef RootNode_ptr(*GenFuncType_C)(const Json::Value&);
    typedef RootNode_ptr(GenNodeOutWay_C)(const Json::Value& name);
    typedef std::function<GenNodeOutWay_C> GenNodeOutWay_STD;
    class Node {
    public:
        flow_w_ptr m_ownedToFlow;
        class input_struct
        {
        public:
            flow_data_ptr input;
            node_info_ptr info;
        };
        typedef enum
        {
            shared = 0, //线程共享
            holdon = 1  //线程独占
        } thread_mode;
    
    
        Node() : m_input_count(0), m_max_thread_number(1), m_bRelease_thread(false)
        {
            m_cb_func = nullptr;
        }
    
        Node(callback_type func) : m_cb_func(func), m_input_count(0), m_max_thread_number(1), m_bRelease_thread(false)
        {
        }
    
        virtual ~Node()
        {
            ReleaseThread();
        }
    
        void setOwnedFlow(flow_ptr o_flow){
            this->m_ownedToFlow = o_flow;
    	}
    
        virtual void genInput(flow_data_ptr input, node_info_ptr info){
            if(!m_ownedToFlow.expired()){
                if(this->next_node){
                    auto  f = this->m_ownedToFlow.lock();
                    f->IncraseInputCount();
                    this->next_node->setInput(input,info);
                }
            }
        }
    
        virtual void setInput(flow_data_ptr input, node_info_ptr info)
        {
            if (m_run_mode == thread_mode::shared){
                {
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    while (m_input_count > m_max_thread_number)
                    {
                        m_cond.wait(lock);
                    }
                    m_input_count++;
                }
                auto &g = flow_thread_pool::GetInstance();
                g.schedule(std::bind((void (Node::*)(flow_data_ptr input, node_info_ptr info)) & Node::thread_loop, this, input, info));
            }
            else{
                //判断线程是否存在,不存在则创建
                if (m_node_Threads.size()==0)
                {
                    auto &g = flow_thread_pool::GetInstance();
                    for (int i = 0; i < this->m_max_thread_number; i++)
                    {
                        auto f = std::bind((void (Node::*)()) & Node::thread_loop, this);
                        m_node_Threads.push_back(std::make_shared<std::thread>(f));
                    }
                }
                std::unique_lock<std::mutex> locker(this->m_mutex);
                while (this->m_input_buf.size() > m_max_input_buf_number){
                     this->m_cond.wait(locker);
                }
                input_struct st_input;
                st_input.input = input;
                st_input.info = info;
                this->m_input_buf.push_back(st_input);
                this->m_cond.notify_one();
            }
        }
        void setCallBack(callback_type func)
        {
            this->m_cb_func = func;
        }
        void setNext(RootNode_ptr next)
        {
            this->next_node = next;
        }
        void SetThreadRunMode(thread_mode mode)
        {
            m_run_mode = mode;
        }
        void SetNodeType(std::string type)
        {
            m_strNode_Type = type;
        }
        std::string GetNodeType()
        {
            return m_strNode_Type;
        }
        //设置缓存区长度
        void SetInputBufNum(int length)
        {
            m_max_input_buf_number = length;
        }
        //设置线程的最大长度
        void SetThreadNum(int num)
        {
            m_max_thread_number = num;
        }
    
    protected:
        friend  flow;
        void thread_loop(flow_data_ptr input, node_info_ptr info)
        {
            if (info->status == Node_Info::NodeStatus::BREAK)
            {
                if (m_cb_func)
                {
                    m_cb_func(input, info);
                }
                if (next_node)
                    this->next_node->setInput(input, info);
            }
            else
            {
                void *ctx = nullptr;//GetThreadContext();
                auto output = this->NodeProcess(input, ctx, info);
                if (m_cb_func)
                {
                    m_cb_func(output, info);
                }
                if (next_node)
                    this->next_node->setInput(output, info);
            }
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_input_count--;
                    m_cond.notify_one();
                }
        };
        void thread_loop()
        {
            void *ctx = nullptr;
            {
                std::lock_guard<std::mutex> lck (m_mutex_thread_ctx);
                ctx = CreateThreadContext();
            }
            while (!m_bRelease_thread)
            {
                {
                    std::unique_lock<std::mutex> locker(this->m_mutex);
                    while (this->m_input_buf.size() == 0 && !m_bRelease_thread){
                        // this->m_cond.wait_for(locker,std::chrono::milliseconds(100));
                        this->m_cond.wait(locker);
                    }
                    if (m_bRelease_thread){
                        break;
                    }
                }
                while (true)
                {
                    input_struct input_st;
                    {
                        std::unique_lock<std::mutex> locker(this->m_mutex);
                        if (this->m_input_buf.size() > 0){
                            input_st = this->m_input_buf.front();
                        }
                        else{
                            break;
                        }
                        m_input_buf.pop_front();
                        m_cond.notify_one();
                    }
                    if (input_st.info->status == Node_Info::NodeStatus::BREAK)
                    {
                        if (m_cb_func)
                        {
                            m_cb_func(input_st.input, input_st.info);
                        }
                        if (next_node)
                            this->next_node->setInput(input_st.input, input_st.info);
                    }
                    else
                    {
                        auto output = this->NodeProcess(input_st.input, ctx, input_st.info);
                        if (m_cb_func)
                        {
                            m_cb_func(output, input_st.info);
                        }
                        if (next_node)
                            this->next_node->setInput(output, input_st.info);
                    }
                }
            }
            {
                std::lock_guard<std::mutex> lck (m_mutex_thread_ctx);
                DestroyThreadContext(ctx);
            }
            {
                std::unique_lock<std::mutex> locker(this->m_wait_mutex);
                this->m_max_thread_number--;
                m_wait_thread.notify_one();
            }
        }
        void ReleaseThread()
        {
            if (m_run_mode == thread_mode::holdon)
            {
                m_bRelease_thread = true;
                {
                    std::unique_lock<std::mutex> locker(this->m_mutex);
                    this->m_cond.notify_all();
                }
                std::unique_lock<std::mutex> locker(this->m_wait_mutex);
                while (m_max_thread_number)
                {
                    m_wait_thread.wait(locker);
                }
                for(int i = 0;i<m_node_Threads.size();i++){
                    if(m_node_Threads[i]->joinable()){
                        m_node_Threads[i]->join();
                    }
                }
            }
        }
    
        virtual flow_data_ptr NodeProcess(flow_data_ptr input, void *ctx, node_info_ptr info) { throw std::logic_error("you need overwrite this function"); return nullptr; } //ctx为传入参数,ctx为getThreadContext函数返回的上下文
        virtual flow_data_ptr NodeProcessBack(flow_data_ptr input, node_info_ptr info) { return input; } //ctx为传入参数,ctx为getThreadContext函数返回的上下文

        virtual void* CreateThreadContext() {return nullptr;}                                                                               //重写函数 建立对应的上下文关系
        virtual void DestroyThreadContext(void*) {}                                                                              //重写函数 销毁对应的上下文关系
        virtual void *GetThreadContext() { return nullptr; }                                                                //获取线程对应的上下文环境 重写此函实现代码的,主要针对线程独占的设计
        virtual void *GetThreadContext(std::thread::id) { return nullptr; }                                                                //获取线程对应的上下文环境 重写此函实现代码的,主要针对线程独占的设计
    
        std::string m_strNode_Type;
        int m_input_count;
        RootNode_ptr next_node;
        std::mutex m_mutex;
        std::condition_variable m_cond;
        callback_type m_cb_func;
        //针对多线程独占线程进行的修改
        std::vector<std::shared_ptr<std::thread>> m_node_Threads;
        bool m_bRelease_thread;
        std::list<input_struct> m_input_buf; //待处理消息队列
        thread_mode m_run_mode;
        int m_max_input_buf_number;
        int m_max_thread_number;
        std::mutex m_wait_mutex;
        std::condition_variable m_wait_thread;
        std::map<std::thread::id,void*>   m_thread_ctx;
        std::mutex                        m_mutex_thread_ctx;
    };
    class _NodeJsonParseInsertType{
    public:
        _NodeJsonParseInsertType(std::string type,void* f){
            NodeJsonParse::InsertNode(type,f);
        }
    };
    class NodeJsonParse{
    public:
        static void InsertNode(std::string type,void* nodegenner) {
            nodetype_funcs.insert({type,nodegenner});
        }
        static std::map<std::string,void*> nodetype_funcs;
    };

    static flow_ptr GetInstance(const Json::Value& flowcfg,GenNodeOutWay_STD OutGenner){
        flow_ptr ret = std::make_shared<flow>(flowcfg,OutGenner);
        for(auto it = ret->m_nodes.begin();it!=ret->m_nodes.end();++it){
            (*it)->setOwnedFlow(ret);
        }
        return ret;
    }

    static flow_ptr GetInstance(const Json::Value& flowcfg){
        flow_ptr ret = std::make_shared<flow>(flowcfg);
        for(auto it = ret->m_nodes.begin();it!=ret->m_nodes.end();++it){
            (*it)->setOwnedFlow(ret);
        }
        return ret;
    }
    virtual ~flow(){
        std::unique_lock<std::mutex> locker(m_input_count_mutex);
        while(m_input_count>0){
            m_input_count_cond.wait(locker);
            // m_input_count_cond.wait_for(locker,std::chrono::milliseconds(100));
        }
        m_nodes.clear();
    }
    flow(const Json::Value& flowcfg,GenNodeOutWay_STD OutGenner):m_input_count(0){
        //TODO: 处理找不到节点的情况
        m_nodes.clear();
        Json::Value flow = flowcfg["flow"];
        for(auto i = flow.begin();i!=flow.end();++i){
            auto jnode = *i;
            std::string nodetype = jnode["type"].asString();
            std::shared_ptr<Node> node;
            auto func = OutGenner;
            node = func(jnode);
            if(!node){
                continue;
            }
            if(m_nodes.size()!=0){
                auto last = m_nodes.back();
                last->setNext(node);
            }
            m_nodes.push_back(node);
        }
        if(m_nodes.size()>0){
            auto node = m_nodes.back();
            node->setCallBack(std::bind(&flow::lastNodeCallBack,this,std::placeholders::_1,std::placeholders::_2));
        }
    }
    flow(const Json::Value& flowcfg):m_input_count(0){
        //TODO: 处理找不到节点的情况
        m_nodes.clear();
        Json::Value flow = flowcfg["flow"];
        for(auto i = flow.begin();i!=flow.end();++i){
            auto jnode = *i;
            std::string nodetype = jnode["type"].asString();
            std::shared_ptr<Node> node;

            auto genf = flow::NodeJsonParse::nodetype_funcs.find(nodetype);
            if(genf!=flow::NodeJsonParse::nodetype_funcs.end()){
                auto func = (GenFuncType_C)(genf->second);
                node = func(jnode);
            }

            if(m_nodes.size()!=0){
                auto last = m_nodes.back();
                last->setNext(node);
            }
            m_nodes.push_back(node);
        }
        if(m_nodes.size()>0){
            auto node = m_nodes.back();
            node->setCallBack(std::bind(&flow::lastNodeCallBack,this,std::placeholders::_1,std::placeholders::_2));
        }
    }

    void SetInput(flow_data_ptr input,node_info_ptr info){
        {
            std::unique_lock<std::mutex> locker(m_input_count_mutex);
            m_input_count += 1;
        }
        auto& first = m_nodes.front();
        first->setInput(input,info);
    }
    void SetFlowBack(flow_data_ptr input, node_info_ptr info) {
        auto it = m_nodes.end();
        --it;
        while(it!=m_nodes.begin()){
            input = (*it)->NodeProcessBack(input,info);
            --it;
        }
        input = (*it)->NodeProcessBack(input,info);
    }

    void IncraseInputCount(){
        std::unique_lock<std::mutex> locker(m_input_count_mutex);
        m_input_count+=1;
    }

    void SetCallBack(callback_type func){
        if(m_nodes.size()>0){
            auto node = m_nodes.back();
            node->setCallBack(std::bind(&flow::lastNodeCallBack,this,std::placeholders::_1,std::placeholders::_2));
        }
        m_callback = func;
    }
    void lastNodeCallBack(flow_data_ptr input, node_info_ptr info){
        {
            std::unique_lock<std::mutex> locker(m_input_count_mutex);
            m_input_count--;
            if(m_flowcfg.m_mode==flowcfg::microsync || m_flowcfg.m_mode==flowcfg::sync){
                m_output.push_back(input);
            }
            if(m_input_count<=0){
                m_input_count_cond.notify_all();
            }
            if(m_flowcfg.m_cb_mode==flowcfg::async){
                locker.unlock();
            }
            if(this->m_callback){
                m_callback(input,info);
            }
            this->SetFlowBack(input,info);
        }
    }
    int m_input_count;
    std::mutex m_input_count_mutex;
    std::condition_variable m_input_count_cond;
    //TODO: 下面几个是同步模式使用的,实现调用时等待,知道回调里
    //进行nodify, 
    vec_flow_data_ptr GetOutput(){
        vec_flow_data_ptr ret;
        std::unique_lock<std::mutex> locker(m_input_count_mutex);
        if(m_flowcfg.m_mode==flowcfg::microsync){
            while(m_input_count>0){
                m_input_count_cond.wait(locker);
            }
        }
        ret = m_output;
        m_output.clear();
        return ret;
    }
    //TODO:: 这里的private和public权限还需要重新设计
    std::list<RootNode_ptr> m_nodes;
    flowcfg m_flowcfg;
    void SetCfg(flowcfg cfg){
        m_flowcfg = cfg;
        if(m_flowcfg.m_mode == flowcfg::sync){
            for(auto i = m_nodes.begin();i!=m_nodes.end();i++){
                (*i)->SetThreadNum(1);
            }
        }
    }
    vec_flow_data_ptr m_output;
private:
    //TODO 开放的public变量太多, 适当做做访问性封装
    callback_type  m_callback;
};

template<class T>
std::map<std::string,void*> flow<T>::NodeJsonParse::nodetype_funcs;
}//anyflow