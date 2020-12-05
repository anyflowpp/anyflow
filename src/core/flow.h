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

    //TODO ȡ��getinstance, 
    //TODO: ʵ�� mgffactory ��mgfflow���й���,
    //Ŀ���Ǳ�֤ mgf_shutdown����ʱ������ȷ��,�ܲ����˳��ͱ���
    //�����˲�����
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
        //TODO: ͨ���������ɲ�ͬ��node��� m_nodes
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
    //TODO: ���漸����ͬ��ģʽʹ�õ�,ʵ�ֵ���ʱ�ȴ�,֪���ص���
    //����nodify, 
    flow_data_ptr GetOutput(); /// Ĭ�ϻ�ȡoutput�����,�´λ�ȡΪ��.
    flow_data_ptr GetOutput(bool clear); /// clearΪfalse����Զ�λ�ȡ
    //TODO:: �����private��publicȨ�޻���Ҫ�������
    std::list<RootNode_ptr> m_nodes;
private:
    //TODO :���ﱣ�����û����ս���Ļص�,������Ҫ���ûص�
    //���õ� mgfflow�ĺ�����, ��һЩ����,(����ͬ��ģʽ,��ͬ��ģʽ
    //ʵ�������ռ�, �ڵ��ÿͻ��Ļص�.
    callback_type  m_callback;
};
} // namespace anyflow