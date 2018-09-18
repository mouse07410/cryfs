#include "subprocess.h"
#include <cstdio>
#include <stdexcept>
#include <cerrno>

#if defined(__APPLE__)

#include <sys/wait.h>
constexpr const char* openmode = "r";

#elif !defined(_MSC_VER)

#include <sys/wait.h>
constexpr const char* openmode = "re";

#else

#define popen _popen
#define pclose _pclose
#define WEXITSTATUS(a) a
#define WIFEXITED(a) true
constexpr const char* openmode = "r";

#endif

using std::string;

namespace cpputils {
    namespace {
	class SubprocessHandle final {
	public:
		SubprocessHandle(const string &command)
		: _subprocess(popen(command.c_str(), openmode)) {
			if (!_subprocess) {
				throw std::runtime_error("Error starting subprocess " + command + ". Errno: " + std::to_string(errno));
			}
		}

		~SubprocessHandle() {
			if (_subprocess != nullptr) {
				close();
			}
		}

		string getOutput() {
			string output;
			char buffer[1024];
			while (fgets(buffer, sizeof(buffer), _subprocess) != nullptr) {
				output += buffer;
			}
			return output;
		}

		int close() {
			auto returncode = pclose(_subprocess);
			_subprocess = nullptr;
			if (returncode == -1) {
				throw std::runtime_error("Error calling pclose. Errno: " + std::to_string(errno));
			}
			if (!WIFEXITED(returncode)) {
				// WEXITSTATUS is only valid if WIFEXITED is 0.
				throw std::runtime_error("WIFEXITED returned " + std::to_string(WIFEXITED(returncode)));
			}
			return WEXITSTATUS(returncode);
		}

	private:
		FILE *_subprocess;
	};

    }

    SubprocessResult Subprocess::call(const string &command) {
		SubprocessHandle subprocess(command);
        string output = subprocess.getOutput();
        int exitcode = subprocess.close();

        return SubprocessResult {output, exitcode};
    }

    SubprocessResult Subprocess::check_call(const string &command) {
        auto result = call(command);
        if(result.exitcode != 0) {
            throw SubprocessError("Subprocess \""+command+"\" exited with code "+std::to_string(result.exitcode));
        }
        return result;
    }

}
