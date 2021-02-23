#pragma once
#include <memory>
#include <cstdio>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#else
#include<dlfcn.h>
#endif
#ifdef WIN32
inline HMODULE GetCurrentModule(BOOL bRef/* = FALSE*/) {
	HMODULE hModule = NULL;
	if (GetModuleHandleEx(bRef ? GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS : (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
		| GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT), (LPCSTR)GetCurrentModule, &hModule))
	{
		return hModule;
	}
	return NULL;
}
inline std::string GetCurrentModulePath() {
	HMODULE dllHmod = GetCurrentModule(false);
	CHAR szBuffer[MAX_PATH] = { 0 };
	std::string g_dllPath;
	if (dllHmod != NULL)
	{
		GetModuleFileNameA(dllHmod, szBuffer, sizeof(szBuffer) / sizeof(TCHAR) - 1);
		g_dllPath = szBuffer;
	}
	const std::string DIR = g_dllPath.substr(0, g_dllPath.find_last_of('\\'));
	return DIR;
}
#else
inline int GetModuleFileName( void* Fun, char* & sFilePath)   {
	int ret = -1;
	char* g_prePath = nullptr;
	if((g_prePath = getcwd(NULL, 0)) == NULL)    //??¡è???????????¡¦????¡§?????????????????¡¦?????????????????????????????????????????????????
	{
		perror("getcwd error");
	}
	else
	{
   		//printf("PrePath: %s\n", g_prePath);
	}
    Dl_info dl_info;        //???????????????
    if(dladdr(Fun, &dl_info))        //??????????????¡ã?¡ã??????¡¦????????????
	{
		ret = 0;
		sFilePath = strdup(dl_info.dli_fname);
 		char *pName = strrchr(sFilePath, '/');    //?????¡ã???????¡¦???????????????????"/"
		*pName = '\0';                            //??????????????????/???????????????????????????¨¦??????¡¦????
	}
	return ret;
}
static void emptyfunc() {}
inline std::string GetCurrentModulePath() {
	//logd("");
	static std::string ret;
	if(ret == ""){
		char* buf=nullptr;
		auto rc = GetModuleFileName( (void*)emptyfunc, buf);
		ret=buf;
		free(buf);
	}
	printf("dir:%s\n",ret.c_str());
	return ret;
}
#endif

template<class T=int>
class Log {
public:
    static Log * Instance(const std::string &dir = ""){
		if (nullptr == instance) {
			if (dir.empty()) {
				instance = std::make_unique<Log>(GetCurrentModulePath() + "/log");
			}
			else {
				instance = std::make_unique<Log>(dir);
			}
		}
		return instance.get();
	}
    void Release(){
		if (nullptr != instance) {
			auto ptr = instance.release();
			delete ptr;
		}
	}
    Log(const std::string &dir): logger(nullptr) {
		#ifdef WIN32
			if (::PathFileExistsA(dir.data())) {
		#else
			if (access(dir.data(),F_OK) != -1) {
		#endif
			logger = std::move(spdlog::rotating_logger_st("ks-java-dll", dir + "/ZHX_ks_java_c_dll.log", 500 * 1024 * 1024, 3));
			spdlog::set_default_logger(logger);
		}


		spdlog::set_level(spdlog::level::warn);
		spdlog::flush_on(spdlog::level::warn);
	}

    std::shared_ptr<spdlog::logger> logger;
protected:
    static std::unique_ptr<Log> instance;
};

template<class T>
std::unique_ptr<Log<T>> Log<T>::instance;

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

//???????????????????????????????????????????????????? ?????????????? ?????????????????? ???????? ???????¡§??????
#ifndef suffix
#define suffix(msg)  std::string(msg).append("  [ ")\
        .append(__FILENAME__).append(":").append(std::to_string(__LINE__))\
		.append(" ] <").append(__FUNCTION__)\
        .append(">").c_str()
#endif

#ifdef WIN32
    #define logt(msg,...) {Log<int>::Instance(); spdlog::trace(suffix(msg),__VA_ARGS__);}
    #define logd(msg,...) {Log<int>::Instance(); spdlog::debug(suffix(msg),__VA_ARGS__);}
    #define logi(msg,...) {Log<int>::Instance(); spdlog::info(suffix(msg),__VA_ARGS__);}
    #define logw(msg,...) {Log<int>::Instance(); spdlog::warn(suffix(msg),__VA_ARGS__);}
    #define loge(msg,...) {Log<int>::Instance(); spdlog::error(suffix(msg),__VA_ARGS__);}
    #define logc(msg,...) {Log<int>::Instance(); spdlog::critical(suffix(msg),__VA_ARGS__);}
#else
    #define logt(msg,args...) Log<int>::Instance(); spdlog::trace(suffix(msg), ##args)
    #define logd(msg,args...) Log<int>::Instance(); spdlog::debug(suffix(msg),##args)
    #define logi(msg,args...) Log<int>::Instance(); spdlog::info(suffix(msg),##args)
    #define logw(msg,args...) Log<int>::Instance(); spdlog::warn(suffix(msg),##args)
    #define loge(msg,args...) Log<int>::Instance(); spdlog::error(suffix(msg),##args)
    #define logc(msg,args...) Log<int>::Instance(); spdlog::critical(suffix(msg),##args)
#endif

inline void setLogLevel(int level) {
    Log<int>::Instance();
    spdlog::set_level((spdlog::level::level_enum)(level));
    spdlog::flush_on((spdlog::level::level_enum)(level));
}