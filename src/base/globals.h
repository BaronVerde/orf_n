
#pragma once

/**
 * Global definitions
 * @todo un-globalize
 */

namespace orf_n {

namespace globals {

/**
 * Number of ticks between frames in microseconds.
 * See render() method in class Renderer.
 */
extern double delta_time;

/**
 * UI vars shared among components
 */
extern bool show_app_ui;

}

}
