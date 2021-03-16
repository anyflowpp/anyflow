#pragma once
#include<map>
#include<memory>
#include<functional>
//node_info主要是实现节点错误信息的保存
//status  保存运行的状态
//err_msg 保存错误信息 key为node类型,value为错误信息描述
//cmd 

namespace anyflow{
class Node_Info{
public:
    typedef enum{
        OK,   //继续下一个节点的运行
        BREAK //跳过下个节点的处理程序,callback返回错误节点
    }NodeStatus;
    Node_Info():status(NodeStatus::OK){
    }
	~Node_Info() {
	}
    NodeStatus status;
    std::map<std::string, std::string> err_msg;    
    std::map<std::string, std::string> cmd;   
};
typedef std::shared_ptr<Node_Info>  node_info_ptr;

typedef enum
{
    shared = 0, //线程共享
    holdon = 1  //线程独占
} node_thread_mode;

class node_exec{
public:
    inline node_exec(){}
    inline virtual ~node_exec(){}
	inline virtual std::shared_ptr<void> NodeExec(std::shared_ptr<void> input, void *ctx, node_info_ptr info) { return input; }
	inline virtual void* CreateThreadContext() { return nullptr; }
	inline virtual void* GetThreadContext() { return nullptr; }
    inline virtual void DestroyThreadContext(void* ctx){}
private:
    void* m_ctx;
};
typedef std::shared_ptr<node_exec> node_exec_ptr;

} //namespace anyflow