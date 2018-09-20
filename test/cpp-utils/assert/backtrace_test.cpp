#include <gmock/gmock.h>
#include <csignal>
#include "cpp-utils/assert/backtrace.h"
#include "cpp-utils/process/subprocess.h"

using std::string;
using testing::HasSubstr;

namespace {
	std::string call_process_exiting_with(const std::string& kind, const std::string& signal = "") {
#if defined(_MSC_VER)
		constexpr const char* executable = "cpp-utils-test_exit_signal.exe";
#else
		constexpr const char* executable = "./test/cpp-utils/cpp-utils-test_exit_signal";
#endif
		const std::string command = std::string(executable) + " \"" + kind + "\" \"" + signal + "\"  2>&1";
		auto result = cpputils::Subprocess::call(command);
		return result.output;
	}
}

TEST(BacktraceTest, ContainsExecutableName) {
    string backtrace = cpputils::backtrace();
    EXPECT_THAT(backtrace, HasSubstr("cpp-utils-test"));
}

TEST(BacktraceTest, ContainsTopLevelLine) {
    string backtrace = cpputils::backtrace();
    EXPECT_THAT(backtrace, HasSubstr("BacktraceTest"));
    EXPECT_THAT(backtrace, HasSubstr("ContainsTopLevelLine"));
}


namespace {
	std::string call_process_exiting_with_nullptr_violation() {
		return call_process_exiting_with("nullptr");
	}
	std::string call_process_exiting_with_exception(const std::string& message) {
		return call_process_exiting_with("exception", message);
	}
}
#if defined(_MSC_VER)
#include <Windows.h>
namespace {
	std::string call_process_exiting_with_sigsegv() {
		return call_process_exiting_with("signal", std::to_string(EXCEPTION_ACCESS_VIOLATION));
	}
	std::string call_process_exiting_with_sigill() {
		return call_process_exiting_with("signal", std::to_string(EXCEPTION_ILLEGAL_INSTRUCTION));
	}
	std::string call_process_exiting_with_code(DWORD code) {
		return call_process_exiting_with("signal", std::to_string(code));
	}
}
#else
namespace {
	std::string call_process_exiting_with_sigsegv() {
		return call_process_exiting_with("signal", std::to_string(SIGSEGV));
	}
	std::string call_process_exiting_with_sigabrt() {
		return call_process_exiting_with("signal", std::to_string(SIGABRT));
	}
	std::string call_process_exiting_with_sigill() {
		return call_process_exiting_with("signal", std::to_string(SIGILL));
	}
}
#endif

TEST(BacktraceTest, DoesntCrashOnCaughtException) {
	// This is needed to make sure we don't use some kind of vectored exception handler on Windows
	// that ignores the call stack and always jumps on when an exception happens.
	cpputils::showBacktraceOnCrash();
	try {
		throw std::logic_error("exception");
	} catch (const std::logic_error& e) {
		// intentionally empty
	}
}

TEST(BacktraceTest, ShowBacktraceOnNullptrAccess) {
	auto output = call_process_exiting_with_nullptr_violation();
	EXPECT_THAT(output, HasSubstr("cpp-utils-test_exit_signal"));
}

TEST(BacktraceTest, ShowBacktraceOnSigSegv) {
	auto output = call_process_exiting_with_sigsegv();
	EXPECT_THAT(output, HasSubstr("cpp-utils-test_exit_signal"));
}

TEST(BacktraceTest, ShowBacktraceOnUnhandledException) {
	auto output = call_process_exiting_with_exception("my_exception_message");
	EXPECT_THAT(output, HasSubstr("cpp-utils-test_exit_signal"));
}

TEST(BacktraceTest, ShowBacktraceOnSigIll) {
	auto output = call_process_exiting_with_sigill();
	EXPECT_THAT(output, HasSubstr("cpp-utils-test_exit_signal"));
}

#if !defined(_MSC_VER)
TEST(BacktraceTest, ShowBacktraceOnSigAbrt) {
	auto output = call_process_exiting_with_sigabrt();
	EXPECT_THAT(output, HasSubstr("cpp-utils-test_exit_signal"));
}

TEST(BacktraceTest, ShowBacktraceOnSigAbrt_ShowsCorrectSignalName) {
	auto output = call_process_exiting_with_sigabrt();
	EXPECT_THAT(output, HasSubstr("SIGABRT"));
}
#endif

#if !defined(_MSC_VER)
constexpr const char* sigsegv_message = "SIGSEGV";
constexpr const char* sigill_message = "SIGILL";
#else
constexpr const char* sigsegv_message = "EXCEPTION_ACCESS_VIOLATION";
constexpr const char* sigill_message = "EXCEPTION_ILLEGAL_INSTRUCTION";
#endif

TEST(BacktraceTest, ShowBacktraceOnSigSegv_ShowsCorrectSignalName) {
	auto output = call_process_exiting_with_sigsegv();
	EXPECT_THAT(output, HasSubstr(sigsegv_message));
}

TEST(BacktraceTest, ShowBacktraceOnSigIll_ShowsCorrectSignalName) {
	auto output = call_process_exiting_with_sigill();
	EXPECT_THAT(output, HasSubstr(sigill_message));
}

#if !defined(_MSC_VER)
TEST(BacktraceTest, ShowBacktraceOnUnhandledException_ShowsCorrectExceptionMessage) {
	auto output = call_process_exiting_with_exception("my_exception_message");
	EXPECT_THAT(output, HasSubstr("my_exception_message"));
}
#endif

#if defined(_MSC_VER)
TEST(BacktraceTest, UnknownCode_ShowsCorrectSignalName) {
	auto output = call_process_exiting_with_code(0x1234567);
	EXPECT_THAT(output, HasSubstr("UNKNOWN_CODE(0x1234567)"));
}
#endif
