using namespace vex;

extern brain Brain;

// VEXcode devices
extern motor leftback;
extern motor rightfront;
extern motor rightback;
extern motor leftfront;
extern controller Controller1;
extern motor rightintake;
extern motor leftintake;
extern motor scooperBottom;
extern motor scooperTop;
extern encoder vertencoder;
extern encoder strafeencoder;
extern inertial inertia;
extern bumper bumper1;
extern bumper bumper2;
extern line lineTracker;
extern triport Expander15;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );