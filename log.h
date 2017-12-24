// log.cpp : �������̨Ӧ�ó������ڵ㡣
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

	// ����̨Lev��������ı�Lev, ��Ȼ�������
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

	// ��ʼ��Logģ��
	// szLogDirPath:    log���Ŀ¼ (�ڿ�ִ���ļ�Ŀ¼������ֻ��һ��) �磺SysLog
	extern "C"      bool	Log_Init(const char *szLogDirPath = "log");

	// ����дLog�ȼ�
	// loglevel: ���ȼ����ڻ���ڸ�����ʱ��д��Log�ļ���Ĭ��eLogLevel_Info
	extern "C" void	Log_SetLogLevel(unsigned int loglevel);

	// ����Log���������̨�ȼ�
	// flushLevel: ���ȼ����ڻ���ڸ�����ʱ���������̨��Ĭ��eLogLevel_Warning
	extern "C" void	Log_SetConsolePrintLevel(unsigned int consolePrintLevel);

	// ����Log����Flush�ȼ�
	// flushLevel: ���ȼ����ڻ���ڸ�����ʱ����Flush�ļ���Ĭ��eLogLevel_Error
	extern "C" void	Log_SetFlushLevel(unsigned int flushLevel);

	// дLog
	// pszModule: ģ��������ͬģ��д��ͬlog�ļ�
	// logLevel:  ����Log�ȼ�
	// fmt: log���ݸ�ʽ���������������
	extern "C" bool	Log_Save(const char* pszModule, unsigned int logLevel, const char* fmt, ...);

	// дLog
	// pszModule: ģ��������ͬģ��д��ͬlog�ļ�
	// fmt: log���ݸ�ʽ���������������
	// ʹ��log�ȼ�Ϊ eLogLevel_Info
	extern "C" bool	LogSave(const char* pszModule, const char* fmt, ...);

	// ǿ�������־�����е���Ϣ
	extern "C" void	Log_ForceLogOut();

	// ��ȡ��ǰ��־Ŀ¼·��, ��Ҫ"\"��"/"��β
	extern "C" const char* Log_GetLogDirPath();
	//
	extern "C" bool Log_Setup(const char* LogDirPath = "log" , unsigned int loglevel = eLogLevel_Info ,const unsigned int PrintLevel = eLogLevel_Error  , const unsigned int FlushLevel = eLogLevel_Error ) ;
}

#endif // !LOG_LOG_H