// log.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "log.h"
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
		keywords::file_name = ph.str(),      //�ļ���
		keywords::rotation_size = 10 * 1024 * 1024,       //�����ļ����ƴ�С
		keywords::time_based_rotation=sinks::file::rotation_at_time_point(0,0,0),    //ÿ���ؽ�
		keywords::auto_flush = true
		));

	//������־���������ʽ
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
    // ·��ĩβ����Ϊ "/" ���� "\"
    m_path = ph;

    if (m_path.size())
    {
        auto ch = *m_path.rbegin();
        if(ch != '/' || ch!='\\')
            m_path += '/';
    }    

    // ��ȡʱ����Ϊ������������־Ŀ¼
    static const char* format = "%Y-%m-%d_%H-%M-%S";
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
	extern "C"   bool	Log_Init(const char *szLogDirPath)
	{
		LogEngine::Instance().Init(szLogDirPath);
		return true;
	}

	// ����дLog�ȼ�
	// loglevel: ���ȼ����ڻ���ڸ�����ʱ��д��Log�ļ���Ĭ��eLogLevel_Info
	extern "C"  void	Log_SetLogLevel(unsigned int loglevel)
	{
		LogEngine::Instance().SetLogLevel(loglevel);
	}

	// ����Log���������̨�ȼ�
	// flushLevel: ���ȼ����ڻ���ڸ�����ʱ���������̨��Ĭ��eLogLevel_Warning
	extern "C"  void	Log_SetConsolePrintLevel(unsigned int consolePrintLevel)
	{
		LogEngine::Instance().SetConsolePrintLevel(consolePrintLevel);
	}

	// ����Log����Flush�ȼ�
	// flushLevel: ���ȼ����ڻ���ڸ�����ʱ����Flush�ļ���Ĭ��eLogLevel_Error
	extern "C"  void	Log_SetFlushLevel(unsigned int flushLevel)
	{
		LogEngine::Instance().SetFlushLevel(flushLevel);
	}

	// дLog
	// pszModule: ģ��������ͬģ��д��ͬlog�ļ�
	// logLevel:  ����Log�ȼ�
	// fmt: log���ݸ�ʽ���������������
	extern "C"  bool	Log_Save(const char* pszModule, unsigned int logLevel, const char* fmt, ...)
	{
		char text[2048];
		va_list ap;
		va_start(ap, fmt);
		::vsnprintf(text, sizeof(text), fmt, ap);
		va_end(ap);

		LogEngine::Instance().Write(pszModule, logLevel, text);
		return true;
	}

	// дLog
	// pszModule: ģ��������ͬģ��д��ͬlog�ļ�
	// fmt: log���ݸ�ʽ���������������
	// ʹ��log�ȼ�Ϊ eLogLevel_Info
	extern "C"  bool	LogSave(const char* pszModule, const char* fmt, ...)
	{
		char text[2048];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(text, sizeof(text), fmt, ap);
		va_end(ap);

		LogEngine::Instance().Write(pszModule, sevlvl_info, text);
		return true;
	}


	// ǿ�������־�����е���Ϣ
	extern "C"   void	Log_ForceLogOut()
	{
		LogEngine::Instance().ForceLogOut();
	}


	// ��ȡ��ǰ��־Ŀ¼·��
	extern "C"  const char* Log_GetLogDirPath()
	{
		return LogEngine::Instance().GetLogDirPath();
	}
 
	extern "C" bool Log_Setup(const char* LogDirPath , unsigned int loglevel ,
		const unsigned int PrintLevel , const unsigned int FlushLevel ) 
	{
		LogEngine::Instance().Init(LogDirPath);
		LogEngine::Instance().SetLogLevel(loglevel);
		LogEngine::Instance().SetConsolePrintLevel(PrintLevel);
		LogEngine::Instance().SetFlushLevel(FlushLevel);
		return true ;
	}

}