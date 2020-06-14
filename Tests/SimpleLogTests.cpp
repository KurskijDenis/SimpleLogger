#include <Logger.h>
#include <gtest/gtest.h>
#include <thread>

namespace SimpleLog
{

namespace
{

class TestException : public std::exception
{

public:
	TestException(std::string what)
		: what_(std::move(what))
	{}

	const char*	what() const noexcept override
	{
		return what_.c_str();
	}

private:
	const std::string what_;
};

const std::string g_file_name(__FILE__);

void GetTimeStamp(char buffer1[64], char buffer2[64])
{
	static const char* string_format = "%d-%m-%Y(%H:%M:%S)";
	const auto now1 = std::time(nullptr);
	{
		struct std::tm* ptmgm = gmtime(&now1); // GMT
		std::strftime(buffer1, 64, string_format, ptmgm);
	}
	{
		const auto now2 = now1 + 1;
		struct std::tm* ptmgm = gmtime(&now2); // GMT
		std::strftime(buffer2, 64, string_format, ptmgm);
	}
}

class LoggerTestClass : public ::testing::Test
{

protected:

	void SetUp() override
	{
		SetLogType(LogType::Debug);
		SetLogInfos(static_cast<uint32_t>(LogInfos::ThreadId));
		SetLogMessageTypes(
			static_cast<uint32_t>(LogMessageType::Error) |
			static_cast<uint32_t>(LogMessageType::Info) |
			static_cast<uint32_t>(LogMessageType::Warning) |
			static_cast<uint32_t>(LogMessageType::FatalError));
	}

	void TearDown() override
	{
		SetLogStream(std::cout);
		SetELogStream(std::cout);
	}

};

} // namespace

TEST(LoggerTest, TestLoggerWithFileName)
{
	SetLogInfos(static_cast<uint32_t>(LogInfos::FileNameWithLine));
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Error, "FileName", 32);
		logger << "Message Test";
	}

	const std::string expected_string("[E][FileName:32]$ Message Test\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestELoggerWithoutAdditionalInfos)
{
	SetLogInfos(0);
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Error, "FileName", 32);
		logger << "Message Test";
	}
	const std::string expected_string("[E]$ Message Test\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestWLoggerWithoutAdditionalInfos)
{
	SetLogInfos(0);
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Warning, "FileName", 32);
		logger << "Message Test1";
	}
	const std::string expected_string("[W]$ Message Test1\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestILoggerWithoutAdditionalInfos)
{
	SetLogInfos(0);
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Info, "FileName", 32);
		logger << "Message Test2";
	}
	const std::string expected_string("[I]$ Message Test2\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestFLoggerWithoutAdditionalInfos)
{
	SetLogInfos(0);
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::FatalError, "FileName", 32);
		logger << "Message Test3";
	}
	const std::string expected_string("[F]$ Message Test3\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestILoggerWithThreadInfo)
{
	SetLogInfos(static_cast<uint32_t>(LogInfos::ThreadId));
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Info, "FileName", 32);
		logger << "Message Test";
	}
	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();
	const std::string expected_string("[I][" + os_thread.str() + "]$ Message Test\n");
	EXPECT_EQ(expected_string, os.str());
}

TEST(LoggerTest, TestWLoggerWithTimeStamp)
{
	SetLogInfos(static_cast<uint32_t>(LogInfos::TimeStamp));
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Warning, "FileName", 32);
		logger << "Message Test";
	}
	char buffer1[64];
	char buffer2[64];
	GetTimeStamp(buffer1, buffer2);

	const std::string expected_string1("[W][(GMT)" + std::string(buffer1) + "]$ Message Test\n");
	const std::string expected_string2("[W][(GMT)" + std::string(buffer2) + "]$ Message Test\n");
	const auto result_string = os.str();
	EXPECT_TRUE(expected_string1 == result_string || expected_string2 == result_string);
}

