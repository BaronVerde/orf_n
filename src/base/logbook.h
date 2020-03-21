
/**
 * Singleton class for the log file.
 * The logfile will be blocked by mutex as long as it is being written to.
 */

#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace orf_n {

namespace logbook {

typedef enum {
	ANY, SHADER, RESOURCE, SCENE, SCHEDULER, RENDERER, WINDOW, TERRAIN
} log_source;

typedef enum {
	LOG, INFO, WARNING, ERROR
} log_severity;

void log_msg( const log_source type, const log_severity severity, const std::string &msg );

void log_msg( const std::string &msg );

void set_log_filename( const std::string &filename );

}

}
