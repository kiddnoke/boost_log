// log.cpp : 定义控制台应用程序的入口点。
//
#ifndef LOG_LOG_H
#define LOG_LOG_H
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <stdio.h>
#include <stdarg.h>

#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;

enum severity_level
{
	sevlvl_trace,
	sevlvl_debug,
	sevlvl_info,
	sevlvl_warn,
	sevlvl_error,
	sevlvl_fatal
};

class LogEngine
{
	typedef sinks::asynchronous_sink< sinks::text_file_backend > file_sink;
	typedef boost::shared_ptr<file_sink> file_sink_ptr;
	typedef src::severity_channel_logger<severity_level, std::string> log_channel;
	typedef boost::shared_ptr<log_channel> log_channel_ptr;
	std::string m_path;
	unsigned m_txtlev;
	unsigned m_flushLev;
	boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend> > m_consoleBackend;
	std::map<std::string,log_channel_ptr> m_channels;
public:
	LogEngine() : m_path("logs/"), m_txtlev(sevlvl_info), m_flushLev(sevlvl_error) {}
	static LogEngine& Instance()
	{
		static LogEngine s_ins;
		return s_ins;
	}

	void Init(const std::string& ph);

	void SetLogLevel(unsigned int lev) { m_txtlev = lev; }

	// 控制台Lev必须大于文本Lev, 不然不会输出
	void SetConsolePrintLevel(unsigned int consolePrintLevel) { m_consoleBackend->set_filter(expr::attr<severity_level>("Severity")>=consolePrintLevel); }

	void SetFlushLevel(unsigned int flushLevel) { m_flushLev = flushLevel; }

	void ForceLogOut() { logging::core::get()->flush(); }

	const char* GetLogDirPath() const { return m_path.c_str(); }

	void Write(const char* module, unsigned int logLevel, const char* msg);
protected:
	file_sink_ptr AddFileSink(const std::string& name);
};
 

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
	extern "C"      bool	Log_Init(const char *szLogDirPath = "log");

	// 设置写Log等级
	// loglevel: 当等级高于或等于该设置时才写入Log文件，默认eLogLevel_Info
	extern "C" void	Log_SetLogLevel(unsigned int loglevel);

	// 设置Log输出到控制台等级
	// flushLevel: 当等级高于或等于该设置时输出到控制台，默认eLogLevel_Warning
	extern "C" void	Log_SetConsolePrintLevel(unsigned int consolePrintLevel);

	// 设置Log立即Flush等级
	// flushLevel: 当等级高于或等于该设置时立即Flush文件，默认eLogLevel_Error
	extern "C" void	Log_SetFlushLevel(unsigned int flushLevel);

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// logLevel:  该条Log等级
	// fmt: log内容格式，后跟不定长参数
	extern "C" bool	Log_Save(const char* pszModule, unsigned int logLevel, const char* fmt, ...);

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// fmt: log内容格式，后跟不定长参数
	// 使用log等级为 eLogLevel_Info
	extern "C" bool	LogSave(const char* pszModule, const char* fmt, ...);

	// 强制输出日志队列中的信息
	extern "C" void	Log_ForceLogOut();

	// 获取当前日志目录路径, 需要"\"或"/"结尾
	extern "C" const char* Log_GetLogDirPath();
	//
	extern "C" bool Log_Setup(const char* LogDirPath = "log" , unsigned int loglevel = eLogLevel_Info ,const unsigned int PrintLevel = eLogLevel_Error  , const unsigned int FlushLevel = eLogLevel_Error ) ;
}

#endif // !LOG_LOG_H