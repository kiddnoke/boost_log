// log.cpp : 定义控制台应用程序的入口点。
//
#include "log.h"
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
	typedef sinks::asynchronous_sink<sinks::text_file_backend> file_sink;
	typedef boost::shared_ptr<file_sink> file_sink_ptr;
	typedef src::severity_channel_logger<severity_level, std::string> log_channel;
	typedef boost::shared_ptr<log_channel> log_channel_ptr;
	std::string m_path;
	unsigned m_txtlev;
	unsigned m_flushLev;
	boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> m_consoleBackend;
	std::map<std::string, log_channel_ptr> m_channels;

  public:
	LogEngine() : m_path("logs/"), m_txtlev(sevlvl_info), m_flushLev(sevlvl_error) {}
	static LogEngine &Instance()
	{
		static LogEngine s_ins;
		return s_ins;
	}

	void Init(const std::string &ph);

	void SetLogLevel(unsigned int lev) { m_txtlev = lev; }

	// 控制台Lev必须大于文本Lev, 不然不会输出
	void SetConsolePrintLevel(unsigned int consolePrintLevel) { m_consoleBackend->set_filter(expr::attr<severity_level>("Severity") >= consolePrintLevel); }

	void SetFlushLevel(unsigned int flushLevel) { m_flushLev = flushLevel; }

	void ForceLogOut() { logging::core::get()->flush(); }

	const char *GetLogDirPath() const { return m_path.c_str(); }

	void Write(const char *module, unsigned int logLevel, const char *msg);

  protected:
	file_sink_ptr AddFileSink(const std::string &name);
};

std::ostream& operator<<(std::ostream& strm, const severity_level& level)
{
	static const char* strings[] =
	{
		"TRACE",
		"DEBUG",
		"INFO",
		"WARN",
		"ERROR",
		"FATAL"
	};

	if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
		strm << strings[level];
	else
		strm << static_cast< int >(level);

	return strm;
}

LogEngine::file_sink_ptr  LogEngine::AddFileSink(const std::string& name) {
	std::stringstream ph;
	ph << m_path << name << "-%Y-%m-%d_%H-%M-%S.log";

	file_sink_ptr sink(new file_sink(
		keywords::file_name = ph.str(),      //文件名
		keywords::rotation_size = 10 * 1024 * 1024,       //单个文件限制大小
		keywords::time_based_rotation=sinks::file::rotation_at_time_point(0,0,0),    //每天重建
		keywords::auto_flush = true
		));

	//设置日志内容输出格式
	sink->set_formatter(expr::format("%1% [%2%]: %3%")
		% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
		% expr::attr<severity_level>("Severity")
		% expr::smessage);
// 	sink->set_formatter(expr::stream 
// 		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S ")
// 		<< expr::attr<severity_level>("Severity")
// 		<< expr::smessage);
	sink->locked_backend()->set_file_collector(sinks::file::make_collector(
		keywords::target = m_path,                          // where to store rotated files
		keywords::max_size = 1 * 1024 * 1024 * 1024,              // maximum total size of the stored files, in bytes
		keywords::min_free_space = 100 * 1024 * 1024        // minimum free space on the drive, in bytes
		));

	// Upon restart, scan the target directory for files matching the file_name pattern
	sink->locked_backend()->scan_for_files();

	sink->set_filter(expr::attr<std::string>("Channel")==name);

	//     std::locale loc = boost::locale::generator()("zh_CN.UTF-8");
	//     sink->imbue(loc);

	logging::core::get()->add_sink(sink);
	return sink;
}

void LogEngine::Write(const char* module, unsigned int logLevel, const char* msg)
{
    if (logLevel<m_txtlev)
        return;
    log_channel_ptr& cnl = m_channels[module];
    if (!cnl)
    {
        AddFileSink(module); //AddFileSink("module-%Y-%m-%d_%H-%M-%S.log");
        cnl.reset(new log_channel(keywords::channel = module));
    }

    BOOST_LOG_SEV(*cnl, severity_level(logLevel))<< msg;

    if (logLevel>=m_flushLev)
        ForceLogOut();
}

