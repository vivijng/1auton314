/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       VEX                                                       */
/*    Created:      Thu Sep 26 2019                                           */
/*    Description:  Competition Template                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// leftback             motor         4               
// rightfront           motor         8               
// rightback            motor         7               
// leftfront            motor         1               
// Controller1          controller                    
// rightintake          motor         6               
// leftintake           motor         11              
// scooperBottom        motor         10              
// scooperTop           motor         2               
// vertencoder          encoder       C, D            
// strafeencoder        encoder       E, F            
// inertia              inertial      3               
// bumper1              bumper        C               
// bumper2              bumper        B               
// lineTracker          line          A               
// Expander15           triport       15              
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;

competition Competition;

//positiont tracking
int xPos = 0;
int yPos = 0;

//debug variables
int yG = 0;
int xG = 0;
int xposG = 0;
int yposG = 0;
int debugX = 0;
int debugY = 0;

float threshold = 19;

bool moving = false;


//Change these to put in waypoints for the robot
const int positions[6][2] = 
{
  //1 inch = 40 lukes
  //first ball
  {0,400},
  //first tower
  {730,400},
  //second ball
  {490, 2130},
  //third ball
  {360, 3580},
  //third goal
  {920, 4800},
};

int rotatedPoints[6][2]; //keeps track of the points once the robot has to make a rotation

//For debugging the different variables
void display() {
  while(true) {
    /*Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Vertencoder: %f", vertencoder.position(degrees));

    Brain.Screen.setCursor(2, 1);
    Brain.Screen.print("Strafeencoder: %f", strafeencoder.position(degrees));

    Brain.Screen.setCursor(3, 1);
    Brain.Screen.print("Is moving: %f", moving);

    Brain.Screen.setCursor(4, 1);
    Brain.Screen.print("Next Y Position: %d", yG);

    Brain.Screen.setCursor(5, 1);
    Brain.Screen.print("Next X Position: %d", xG);

    Brain.Screen.setCursor(6, 1);
    Brain.Screen.print("X Position: %d", xposG);

    Brain.Screen.setCursor(7, 1);
    Brain.Screen.print("Y Position: %d", yposG);

    Brain.Screen.setCursor(8, 1);
    Brain.Screen.print("X math value: %d", debugX);

    Brain.Screen.setCursor(9, 1);
    Brain.Screen.print("Y math value: %d", debugY);

    Brain.Screen.setCursor(10, 1);
    Brain.Screen.print("Reflectivity: %d", lineTracker.reflectivity());*/

    Brain.Screen.setCursor(10, 1);
    Brain.Screen.print("Intertia Heading: %d", inertia.heading(degrees));

    wait(15, msec);
    Brain.Screen.clearScreen();

  }
}

//Iterates though every index in the rotatedPoints array and rotates the point by the amount that the robot is turning
void rotate(int degree, int cx, int cy) {
  int arraySize = sizeof(rotatedPoints) / sizeof(rotatedPoints[0]);

  double degrees = ((degree) * (3.145926/180)); //converting the degrees into radians so that sin and cos works

  for(int i=0; i<arraySize; i++) {
      int x = rotatedPoints[i][0];
      int y = rotatedPoints[i][1];

      //rotating the point in the array around the current x y position of the robot (Math found online)
      rotatedPoints[i][0] = round(cos(degrees) * (x - cx) - sin(degrees) * (y - cy) + cx);
      rotatedPoints[i][1] = round(sin(degrees) * (x - cx) + cos(degrees) * (y - cy) + cy);
    }
}

//copies all of the x y positions from the positions array to the rotatedPoints array so that the user doesn't have to do that manually
void initRotations() {
  int arraySize = sizeof(positions) / sizeof(positions[0]);

  for(int i=0; i<arraySize; i++){
    rotatedPoints[i][0] = positions[i][0];
    rotatedPoints[i][1] = positions[i][1];
  }
}

//method to quickly calculate distance between two points
float distanceXY(int x, int y, int x2, int y2) {
  float dx = x - x2;
  float dy = y - y2;

  return sqrt(dx*dx + dy*dy);
}


//For ramping up and ramping down
float calculateSpeed(float distanceToFinish, float endGoal) {
  float rampUpK = .1;
  float rampDownK = .1;

  float halfWay = endGoal / 2;

  float perc = 0;

  if(distanceToFinish < halfWay){
    //ramp up
    float left = endGoal - distanceToFinish;
    perc = endGoal / left+1;
    perc *= perc;
    perc *= rampUpK;
  }
  else {
    //ramp down
    perc = endGoal / distanceToFinish+1;
    perc *= perc;
    perc *= rampDownK;
  }

  return perc;
}


