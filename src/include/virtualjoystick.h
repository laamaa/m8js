//
// Created by jonne on 9/15/24.
//

#ifndef VIRTUALJOYSTICK_H
#define VIRTUALJOYSTICK_H
#include <stdint.h>

int initialize_virtual_joystick();
int destroy_virtual_joystick();
int send_virtual_joystick_message(uint8_t keycode);

#endif //VIRTUALJOYSTICK_H
