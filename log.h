// log.cpp : 定义控制台应用程序的入口点。
//
#ifndef LOG_LOG_H
#define LOG_LOG_H

#if defined(WIN)
#pragma message("编译Windows")
#ifdef LOG_EXPORTS
#define  LOG_API __declspec(dllexport)
#else
#define  LOG_API __declspec(dllimport)
#endif 
#elif defined(__GNUC__)
//gcc export or import
#pragma message("编译 GNUC")
#define LOG_API __attribute__((visibility("default")))
#endif 
 

namespace Log
{
	enum ELOG_LEVEL
	{
		eLogLevel_Trace		= 0,
		eLogLevel_Debug		= 1,
		eLogLevel_Info		= 2,
		eLogLevel_Warning	= 3,
		eLogLevel_Error		= 4,
		eLogLevel_Fatal		= 5,
	};

	// 初始话Log模块
	// szLogDirPath:    log输出目录 (在可执行文件目录创建，只能一层) 如：SysLog
	extern "C" LOG_API bool Log_Init(const char *szLogDirPath = "log");

	// 设置写Log等级
	// loglevel: 当等级高于或等于该设置时才写入Log文件，默认eLogLevel_Info
	extern "C" LOG_API void Log_SetLogLevel(unsigned int loglevel);

	// 设置Log输出到控制台等级
	// flushLevel: 当等级高于或等于该设置时输出到控制台，默认eLogLevel_Warning
	extern "C" LOG_API void Log_SetConsolePrintLevel(unsigned int consolePrintLevel);

	// 设置Log立即Flush等级
	// flushLevel: 当等级高于或等于该设置时立即Flush文件，默认eLogLevel_Error
	extern "C" LOG_API void Log_SetFlushLevel(unsigned int flushLevel);

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// logLevel:  该条Log等级
	// fmt: log内容格式，后跟不定长参数
	extern "C" LOG_API bool Log_Save(const char *pszModule, unsigned int logLevel, const char *fmt, ...);

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// fmt: log内容格式，后跟不定长参数
	// 使用log等级为 eLogLevel_Info
	extern "C" LOG_API bool LogSave(const char *pszModule, const char *fmt, ...);

	// 强制输出日志队列中的信息
	extern "C" LOG_API void Log_ForceLogOut();

	// 获取当前日志目录路径, 需要"\"或"/"结尾
	extern "C" LOG_API const char *Log_GetLogDirPath();
	//
	extern "C" LOG_API bool Log_Setup(const char *LogDirPath = "log", unsigned int loglevel = eLogLevel_Info, const unsigned int PrintLevel = eLogLevel_Error, const unsigned int FlushLevel = eLogLevel_Error);
}

#endif // !LOG_LOG_H