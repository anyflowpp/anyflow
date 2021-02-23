#pragma once
namespace anyflow{
class flowcfg{
public:
    typedef enum{
        sync,
        async,
        microsync,
    } run_mode;
    inline flowcfg(){}
    inline flowcfg(run_mode mode,run_mode cb_mode):m_mode(mode),m_cb_mode(cb_mode){}
    inline virtual ~flowcfg(){}
    run_mode m_mode;
    run_mode m_cb_mode;
};
}//anyflow