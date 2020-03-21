
#include "logbook.h"
#include <iostream>
#include <chrono>

namespace orf_n {

namespace logbook {

std::string log_filename{ "log.txt" };

std::mutex logfile_mutex{};

std::ofstream log_handle{};

void log_msg( const log_source type, const log_severity severity, const std::string &msg ) {
	if( "" == log_filename ) {
		std::string s{ "ERROR: Log filename not set !\n" };
		std::cerr << s;
		throw std::runtime_error( s );
	}
	std::cout << "logbook: " << msg << std::endl;
	std::string lt, st;
	switch( type ) {
		case ANY		: lt = "ANY"; break;
		case SHADER		: lt = "SHADER"; break;
		case RESOURCE	: lt = "RESOURCE"; break;
		case SCENE		: lt = "SCENE"; break;
		case SCHEDULER	: lt = "SCHEDULER"; break;
		case RENDERER	: lt = "RENDERER"; break;
		case WINDOW		: lt = "WINDOW"; break;
		case TERRAIN	: lt = "TERRAIN"; break;
		default			: lt = "UNKNOWN"; break;
	};
	switch( severity ) {
		case LOG		: st = "LOG"; break;
		case INFO		: st = "INF"; break;
		case WARNING	: st = "WRN"; break;
		case ERROR		: st = "ERR"; break;
		default			: st = "UNK"; break;
	};
	using std::chrono::system_clock;
	system_clock::time_point n = system_clock::now();
	std::time_t t = system_clock::to_time_t( n );
	std::string s = ctime( &t );
	// delete trailing newline
	s.at( s.size()-1 ) = ']';
	std::lock_guard<std::mutex> lock{ logfile_mutex };
	if( !log_handle.is_open() )
		// If file exists, append to it.
		log_handle.open( log_filename, std::ios::out | std::ios::app );
	log_handle << '[' << s << " [" << st << "] " << lt <<  ": " << msg << std::endl;
	log_handle.close();
}

void log_msg( const std::string &msg ) {
	log_msg( ANY, LOG, msg );
}

void set_log_filename( const std::string &filename ) {
	log_filename = filename;
	if( log_handle.is_open() )
		log_handle.close();
	// If file exists, overwrite it.
	log_handle.open( log_filename, std::ios::out );
	if( !log_handle.is_open() ) {
		std::string s{ "ERROR: Log file could not be opened !\n" };
		std::cerr << s;
		throw std::runtime_error( s );
	}
	std::cout << "logbook: Log filename set to " + log_filename + '\n';
	log_handle.close();
}

}

}