void LogEngine::Init(const std::string& ph)
{
    // 路径末尾必须为 "/" 或者 "\"
    m_path = ph;

    if (m_path.size())
    {
        auto ch = *m_path.rbegin();
        if(ch != '/' || ch!='\\')
            m_path += '/';
    }    

    // 获取时间作为本次启动的日志目录
    const char* format = "%Y-%m-%d_%H-%M-%S";
    time_t now = time(NULL);
    char szDate[32];
    ::strftime(szDate, sizeof(szDate), format, localtime(&now));
    m_path.append(szDate);
    m_path += '/' ;


    m_channels.clear();
    logging::core::get()->remove_all_sinks();
    logging::add_common_attributes();

    m_consoleBackend = logging::add_console_log(std::clog, keywords::format =
        expr::format("[%1%] %2%: %3%")
        % expr::attr<severity_level>("Severity")
        % expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
        % expr::smessage
        );
    m_consoleBackend->set_filter(expr::attr<severity_level>("Severity")>=sevlvl_warn);
}

namespace Log
{
extern "C" LOG_API bool Log_Init(const char *szLogDirPath)
{
	LogEngine::Instance().Init(szLogDirPath);
	return true;
	}

	// 设置写Log等级
	// loglevel: 当等级高于或等于该设置时才写入Log文件，默认eLogLevel_Info
	extern "C" LOG_API void Log_SetLogLevel(unsigned int loglevel)
	{
		LogEngine::Instance().SetLogLevel(loglevel);
	}

	// 设置Log输出到控制台等级
	// flushLevel: 当等级高于或等于该设置时输出到控制台，默认eLogLevel_Warning
	extern "C" LOG_API void Log_SetConsolePrintLevel(unsigned int consolePrintLevel)
	{
		LogEngine::Instance().SetConsolePrintLevel(consolePrintLevel);
	}

	// 设置Log立即Flush等级
	// flushLevel: 当等级高于或等于该设置时立即Flush文件，默认eLogLevel_Error
	extern "C" LOG_API void Log_SetFlushLevel(unsigned int flushLevel)
	{
		LogEngine::Instance().SetFlushLevel(flushLevel);
	}

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// logLevel:  该条Log等级
	// fmt: log内容格式，后跟不定长参数
	extern "C" LOG_API bool Log_Save(const char *pszModule, unsigned int logLevel, const char *fmt, ...)
	{
		char text[2048];
		va_list ap;
		va_start(ap, fmt);
		::vsnprintf(text, sizeof(text), fmt, ap);
		va_end(ap);

		LogEngine::Instance().Write(pszModule, logLevel, text);
		return true;
	}

	// 写Log
	// pszModule: 模块名，不同模块写不同log文件
	// fmt: log内容格式，后跟不定长参数
	// 使用log等级为 eLogLevel_Info
	extern "C" LOG_API bool LogSave(const char *pszModule, const char *fmt, ...)
	{
		char text[2048];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(text, sizeof(text), fmt, ap);
		va_end(ap);

		LogEngine::Instance().Write(pszModule, sevlvl_info, text);
		return true;
	}


	// 强制输出日志队列中的信息
	extern "C" LOG_API void Log_ForceLogOut()
	{
		LogEngine::Instance().ForceLogOut();
	}


	// 获取当前日志目录路径
	extern "C" LOG_API const char *Log_GetLogDirPath()
	{
		return LogEngine::Instance().GetLogDirPath();
	}

	extern "C" LOG_API bool Log_Setup(const char *LogDirPath, unsigned int loglevel,
									 const unsigned int PrintLevel, const unsigned int FlushLevel)
	{
		LogEngine::Instance().Init(LogDirPath);
		LogEngine::Instance().SetLogLevel(loglevel);
		LogEngine::Instance().SetConsolePrintLevel(PrintLevel);
		LogEngine::Instance().SetFlushLevel(FlushLevel);
		return true ;
	}

}