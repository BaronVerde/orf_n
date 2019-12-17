
#include "Logbook.h"
#include <iostream>
#include <chrono>

namespace orf_n {

std::string Logbook::m_logFilename{ "log.txt" };

std::mutex Logbook::m_logfileMutex{};

std::ofstream Logbook::m_logHandle{};

// static
Logbook &Logbook::getInstance() {
	static Logbook onceOnly;
	return onceOnly;
}

void Logbook::logMsg( const logSource type, const logSeverity severity, const std::string &msg ) const {
	if( "" == m_logFilename ) {
		std::string s{ "ERROR: Log filename not set !\n" };
		std::cerr << s;
		throw std::runtime_error( s );
	}
	std::cout << "Logbook: " << msg << std::endl;
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
	std::lock_guard<std::mutex> lock{ m_logfileMutex };
	if( !m_logHandle.is_open() )
		// If file exists, append to it.
		m_logHandle.open( m_logFilename, std::ios::out | std::ios::app );
	m_logHandle << '[' << s << " [" << st << "] " << lt <<  ": " << msg << std::endl;
	m_logHandle.close();
}

void Logbook::logMsg( const std::string &msg ) const {
	logMsg( ANY, LOG, msg );
}

void Logbook::setLogFilename( const std::string &filename ) {
	m_logFilename = filename;
	if( m_logHandle.is_open() )
		m_logHandle.close();
	// If file exists, overwrite it.
	m_logHandle.open( m_logFilename, std::ios::out );
	if( !m_logHandle.is_open() ) {
		std::string s{ "ERROR: Log file could not be opened !\n" };
		std::cerr << s;
		throw std::runtime_error( s );
	}
	std::cout << "Logbook: Log filename set to " + m_logFilename + '\n';
	m_logHandle.close();
}

Logbook::~Logbook() {
	if( m_logHandle.is_open() )
		m_logHandle.close();
}

}
