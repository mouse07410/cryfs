from conans import ConanFile, CMake

class CryFSConan(ConanFile):
	settings = "os", "build_type", "arch"
	requires = [
		"range-v3/0.9.1@ericniebler/stable",
		"spdlog/1.4.2",
	]
	generators = "cmake"
	