TEST(LoggerTest, TestELoggerWithAllInfo)
{
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::TimeStamp) |
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));
	std::ostringstream os;
	{
		Logger logger(os, LogMessageType::Warning, "FileName", 32);
		logger << "Message Test";
	}

	char buffer1[64];
	char buffer2[64];
	GetTimeStamp(buffer1, buffer2);

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();

	const std::string expected_string1("[W][(GMT)" + std::string(buffer1) + "][" + os_thread.str() +"][FileName:32]$ Message Test\n");
	const std::string expected_string2("[W][(GMT)" + std::string(buffer2) + "][" + os_thread.str() +"][FileName:32]$ Message Test\n");
	const auto result_string = os.str();
	EXPECT_TRUE(expected_string1 == result_string || expected_string2 == result_string);
}

TEST_F(LoggerTestClass, TestEMacros)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetELogStream(os);

	const auto line = std::to_string(__LINE__ + 1);
	LOG_ERROR << "Some message";

	const auto line_debug = std::to_string(__LINE__ + 1);
	DEBUG_LOG_ERROR << "Some message debug";

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();
	std::string expected_string("[E][" + os_thread.str() +"][" + g_file_name + ":" + line + "]$ Some message\n");
	expected_string += "[E][" + os_thread.str() +"][" + g_file_name + ":" + line_debug + "]$ Some message debug\n";

	EXPECT_EQ(expected_string, os.str());
}

TEST_F(LoggerTestClass, TestWMacros)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetLogStream(os);

	const auto line = std::to_string(__LINE__ + 1);
	LOG_WARNING << "Some message";

	const auto line_debug = std::to_string(__LINE__ + 1);
	DEBUG_LOG_WARNING << "Some message debug";

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();

	std::string expected_string("[W][" + os_thread.str() +"][" + g_file_name + ":" + line + "]$ Some message\n");
	expected_string += "[W][" + os_thread.str() +"][" + g_file_name + ":" + line_debug + "]$ Some message debug\n";

	EXPECT_EQ(expected_string, os.str());
}

TEST_F(LoggerTestClass, TestIMacros)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetLogStream(os);

	const auto line = std::to_string(__LINE__ + 1);
	LOG_INFO << "Some message";

	const auto line_debug = std::to_string(__LINE__ + 1);
	DEBUG_LOG_INFO << "Some message debug";

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();

	std::string expected_string("[I][" + os_thread.str() +"][" + g_file_name + ":" + line + "]$ Some message\n");
	expected_string += "[I][" + os_thread.str() +"][" + g_file_name + ":" + line_debug + "]$ Some message debug\n";

	EXPECT_EQ(expected_string, os.str());
}

TEST_F(LoggerTestClass, TestFMacros)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetELogStream(os);

	const auto line = std::to_string(__LINE__ + 1);
	LOG_FATAL_ERROR << "Some message";

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();

	const std::string expected_string("[F][" + os_thread.str() +"][" + g_file_name + ":" + line + "]$ Some message\n");

	EXPECT_EQ(expected_string, os.str());
}

TEST_F(LoggerTestClass, TestDebugLogsInRelease)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetLogMessageTypes(
		static_cast<uint32_t>(LogMessageType::FatalError));

	SetLogStream(os);
	SetELogStream(os);

	DEBUG_LOG_INFO << "Some message1";
	DEBUG_LOG_WARNING << "Some message3";
	DEBUG_LOG_ERROR << "Some message2";

	EXPECT_EQ("", os.str());
}

TEST_F(LoggerTestClass, TestIfIWMacrosDisable)
{
	std::ostringstream os;
	SetLogInfos(
		static_cast<uint32_t>(LogInfos::ThreadId) |
		static_cast<uint32_t>(LogInfos::FileNameWithLine));

	SetLogMessageTypes(static_cast<uint32_t>(LogMessageType::Error));
	SetELogStream(os);

	LOG_INFO << "Some message1";
	LOG_WARNING << "Some message2";

	DEBUG_LOG_INFO << "Some message debug3";
	DEBUG_LOG_WARNING << "Some message debug4";

	const auto line = std::to_string(__LINE__ + 1);
	LOG_ERROR << "Some message";

	const auto line_debug = std::to_string(__LINE__ + 1);
	DEBUG_LOG_ERROR << "Some message debug";

	std::ostringstream os_thread;
	os_thread << std::this_thread::get_id();

	std::string expected_string("[E][" + os_thread.str() +"][" + g_file_name + ":" + line + "]$ Some message\n");
	expected_string += "[E][" + os_thread.str() +"][" + g_file_name + ":" + line_debug + "]$ Some message debug\n";

	EXPECT_EQ(expected_string, os.str());
}

