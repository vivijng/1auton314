#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
triport Expander15 = triport(PORT15);
motor leftback = motor(PORT4, ratio18_1, false);
motor rightfront = motor(PORT8, ratio18_1, true);
motor rightback = motor(PORT7, ratio18_1, true);
motor leftfront = motor(PORT1, ratio18_1, false);
controller Controller1 = controller(primary);
motor rightintake = motor(PORT6, ratio18_1, false);
motor leftintake = motor(PORT11, ratio18_1, true);
motor scooperBottom = motor(PORT10, ratio18_1, false);
motor scooperTop = motor(PORT2, ratio18_1, false);
encoder vertencoder = encoder(Brain.ThreeWirePort.C);
encoder strafeencoder = encoder(Brain.ThreeWirePort.E);
inertial inertia = inertial(PORT3);
bumper bumper1 = bumper(Expander15.C);
bumper bumper2 = bumper(Expander15.B);
line lineTracker = line(Expander15.A);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}