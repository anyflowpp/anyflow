#pragma once
#include<map>
#include<memory>
#include<functional>
//node_info��Ҫ��ʵ�ֽڵ������Ϣ�ı���
//status  �������е�״̬
//err_msg ���������Ϣ keyΪnode����,valueΪ������Ϣ����
//cmd 

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