//Move to: takes in a point index number and a speed value and travels to that point based off of its current x y position
void moveTo(int posIndex, float speed, bool usePSpeed) {
  //resetting the encoders
  vertencoder.setPosition(0, degrees);
  strafeencoder.setPosition(0, degrees);

  //getting the points that the user wanted
  int x = rotatedPoints[posIndex][0];
  int y = rotatedPoints[posIndex][1];

  xG = x;
  yG = y;

  moving = true; //for debugging... not really useful in any way. Feel free to delete
  
  //Remembering where the robot started for calculations
  int startPosX = xPos;
  int startPosY = yPos;

  float originalSpeed = speed; //saving the original speed so that the robot can manipulate it's speed without going too fast or too slow

  //storing the x y positions in new variables so that the code doesn't use the main variables
  int currentX = xPos; 
  int currentY = yPos;

  //moving until the distance between the x and y points are less than 10 plus the speed divided by 10. This value can be lowered, but the robot might overshoot and wobble a bit. It can also be raised, but the robot will stop moving further from the point but with less chance of wobbling
  while(!(abs(x-currentX) < 10 + (originalSpeed/10) && abs(y-currentY) < 10 + (originalSpeed/10))) {
    //Creating a 2D vector for the robot to travel on to get to the assigned point
    double xValue = (x - currentX);
    double yValue = (y - currentY);

    debugX = x;
    debugY = y;

    //Normalizing the vector so that the code can have full control over the robot's speed 
    float length = sqrt(xValue * xValue + yValue * yValue);

    xValue /= length;
    yValue /= length;
    
    //Creating the motor speeds. (replacing x and y controller input with xValue and yValue since they mimic controller input)
    double frontLeft = (double)((yValue + xValue));
    double backLeft = (double)((yValue - xValue));
    double frontRight = (double)((yValue - xValue));
    double backRight = (double)((yValue + xValue));

    //calculating the ramp up and down speeds
    float currentProgress = distanceXY(currentX, currentY, startPosX, startPosY);
    float totalProgress = distanceXY(startPosX, startPosY, x, y);

    speed = originalSpeed;
    if(usePSpeed)
      speed *= calculateSpeed(currentProgress, totalProgress);

    //applying motor speeds to motors and spinning
    leftfront.setVelocity(frontLeft * speed, vex::velocityUnits::pct);
    leftback.setVelocity(backLeft * speed, vex::velocityUnits::pct);
    rightfront.setVelocity(frontRight * speed, vex::velocityUnits::pct);
    rightback.setVelocity(backRight * speed, vex::velocityUnits::pct);
    
    leftfront.spin(forward);
    leftback.spin(forward);
    rightfront.spin(forward);
    rightback.spin(forward);

    //updating the positions so that the robot knows how far away it is from the assigned point
    currentX = vertencoder.position(degrees) + startPosX;
    currentY = strafeencoder.position(degrees) + startPosY;

    //for debugging
    xposG = xPos;
    yposG = yPos;
  }

  //setting the x y position of the robot to the points it just traveled to
  xPos = rotatedPoints[posIndex][0];
  yPos = rotatedPoints[posIndex][1];

  //updatting the debugging variables one last time
  xposG = xPos;
  yposG = yPos;

  //stopping the motors
  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();
  moving = false; //for debugging
}

void forwardsdrive(int distancetotravel)
{

  leftfront.setVelocity(40, vex::velocityUnits::pct);
  leftback.setVelocity(40, vex::velocityUnits::pct);
  rightfront.setVelocity(40, vex::velocityUnits::pct);
  rightback.setVelocity(40, vex::velocityUnits::pct);

  strafeencoder.setPosition(0,degrees);
  while(strafeencoder.position(degrees) < distancetotravel)
  {
    leftfront.spin(forward);
    leftback.spin(forward);
    rightfront.spin(forward);
    rightback.spin(forward);
  }

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();
}

void backwardsdrive(int distancetotravel)
{

  leftfront.setVelocity(40, vex::velocityUnits::pct);
  leftback.setVelocity(40, vex::velocityUnits::pct);
  rightfront.setVelocity(40, vex::velocityUnits::pct);
  rightback.setVelocity(40, vex::velocityUnits::pct);
  
  strafeencoder.setPosition(0,degrees);
  while(strafeencoder.position(degrees)>distancetotravel)
  {
    leftfront.spin(reverse);
    leftback.spin(reverse);
    rightfront.spin(reverse);
    rightback.spin(reverse);
  }
  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();
}

