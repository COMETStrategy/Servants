#include "../Services/Logger.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace comet;

TEST(LoggerTest, LogLevelToString) {
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::INFO), "INFO");
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::DEBUG), "DEBUG");
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::WARNING), "WARNING");
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::CRITICAL), "CRITICAL");
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::NONE), "NONE");
        EXPECT_EQ(LogLevelToString(static_cast<comet::LoggerLevel>(999)), "UNKNOWN");
}

TEST(LoggerTest, FormatTime) {
        std::time_t testTime = 1736200000; // Example timestamp
        std::string formattedTime = formatTime(testTime);
        EXPECT_EQ(formattedTime, "2025-01-06 21:46:40");
}

TEST(LoggerTest, LogMessage) {
        comet::Logger::setLoggerLevel(comet::LoggerLevel::INFO);

        std::ostringstream output;
        std::streambuf* oldCoutBuf = std::cout.rdbuf(output.rdbuf());

        comet::Logger::log("Test message", comet::LoggerLevel::INFO);

        std::cout.rdbuf(oldCoutBuf); // Restore original buffer

        std::string logOutput = output.str();
        EXPECT_NE(logOutput.find("Test message"), std::string::npos);
        EXPECT_NE(logOutput.find("INFO"), std::string::npos);
}

TEST(LoggerTest, SetLoggerLevel) {
        comet::Logger::setLoggerLevel(comet::LoggerLevel::DEBUG);
        EXPECT_EQ(LogLevelToString(comet::LoggerLevel::DEBUG), "DEBUG");
}

TEST(LoggerTest, SetFileName) {
        comet::Logger::setFileName("test.log");
        EXPECT_EQ(comet::Logger::getFileName()  , "test.log");
}