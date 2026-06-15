#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <utils/Logger.h>

Logger& Logger::get_instance() {
	static Logger instance;
	return instance;
}

Logger::Logger() : current_level_(LogLevel::INFO), console_output_(true) {
}

Logger::~Logger() {
	if(file_.is_open()) {
		file_.close();
	}
}

void Logger::set_level(LogLevel level) {
	current_level_ = level;
}

void Logger::set_file(const std::string& filepath) {
	std::lock_guard<std::mutex> lock(mutex_);
	if(file_.is_open()) {
		file_.close();
	}
	file_.open(filepath,std::ios::app);
	if(!file_.is_open()) {
		std::cerr << "[Logger] Failed to open log file: " << filepath << std::endl;
	}
}

std::string Logger::get_timestamp() {
	auto now = std::time(nullptr);
	std::ostringstream oss;
	oss << std::put_time(std::localtime(&now),"%Y-%m-%d %H:%M:%S");
	return oss.str();
}

std::string Logger::level_to_string(LogLevel level) {
	switch(level) {
		case LogLevel::DEBUG:	return "DEBUG";
		case LogLevel::INFO:	return "INFO";
		case LogLevel::WARNING:	return "WARNING";
		case LogLevel::ERROR:	return "ERROR";
		default:		return "UNKNOWN";
	}
}

void Logger::log(LogLevel level,const std::string& message) {
	if(level < current_level_) {
		return;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	std::string timestamp = get_timestamp();
	std::string level_str = level_to_string(level);
	std::string formatted = "[" + timestamp + "] [" + level_str + "] " + message;

	if(console_output_) {
		if(level == LogLevel::ERROR) {
			std::cerr << formatted << std::endl;
		} else {
			std::cout << formatted << std::endl;
		}
	}

	if(file_.is_open()) {
		file_ << formatted << std::endl;
		file_.flush();
	}
}

void Logger::debug(const std::string& message) {
	log(LogLevel::DEBUG,message);
}

void Logger::info(const std::string& message) {
	log(LogLevel::INFO,message);
}

void Logger::warning(const std::string& message) {
	log(LogLevel::WARNING,message);
}

void Logger::error(const std::string& message) {
	log(LogLevel::ERROR,message);
}