TEST_F(LoggerTestClass, TestReturnCheckILogs)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_INFO << "Some message1";
		SetLogStream(os3);
		CHECK_ILOG_RETURN(true, "Skip message1");
		SetLogStream(os2);
		CHECK_ILOG_RETURN(false, "Some message1");
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_INFO << "Some message2";
		SetLogStream(os3);
		CHECK_DILOG_RETURN(true, "Skip message2");
		SetLogStream(os2);
		CHECK_DILOG_RETURN(false, "Some message2");
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_INFO << "Some message2";
		SetLogStream(os3);
		CHECK_ILOG_RETURN(true, "Skip message2", 2);
		SetLogStream(os2);
		CHECK_ILOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_INFO << "Some message2";
		SetLogStream(os3);
		CHECK_DILOG_RETURN(true, "Skip message2", 2);
		SetLogStream(os2);
		CHECK_DILOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckWLogs)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_WARNING << "Some message1";
		SetLogStream(os3);
		CHECK_WLOG_RETURN(true, "Skip message1");
		SetLogStream(os2);
		CHECK_WLOG_RETURN(false, "Some message1");
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_WARNING << "Some message2";
		SetLogStream(os3);
		CHECK_DWLOG_RETURN(true, "Skip message2");
		SetLogStream(os2);
		CHECK_DWLOG_RETURN(false, "Some message2");
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_WARNING << "Some message2";
		SetLogStream(os3);
		CHECK_WLOG_RETURN(true, "Skip message2", 2);
		SetLogStream(os2);
		CHECK_WLOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_WARNING << "Some message2";
		SetLogStream(os3);
		CHECK_DWLOG_RETURN(true, "Skip message2", 2);
		SetLogStream(os2);
		CHECK_DWLOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckELogs)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_ERROR << "Some message1";
		SetELogStream(os3);
		CHECK_ELOG_RETURN(true, "Skip message1");
		SetELogStream(os2);
		CHECK_ELOG_RETURN(false, "Some message1");
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		DEBUG_LOG_ERROR << "Some message2";
		SetELogStream(os3);
		CHECK_DELOG_RETURN(true, "Skip message2");
		SetELogStream(os2);
		CHECK_DELOG_RETURN(false, "Some message2");
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_ERROR << "Some message2";
		SetELogStream(os3);
		CHECK_ELOG_RETURN(true, "Skip message2", 2);
		SetELogStream(os2);
		CHECK_ELOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		DEBUG_LOG_ERROR << "Some message2";
		SetELogStream(os3);
		CHECK_DELOG_RETURN(true, "Skip message2", 2);
		SetELogStream(os2);
		CHECK_DELOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckFLogs)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_FATAL_ERROR << "Some message1";
		SetELogStream(os3);
		CHECK_FLOG_RETURN(true, "Skip message1");
		SetELogStream(os2);
		CHECK_FLOG_RETURN(false, "Some message1");
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_FATAL_ERROR << "Some message2";
		SetELogStream(os3);
		CHECK_FLOG_RETURN(true, "Skip message2", 2);
		SetELogStream(os2);
		CHECK_FLOG_RETURN(false, "Some message2", 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckILogsAutoMsg)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_INFO << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_ILOG_AUTO_RETURN(3 == 3);
		SetLogStream(os2);
		CHECK_ILOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_INFO << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_DILOG_AUTO_RETURN(3 == 3);
		SetLogStream(os2);
		CHECK_DILOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_INFO << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_ILOG_AUTO_RETURN(3 == 3, 2);
		SetLogStream(os2);
		CHECK_ILOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_INFO << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_DILOG_AUTO_RETURN(3 == 3, 2);
		SetLogStream(os2);
		CHECK_DILOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckWLogsAutoMsg)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_WARNING << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_WLOG_AUTO_RETURN(3 == 3);
		SetLogStream(os2);
		CHECK_WLOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_WARNING << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_DWLOG_AUTO_RETURN(3 == 3);
		SetLogStream(os2);
		CHECK_DWLOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		LOG_WARNING << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_WLOG_AUTO_RETURN(3 == 3, 2);
		SetLogStream(os2);
		CHECK_WLOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetLogStream(os);
		DEBUG_LOG_WARNING << "3 == 2 = false";
		SetLogStream(os3);
		CHECK_DWLOG_AUTO_RETURN(3 == 3, 2);
		SetLogStream(os2);
		CHECK_DWLOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckELogsAutoMsg)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_ELOG_AUTO_RETURN(3 == 3);
		SetELogStream(os2);
		CHECK_ELOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_debug = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		DEBUG_LOG_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_DELOG_AUTO_RETURN(3 == 3);
		SetELogStream(os2);
		CHECK_DELOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_ELOG_AUTO_RETURN(3 == 3, 2);
		SetELogStream(os2);
		CHECK_ELOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	const auto funct_debug_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		DEBUG_LOG_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_DELOG_AUTO_RETURN(3 == 3, 2);
		SetELogStream(os2);
		CHECK_DELOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();
	funct_debug();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(funct_debug_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestReturnCheckFLogsAutoMsg)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	const auto funct = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_FATAL_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_FLOG_AUTO_RETURN(3 == 3);
		SetELogStream(os2);
		CHECK_FLOG_AUTO_RETURN(3 == 2);
		EXPECT_TRUE(false);
	};

	const auto funct_return = [&os, &os2, &os3]()
	{
		SetELogStream(os);
		LOG_FATAL_ERROR << "3 == 2 = false";
		SetELogStream(os3);
		CHECK_FLOG_AUTO_RETURN(3 == 3, 2);
		SetELogStream(os2);
		CHECK_FLOG_AUTO_RETURN(3 == 2, 12);
		EXPECT_TRUE(false);
		return 2;
	};

	funct();

	EXPECT_EQ(funct_return(), 12);
	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST(LoggerTest, TestContinueAndBreak)
{
	size_t i = 0;
	for (; i < 10; ++i)
	{
		CHECK_CONTINUE(i == 0);
		EXPECT_TRUE(i == 0);
		CHECK_BREAK(i == 1);
	}
	EXPECT_EQ(i, 0);

	size_t j = 0;
	const auto funct = [&j]()
	{
		CHECK_RETURN(true);
		j = 1;
		CHECK_RETURN(false);
		j = 2;

	};

	funct();

	EXPECT_EQ(j, 1);
}