void leftstrafe(int distancetotravel)
{
  vertencoder.setPosition(0,degrees);

  leftfront.setVelocity(40, vex::velocityUnits::pct);
  leftback.setVelocity(40, vex::velocityUnits::pct);
  rightfront.setVelocity(40, vex::velocityUnits::pct);
  rightback.setVelocity(40, vex::velocityUnits::pct);

  vertencoder.setPosition(0,degrees);
  while(vertencoder.position(degrees) > distancetotravel)
  {
    leftfront.spin(reverse);
    leftback.spin(forward);
    rightfront.spin(forward);
    rightback.spin(reverse);
  }

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();
}

void rightstrafe(int distancetotravel)
{
  vertencoder.setPosition(0,degrees);

  leftfront.setVelocity(40, vex::velocityUnits::pct);
  leftback.setVelocity(40, vex::velocityUnits::pct);
  rightfront.setVelocity(40, vex::velocityUnits::pct);
  rightback.setVelocity(40, vex::velocityUnits::pct);
  
  vertencoder.setPosition(0,degrees);
  while(vertencoder.position(degrees) < distancetotravel)
  {
    leftfront.spin(forward);
    leftback.spin(reverse);
    rightfront.spin(reverse);
    rightback.spin(forward);
  }
  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();
}

void rightinertialturn(double goaldegrees)
{

  inertia.resetRotation();

  //using the calculateSpeed method to ramp up and down the turn speed
  while(inertia.rotation(degrees) < goaldegrees)
  {
    float speed = calculateSpeed(inertia.rotation(degrees), goaldegrees);
    speed *= 40;

    leftfront.setVelocity(speed, vex::velocityUnits::pct);
    leftback.setVelocity(speed, vex::velocityUnits::pct);
    rightfront.setVelocity(-speed, vex::velocityUnits::pct);
    rightback.setVelocity(-speed, vex::velocityUnits::pct);
  
    leftfront.spin(forward);
    leftback.spin(forward);
    rightfront.spin(forward);
    rightback.spin(forward);
  }

  rotate(goaldegrees, xPos, yPos); //position tracking

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();

  wait (1,seconds);
}

void leftinertialturn(double goaldegrees)
{
  goaldegrees *= -1;

  inertia.resetRotation();

  //using the calculateSpeed method to ramp up and down the turn speed
  while(inertia.rotation(degrees) > goaldegrees)
  {
    float speed = calculateSpeed(-inertia.rotation(degrees), -goaldegrees);
    speed *= 40;

    leftfront.setVelocity(-speed, vex::velocityUnits::pct);
    leftback.setVelocity(-speed, vex::velocityUnits::pct);
    rightfront.setVelocity(speed, vex::velocityUnits::pct);
    rightback.setVelocity(speed, vex::velocityUnits::pct);
  
    leftfront.spin(forward);
    leftback.spin(forward);
    rightfront.spin(forward);
    rightback.spin(forward);
  }

  rotate(goaldegrees, xPos, yPos); //position tracking

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();

  wait (1,seconds);
}

void intake(){
  leftintake.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
  rightintake.spin(vex::directionType::rev,100,vex::velocityUnits::pct);

}

void outtake(){
  leftintake.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
  rightintake.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
}

void shoot(){
  scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
}

void revShoot(){
  scooperTop.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
}

void lightRevShoot(){
  while(true)
  {
    if(lineTracker.reflectivity() > threshold)
    {
      scooperTop.spin(vex::directionType::rev,10,vex::velocityUnits::pct);
      scooperBottom.spin(vex::directionType::fwd,10,vex::velocityUnits::pct);
    }
    else{
      scooperTop.stop();
      scooperBottom.stop();
      break;
    }
  }
}

void pscoop(){
  scooperTop.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
}

void stopintake(){
  leftintake.stop();
  rightintake.stop();
}

void stopscoop(){
  scooperTop.stop();
  scooperBottom.stop();
}

void bumperShoot()
{
  leftfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  leftback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);

  waitUntil(bumper1.pressing() || bumper2.pressing());

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();

  scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);

  wait(.5,seconds);

  scooperTop.stop();
  scooperBottom.stop();
}

void bumperShoot2()
{
  leftfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  leftback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);

  waitUntil(bumper1.pressing() || bumper2.pressing());

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();

  scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);

  wait(.35,seconds);

  scooperTop.stop();
  scooperBottom.stop();
}

