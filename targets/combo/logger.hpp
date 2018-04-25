#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#define ENABLE_DEBUG

class Logger {
public:
	static void debug(const std::string &str) {
		#ifdef ENABLE_DEBUG
		std::cout << str << "\n";
		#else
		(void)str;
		#endif
	}
};

#endif
