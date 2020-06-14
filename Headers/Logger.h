#pragma once
#include <atomic>
#include <iostream>
#include <type_traits>
#include <vector>
#include <shared_mutex>
#include <sstream>

namespace SimpleLog
{

enum class LogInfos : uint32_t
{
	ThreadId = 0x1,
	TimeStamp = 0x2,
	FileNameWithLine = 0x4
};

enum class LogMessageType : uint32_t
{
	Error = 0x1,
	Warning = 0x2,
	Info = 0x4,
	FatalError = 0x8,
};

enum class LogType : uint32_t
{
	Debug = 1,
	Release = 2,
};

class Logger
{
public:
	explicit Logger(
		std::ostream& out_str,
		const LogMessageType message_type,
		const char* file_name,
		const int line);

	template <typename T>
	Logger& operator<<(const T& value);
	~Logger();
private:
	std::stringstream ss_;
	std::ostream& out_str_;
};

template <typename T>
Logger& Logger::operator<<(const T& value)
{
	ss_ << value;
	return *this;
}

LogType GetLogType();
void SetLogType(const LogType log_type);

uint32_t GetLogMessageTypes();
void SetLogMessageTypes(const uint32_t log_message_types);

uint32_t GetLogInfos();
void SetLogInfos(const uint32_t log_infos);

std::ostream& GetLogStream();
void SetLogStream(std::ostream& stream);

std::ostream& GetELogStream();
void SetELogStream(std::ostream& stream);

} //namespace SimpleLog

#define LOG_MESSAGE_PRIVATE(ss, m) \
	if ((static_cast<uint32_t>(m) & SimpleLog::GetLogMessageTypes()) != 0) SimpleLog::Logger(ss, m, __FILE__, __LINE__)

#define LOG_FATAL_ERROR \
	LOG_MESSAGE_PRIVATE(SimpleLog::GetELogStream(), SimpleLog::LogMessageType::FatalError)
#define LOG_ERROR \
	LOG_MESSAGE_PRIVATE(SimpleLog::GetELogStream(), SimpleLog::LogMessageType::Error)
#define LOG_WARNING \
	LOG_MESSAGE_PRIVATE(SimpleLog::GetLogStream(), SimpleLog::LogMessageType::Warning)
#define LOG_INFO \
	LOG_MESSAGE_PRIVATE(SimpleLog::GetLogStream(), SimpleLog::LogMessageType::Info)

#define LOG_DEBUG_MESSAGE_PRIVATE(ss, m) \
	if (SimpleLog::LogType::Debug == SimpleLog::GetLogType()) LOG_MESSAGE_PRIVATE(ss, m)

#define DEBUG_LOG_ERROR \
	LOG_DEBUG_MESSAGE_PRIVATE(SimpleLog::GetELogStream(), SimpleLog::LogMessageType::Error)
#define DEBUG_LOG_WARNING \
	LOG_DEBUG_MESSAGE_PRIVATE(SimpleLog::GetLogStream(), SimpleLog::LogMessageType::Warning)
#define DEBUG_LOG_INFO \
	LOG_DEBUG_MESSAGE_PRIVATE(SimpleLog::GetLogStream(), SimpleLog::LogMessageType::Info)

#define PRIVATE_EMPTY_BLOCK do {} while(false)
#define PRIVATE_IF_CONDITION(condition) if (!(condition))

#define CHECK_RETURN(condition, ...) \
	PRIVATE_IF_CONDITION(condition) return __VA_ARGS__
#define CHECK_CONTINUE(condition) \
	PRIVATE_IF_CONDITION(condition) continue
#define CHECK_BREAK(condition) \
	PRIVATE_IF_CONDITION(condition) break
#define CHECK_THROW(condition, message) \
	PRIVATE_IF_CONDITION(condition) throw std::runtime_error(message)
#define CHECK_CUSTOM_THROW(condition, exc) \
	PRIVATE_IF_CONDITION(condition) throw exc

#define PRIVATE_CHECK(condition, out_stream, message, action) \
	PRIVATE_IF_CONDITION(condition) { out_stream << message; action; } PRIVATE_EMPTY_BLOCK

#define PRIVATE_ADD_EQUAL_FALSE(arg1) #arg1 " = false"

#define CHECK_FLOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, LOG_FATAL_ERROR, message, return __VA_ARGS__)
#define CHECK_ELOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, LOG_ERROR, message, return __VA_ARGS__)
#define CHECK_WLOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, LOG_WARNING, message, return __VA_ARGS__)
#define CHECK_ILOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, LOG_INFO, message, return __VA_ARGS__)
#define CHECK_DELOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, DEBUG_LOG_ERROR, message, return __VA_ARGS__)
#define CHECK_DWLOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, DEBUG_LOG_WARNING, message, return __VA_ARGS__)
#define CHECK_DILOG_RETURN(condition, message, ...) PRIVATE_CHECK(condition, DEBUG_LOG_INFO, message, return __VA_ARGS__)


#define CHECK_FLOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, LOG_FATAL_ERROR, message, continue)
#define CHECK_ELOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, LOG_ERROR, message, continue)
#define CHECK_WLOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, LOG_WARNING, message, continue)
#define CHECK_ILOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, LOG_INFO, message, continue)
#define CHECK_DELOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, DEBUG_LOG_ERROR, message, continue)
#define CHECK_DWLOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, DEBUG_LOG_WARNING, message, continue)
#define CHECK_DILOG_CONTINUE(condition, message) PRIVATE_CHECK(condition, DEBUG_LOG_INFO, message, continue)

#define CHECK_FLOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, LOG_FATAL_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_ELOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, LOG_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_WLOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, LOG_WARNING, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_ILOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, LOG_INFO, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_DELOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, DEBUG_LOG_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_DWLOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, DEBUG_LOG_WARNING, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)
#define CHECK_DILOG_AUTO_RETURN(condition, ...) PRIVATE_CHECK(condition, DEBUG_LOG_INFO, PRIVATE_ADD_EQUAL_FALSE(condition), return __VA_ARGS__)

#define CHECK_FLOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, LOG_FATAL_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_ELOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, LOG_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_WLOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, LOG_WARNING, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_ILOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, LOG_INFO, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_DELOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, DEBUG_LOG_ERROR, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_DWLOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, DEBUG_LOG_WARNING, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
#define CHECK_DILOG_AUTO_CONTINUE(condition) PRIVATE_CHECK(condition, DEBUG_LOG_INFO, PRIVATE_ADD_EQUAL_FALSE(condition), continue)
