
#include "base/globals.h"
#include "base/logbook.h"
#include "renderer/renderer.h"
#include "glad/glad.h"
#include <iostream>
#include <cstring>	// strcmp()
//#include "../utils/asc2png.h"

using namespace orf_n;

double globals::delta_time = 0.0;
bool globals::show_app_ui = false;

int main( int argc, char *argv[] ) {
	//return converter::asc2png( "/home/kemde/Downloads/OpenGL/srtm/big/cut_n30e000.asc", 2048 ) ? EXIT_SUCCESS : EXIT_FAILURE;

	logbook::set_log_filename( "orfnlog.log" );
	logbook::log_msg( "Program started." );

	bool debug{ true };
	if( argc > 1 ) {
		if( 0 == strcmp( argv[1], "-d" ) || 0 == strcmp( argv[1], "--debug" ) )
			debug = true;
		else
			std::cout << "Unknown parameter: " << argv[1] << "\n";
	}

	try {
		renderer* r = new renderer( debug );
		r->setupRenderer();
		r->setup();
		r->render();
		r->cleanup();
		r->cleanupRenderer();
		delete r;
	} catch( std::runtime_error &e ) {
		std::cerr << e.what() << std::endl;
	}

	logbook::log_msg( "Program ending normally." );
	return EXIT_SUCCESS;

}
