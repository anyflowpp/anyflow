#pragma once
#include<map>
#include<memory>
#include<functional>

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
	virtual std::shared_ptr<void> NodeExec(std::shared_ptr<void> input, void *ctx, node_info_ptr info)=0;
    virtual bool Init(const Json::Value&)=0;
	virtual void* CreateThreadContext() =0;
	virtual void* GetThreadContext()=0; 
    virtual void DestroyThreadContext(void* ctx)=0;
};
typedef std::shared_ptr<node_exec> node_exec_ptr;

}