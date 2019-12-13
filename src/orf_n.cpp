
#include <base/Globals.h>
#include <base/Logbook.h>
#include <iostream>
#include <curl/curl.h>
#include <renderer/Renderer.h>
#include <memory>	// unique_ptr
#include <cstring>	// strcmp()

#include "../utils/asc2png.h"

using namespace orf_n;

extern double globals::deltaTime = 0.0;

int main( int argc, char *argv[] ) {

	//return converter::asc2png( "/home/kemde/Downloads/OpenGL/srtm/big/cut_n30e000.asc", 2048 ) ? EXIT_SUCCESS : EXIT_FAILURE;

	Logbook::getInstance().setLogFilename( "orfnlog.log" );
	Logbook::getInstance().logMsg( "Program started." );

	if( 0 != curl_global_init( /*CURL_GLOBAL_WIN32 | CURL_GLOBAL_ACK_EINTR |*/ CURL_GLOBAL_SSL ) )
		std::cerr << "Error initializing curl." << std::endl;

	bool debug{ true };
	if( argc > 1 ) {
		if( 0 == std::strcmp( argv[1], "-d" ) || 0 == std::strcmp( argv[1], "--debug" ) )
			debug = true;
		else
			std::cout << "Unknown parameter: " << argv[1] << "\n";
	}

	try {
		std::unique_ptr<Renderer> r{ std::make_unique<Renderer>( debug ) };
		r->setupRenderer();
		r->setup();
		r->render();
		r->cleanup();
		r->cleanupRenderer();
	} catch( std::runtime_error &e ) {
		std::cerr << e.what() << std::endl;
	}

	curl_global_cleanup();

	Logbook::getInstance().logMsg( "Program ending normally." );
	return EXIT_SUCCESS;

}
