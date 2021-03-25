#pragma once
#include<map>
#include<memory>
#include<functional>

namespace anyflow{
class Node_Info{
public:
    typedef enum{
        OK,   //������һ���ڵ������
        BREAK //�����¸��ڵ�Ĵ������,callback���ش���ڵ�
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
    shared = 0, //�̹߳���
    holdon = 1  //�̶߳�ռ
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