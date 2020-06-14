#include "../Headers/Logger.h"

#include <map>
#include <thread>

namespace SimpleLog
{

namespace
{
#ifndef NDEBUG
std::atomic<LogType> log_type_(LogType::Debug);
#else
std::atomic<LogType> log_type_(LogType::Release);
#endif

std::atomic<uint32_t> log_infos_(
	static_cast<uint32_t>(LogInfos::ThreadId) |
	static_cast<uint32_t>(LogInfos::FileNameWithLine) |
	static_cast<uint32_t>(LogInfos::TimeStamp));

std::atomic<uint32_t> log_message_types_(
	static_cast<uint32_t>(LogMessageType::Error) |
	static_cast<uint32_t>(LogMessageType::Info) |
	static_cast<uint32_t>(LogMessageType::Warning) |
	static_cast<uint32_t>(LogMessageType::FatalError));

std::atomic<std::ostream*> log_stream_(&std::cout);
std::atomic<std::ostream*> elog_stream_(&std::cerr);

void GetTimeStamp(char buffer[64])
{
	const auto now = std::time(nullptr);
	struct std::tm* ptmgm = gmtime(&now); // GMT
	std::strftime(buffer, 64, "%d-%m-%Y(%H:%M:%S)", ptmgm);
}

std::string MessageTypeToString(const LogMessageType message_type)
{
	static const std::map<LogMessageType, std::string> type_to_string = {
		{LogMessageType::Info,        "I"},
		{LogMessageType::Warning,     "W"},
		{LogMessageType::Error,       "E"},
		{LogMessageType::FatalError,  "F"}
	};
	const auto it = type_to_string.find(message_type);
	return it != type_to_string.cend() ? it->second : "I";
}

void PrintInfos(
	std::stringstream& ss,
	const LogMessageType message_type,
	const char* const file_name,
	const int line)
{
	const auto type = GetLogInfos();
	ss << "[" << MessageTypeToString(message_type) << "]";
	if (type == 0)
	{
		return;
	}

	if ((type & static_cast<uint32_t>(LogInfos::TimeStamp)) != 0)
	{
		char buffer[64];
		GetTimeStamp(buffer);
		ss << "[(GMT)" << buffer << "]";
	}

	if ((type & static_cast<uint32_t>(LogInfos::ThreadId)) != 0)
	{
		ss << "[" << std::this_thread::get_id() << "]";
	}

	if ((type & static_cast<uint32_t>(LogInfos::FileNameWithLine)) != 0)
	{
		ss << "[" << file_name << ":" << line << "]";
	}
}

} // namespace

LogType GetLogType()
{
	return log_type_.load();
}

void SetLogType(const LogType log_type)
{
	log_type_.store(log_type);
}

uint32_t GetLogMessageTypes()
{
	return log_message_types_.load();
}

void SetLogMessageTypes(const uint32_t log_message_types)
{
	log_message_types_.store(log_message_types);
}

uint32_t GetLogInfos()
{
	return log_infos_.load();
}

void SetLogInfos(const uint32_t log_infos)
{
	log_infos_.store(log_infos);
}

std::ostream& GetLogStream()
{
	return *log_stream_.load();
}

void SetLogStream(std::ostream& stream)
{
	log_stream_.store(&stream);
}

std::ostream& GetELogStream()
{
	return *elog_stream_.load();
}

void SetELogStream(std::ostream& stream)
{
	elog_stream_.store(&stream);
}

Logger::Logger(
	std::ostream& out_str,
	const LogMessageType message_type,
	const char* const file_name,
	const int line)
	: out_str_(out_str)
{
	PrintInfos(ss_, message_type, file_name, line);
	ss_ << "$ ";
}

Logger::~Logger()
{
	ss_ << "\n";
	out_str_ << ss_.str();
}

} // namespace SimpleLog