void bumperShoot3()
{
  leftfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  leftback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightfront.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);
  rightback.spin(vex::directionType::fwd,40,vex::velocityUnits::pct);

  waitUntil(bumper1.pressing() || bumper2.pressing());

  leftfront.stop();
  leftback.stop();
  rightfront.stop();
  rightback.stop();

  scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
  scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);

  wait(.65,seconds);

  scooperTop.stop();
  scooperBottom.stop();
}

void auton1()
{
  initRotations();

  //deploy and get one ball
  forwardsdrive(10);
  backwardsdrive(-10);
  intake();
  moveTo(0 , 40, true);
  rightinertialturn(135);
  stopintake();

  //score first ball
  moveTo(1, 40, false);
  bumperShoot();
  backwardsdrive(-300);
  stopscoop();
  rightinertialturn(218); //this is iffy
  intake();

  //get second ball
  moveTo(2, 40, true);
  rightinertialturn(88);
  stopintake();
  bumperShoot2();

  //third goal
  backwardsdrive(-280);
  leftinertialturn(68);
  bumperShoot3();

}
void pre_auton(void) {
  vexcodeInit();

  inertia.calibrate();
  while (inertia.isCalibrating()) 
  {
    wait(.3, seconds);
  }

  Controller1.rumble(rumbleShort);

}

void autonomous(void) {

  thread t(display);

  initRotations();

  //deploy and get one ball
  forwardsdrive(10);
  backwardsdrive(-10);
  intake();
  moveTo(0 , 40, true);
  rightinertialturn(135);
  stopintake();

  //score first ball
  moveTo(1, 40, false);
  bumperShoot();
  backwardsdrive(-300);
  stopscoop();
  rightinertialturn(215); //this is iffy
  intake();

  //get second ball
  moveTo(2, 40, true);
  leftstrafe(-400);

  //score second ball
  rightinertialturn(90);
  stopintake();
  bumperShoot2();

  //get third ball
  backwardsdrive(-340);
  leftinertialturn(85);
  intake();
  moveTo(3, 40, true);

  //score third ball
  rightinertialturn(24);
  stopintake();
  bumperShoot3();
  
}

void usercontrol(void) {
  while (true)
  {
    thread t(display);
    while (true)
  {
    double frontLeft = (double)(Controller1.Axis3.position(pct) + Controller1.Axis4.position(pct) + (Controller1.Axis1.position(pct)) * .5);
    double backLeft = (double)(Controller1.Axis3.position(pct) - Controller1.Axis4.position(pct) + (Controller1.Axis1.position(pct)) * .5);
    double frontRight = (double)(Controller1.Axis3.position(pct) - Controller1.Axis4.position(pct) - (Controller1.Axis1.position(pct)) * .5);
    double backRight = (double)(Controller1.Axis3.position(pct) + Controller1.Axis4.position(pct) - (Controller1.Axis1.position(pct)) * .5);

    leftfront.spin(fwd, frontLeft * .8, velocityUnits::pct);
    leftback.spin(fwd, backLeft * .8, velocityUnits::pct);
    rightfront.spin(fwd, frontRight* .8,velocityUnits::pct);
    rightback.spin(fwd, backRight * .8, velocityUnits::pct);
  
    if(Controller1.ButtonL1.pressing())
    {
      scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
      scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
    }
    else if (Controller1.ButtonL2.pressing())
    {
      scooperTop.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
      scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
    }
    else
    {
      scooperTop.stop(vex::brakeType::hold);
      scooperBottom.stop(vex::brakeType::hold);
    }
    if(Controller1.ButtonR1.pressing())
    {
      leftintake.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
      rightintake.spin(vex::directionType::rev,100,vex::velocityUnits::pct);
    }
    else if (Controller1.ButtonR2.pressing())
    {
      leftintake.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
      rightintake.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
    }
    else
    {
      leftintake.stop(vex::brakeType::coast);
      rightintake.stop(vex::brakeType::coast);
    }
    /*if(Controller1.ButtonA.pressing())
    {
      scooperTop.spin(vex::directionType::fwd,100,vex::velocityUnits::pct);
      scooperBottom.spin(vex::directionType::rev,100,vex::velocityUnits::pct);

      wait(.65,seconds);

      scooperTop.stop();
      scooperBottom.stop();
    }*/
    wait(20, msec);
   }
  }
}

int main() {
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  pre_auton();

  while (true) {
    wait(100, msec);
  }
}
