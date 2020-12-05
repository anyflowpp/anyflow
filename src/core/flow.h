#pragma once
#include<memory>
#include"json/json.h"
#include"node.h"
#include<list>
#include"NodeJsonParse.h"

#define FLOW_REGISTER_NODE(t,name,genfunc) static flow<t>::_NodeJsonParseInsertType s__insertTag##__LINE__(name,genfunc) 

namespace anyflow {
extern std::map<std::string,void*> nodetype_funcs;
template<class T>
class flow{
public:
    typedef std::function<void (std::shared_ptr<T>)> callback_type;
    typedef std::shared_ptr<flow> flow_ptr;
    typedef Node<T> RootNode;
    typedef std::shared_ptr<Node<T>> RootNode_ptr;
    typedef std::shared_ptr<T> flow_data_ptr;

    typedef std::function<RootNode_ptr(const Json::Value&)> genfunc; 
    typedef RootNode_ptr(*GenFuncType_C)(const Json::Value&);
    class _NodeJsonParseInsertType{
    public:
        typedef std::shared_ptr<Node<T>> RootNode_ptr;
    public:
        _NodeJsonParseInsertType(std::string type,void* f){
            NodeJsonParse::InsertNode(type,f);
        }
    };
    class NodeJsonParse{
        typedef std::shared_ptr<Node<T>> RootNode_ptr;
    public:
        typedef std::function<RootNode_ptr(const Json::Value&)> genfunc; 
        static void InsertNode(std::string type,void* nodegenner) {
            nodetype_funcs.insert({type,nodegenner});
        }
    };

    //TODO 取消getinstance, 
    //TODO: 实现 mgffactory 对mgfflow进行管理,
    //目的是保证 mgf_shutdown调用时机的正确性,总不能退出就崩溃
    //或者退不出来
    static flow_ptr GetInstance(const Json::Value& flowcfg){
        flow_ptr ret = std::make_shared<flow>(flowcfg);
        return ret;
    }
    virtual ~flow(){
        // auto &g = anyflow_thread_pool::GetInstance();
        // g.wait();
        std::unique_lock<std::mutex> locker(m_input_count_mutex);
        while(m_input_count>0){
            m_input_count_cond.wait(locker);
        }
        m_nodes.clear();
    }
    flow(const Json::Value& flowcfg):m_input_count(0){
        //TODO: 通过配置生成不同的node填充 m_nodes
        m_nodes.clear();
        Json::Value flow = flowcfg["flow"];
        for(auto i = flow.begin();i!=flow.end();++i){
            auto jnode = *i;
            std::string nodetype = jnode["type"].asString();
            std::shared_ptr<RootNode> node;

            auto genf = nodetype_funcs.find(nodetype);
            if(genf!=nodetype_funcs.end()){
                auto func = (GenFuncType_C)(genf->second);
                node = func(jnode);
            }

            if(m_nodes.size()!=0){
                auto last = m_nodes.back();
                last->setNext(node);
            }
            m_nodes.push_back(node);
        }
        auto node = m_nodes.back();
        node->setCallBack(std::bind(&flow::lastNodeCallBack,this,std::placeholders::_1));
    }
    void SetInput(flow_data_ptr input){
        {
            std::unique_lock<std::mutex> locker(m_input_count_mutex);
            m_input_count++;
        }
        auto& first = m_nodes.front();
        first->setInput(input);
    }
    void SetCallBack(callback_type func){
        m_callback = func;
    }
    void lastNodeCallBack(flow_data_ptr input){
        {
            std::unique_lock<std::mutex> locker(m_input_count_mutex);
            m_input_count--;
            if(m_input_count<=0){
                m_input_count_cond.notify_all();
            }
            if(this->m_callback){
                m_callback(input);
            }
        }
    }
    int m_input_count;
    std::mutex m_input_count_mutex;
    std::condition_variable m_input_count_cond;
    //TODO: 下面几个是同步模式使用的,实现调用时等待,知道回调里
    //进行nodify, 
    flow_data_ptr GetOutput(); /// 默认获取output后清空,下次获取为空.
    flow_data_ptr GetOutput(bool clear); /// clear为false则可以多次获取
    //TODO:: 这里的private和public权限还需要重新设计
    std::list<RootNode_ptr> m_nodes;
private:
    //TODO :这里保存了用户最终结果的回调,我们需要先让回调
    //调用到 mgfflow的函数里, 做一些操作,(比如同步模式,给同步模式
    //实现留出空间, 在调用客户的回调.
    callback_type  m_callback;
};
} // namespace anyflow