TEST_F(LoggerTestClass, TestContinueCheckLogs)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	size_t i = 0;
	for (; i < 10; ++i)
	{
		if (i == 0)
		{
			SetLogStream(os);
			LOG_INFO << "Message1";
			SetLogStream(os3);
			CHECK_ILOG_CONTINUE(i == 0, "Message");
			SetLogStream(os2);
			CHECK_ILOG_CONTINUE(i == 1, "Message1");
			FAIL();
		}
		if (i == 1)
		{
			SetLogStream(os);
			LOG_WARNING << "Message2";
			SetLogStream(os3);
			CHECK_WLOG_CONTINUE(i == 1, "Message");
			SetLogStream(os2);
			CHECK_WLOG_CONTINUE(i == 2, "Message2");
			FAIL();
		}
		if (i == 2)
		{
			SetELogStream(os);
			LOG_ERROR << "Message3";
			SetELogStream(os3);
			CHECK_ELOG_CONTINUE(i == 2, "Message");
			SetELogStream(os2);
			CHECK_ELOG_CONTINUE(i == 3, "Message3");
			FAIL();
		}
		if (i == 3)
		{
			SetELogStream(os);
			LOG_FATAL_ERROR << "Message4";
			SetELogStream(os3);
			CHECK_FLOG_CONTINUE(i == 3, "Message");
			SetELogStream(os2);
			CHECK_FLOG_CONTINUE(i == 4, "Message4");
			FAIL();
		}
		if (i == 4)
		{
			SetLogStream(os);
			DEBUG_LOG_INFO << "Message5";
			SetLogStream(os3);
			CHECK_DILOG_CONTINUE(i == 4, "Message");
			SetLogStream(os2);
			CHECK_DILOG_CONTINUE(i == 5, "Message5");
			FAIL();
		}
		if (i == 5)
		{
			SetLogStream(os);
			DEBUG_LOG_WARNING << "Message6";
			SetLogStream(os3);
			CHECK_DWLOG_CONTINUE(i == 5, "Message");
			SetLogStream(os2);
			CHECK_DWLOG_CONTINUE(i == 6, "Message6");
			FAIL();
		}
		if (i == 6)
		{
			SetELogStream(os);
			DEBUG_LOG_ERROR << "Message7";
			SetELogStream(os3);
			CHECK_DELOG_CONTINUE(i == 6, "Message");
			SetELogStream(os2);
			CHECK_DELOG_CONTINUE(i == 7, "Message7");
			FAIL();
		}
		break;
	}

	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST_F(LoggerTestClass, TestContinueCheckLogsAutoMsg)
{
	std::ostringstream os;
	std::ostringstream os2;
	std::ostringstream os3;

	size_t i = 0;
	for (; i < 10; ++i)
	{
		if (i == 0)
		{
			SetLogStream(os);
			LOG_INFO << "i == 1 = false";
			SetLogStream(os3);
			CHECK_ILOG_AUTO_CONTINUE(i == 0);
			SetLogStream(os2);
			CHECK_ILOG_AUTO_CONTINUE(i == 1);
			FAIL();
		}
		if (i == 1)
		{
			SetLogStream(os);
			LOG_WARNING << "i == 2 = false";
			SetLogStream(os3);
			CHECK_WLOG_AUTO_CONTINUE(i == 1);
			SetLogStream(os2);
			CHECK_WLOG_AUTO_CONTINUE(i == 2);
			FAIL();
		}
		if (i == 2)
		{
			SetELogStream(os);
			LOG_ERROR << "i == 3 = false";
			SetELogStream(os3);
			CHECK_ELOG_AUTO_CONTINUE(i == 2);
			SetELogStream(os2);
			CHECK_ELOG_AUTO_CONTINUE(i == 3);
			FAIL();
		}
		if (i == 3)
		{
			SetELogStream(os);
			LOG_FATAL_ERROR << "i == 4 = false";
			SetELogStream(os3);
			CHECK_FLOG_AUTO_CONTINUE(i == 3);
			SetELogStream(os2);
			CHECK_FLOG_AUTO_CONTINUE(i == 4);
			FAIL();
		}
		if (i == 4)
		{
			SetLogStream(os);
			DEBUG_LOG_INFO << "i == 5 = false";
			SetLogStream(os3);
			CHECK_DILOG_AUTO_CONTINUE(i == 4);
			SetLogStream(os2);
			CHECK_DILOG_AUTO_CONTINUE(i == 5);
			FAIL();
		}
		if (i == 5)
		{
			SetLogStream(os);
			DEBUG_LOG_WARNING << "i == 6 = false";
			SetLogStream(os3);
			CHECK_DWLOG_AUTO_CONTINUE(i == 5);
			SetLogStream(os2);
			CHECK_DWLOG_AUTO_CONTINUE(i == 6);
			FAIL();
		}
		if (i == 6)
		{
			SetELogStream(os);
			DEBUG_LOG_ERROR << "i == 7 = false";
			SetELogStream(os3);
			CHECK_DELOG_AUTO_CONTINUE(i == 6);
			SetELogStream(os2);
			CHECK_DELOG_AUTO_CONTINUE(i == 7);
			FAIL();
		}
		break;
	}

	EXPECT_EQ(os2.str(), os.str());
	EXPECT_EQ(os3.str(), "");
}

TEST(LoggerTest, TestThrowExceptions)
{
	try
	{
		try
		{
			CHECK_THROW(true, "Error");
		}
		catch (const std::exception& exc)
		{
			FAIL();
		}

		try
		{
			CHECK_THROW(false, "Error");
			FAIL();
		}
		catch (const std::exception& exc)
		{
			EXPECT_EQ("Error", std::string(exc.what()));
		}

		try
		{
			CHECK_CUSTOM_THROW(true, TestException("Error"));
		}
		catch (const TestException& exc)
		{
			FAIL();
		}

		try
		{
			CHECK_CUSTOM_THROW(false, TestException("Error"));
			FAIL();
		}
		catch (const TestException& exc)
		{
			EXPECT_EQ("Error", std::string(exc.what()));
		}
	}
	catch (...)
	{
		FAIL();
	}
}

} // SimpleLog