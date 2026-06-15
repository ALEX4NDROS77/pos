#ifndef UTILS_LOGGER_H
#define	UTILS_LOGGER_H	1

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

class Logger {
	public:
		static Logger& get_instance();

		void set_level(LogLevel level);
		void set_file(const std::string& filepath);

		void debug(const std::string& message);
		void info(const std::string& message);
		void warning(const std::string& message);
		void error(const std::string& message);

		void log(LogLevel level,const std::string& message);

	private:
		Logger();
		~Logger();
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		std::string level_to_string(LogLevel level);
		std::string get_timestamp();

		LogLevel current_level_;
		std::ofstream file_;
		std::mutex mutex_;
		bool console_output_;
};

#define LOG_DEBUG(msg) Logger::get_instance().debug(msg)
#define LOG_INFO(msg) Logger::get_instance().debug(msg)
#define LOG_WARNING(msg) Logger::get_instance().debug(msg)
#define LOG_ERROR(msg) Logger::get_instance().debug(msg)

#endif

