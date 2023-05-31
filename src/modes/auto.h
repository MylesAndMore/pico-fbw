#ifndef auto_h
#define auto_h

/**
 * Executes one cycle of the auto mode.
*/
void mode_auto();

/**
 * Fully resets/de-initializes auto mode.
 * This also frees up the second core that it makes use of.
*/
void mode_autoDeinit();

#endif // auto_h
