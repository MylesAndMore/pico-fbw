#ifndef auto_h
#define auto_h

#ifdef WIFLY_ENABLED

#define INTERCEPT_RADIUS 0.5 // The radius at which to consider a waypoint "incercepted"
// TODO: do I need to change this for different speeds? idk if it will make too much of a difference, remember what aviation simmer said

/**
 * Executes one cycle of the auto mode.
*/
void mode_auto();

#endif

/**
 * Fully resets/de-initializes auto mode.
 * This also frees up the second core that it makes use of.
*/
void mode_autoDeinit();

#endif // auto_h
