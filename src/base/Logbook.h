
/**
 * Singleton class for the log file.
 * The logfile will be blocked by mutex as long as it is being written to.
 */

#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace orf_n {

class Logbook {
public:
	typedef enum {
		ANY, SHADER, RESOURCE, SCENE, SCHEDULER, RENDERER, WINDOW, TERRAIN
	} logSource;

	typedef enum {
		LOG, INFO, WARNING, ERROR
	} logSeverity;

	static Logbook &getInstance();

	void logMsg( const logSource type, const logSeverity severity, const std::string &msg ) const;

	void logMsg( const std::string &msg ) const;

	void setLogFilename( const std::string &filename );

private:
	// lock handle for the file
	static std::mutex m_logfileMutex;

	static std::string m_logFilename;

	static std::ofstream m_logHandle;

	Logbook() = default;

	Logbook( const Logbook & ) = delete;

	void operator=( const Logbook & ) = delete;

	virtual ~Logbook();

};

}	// namespace
