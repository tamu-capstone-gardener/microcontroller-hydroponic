// relay_controller.h
#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

void initPumpN();
void initPumpP();
void initPumpK();
void initLightRelay();

void turnPumpNOn();
void turnPumpNOff();
void turnPumpPOn();
void turnPumpPOff();
void turnPumpKOn();
void turnPumpKOff();

void turnLightOn();
void turnLightOff();

// (If you still have your legacy pump functions, you can keep them or remove as needed.)
#endif
