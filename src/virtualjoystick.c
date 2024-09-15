//
// Created by jonne on 9/15/24.
//

#include "virtualjoystick.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <linux/uinput.h>

// Bits for M8 input messages
typedef enum keycodes_t {
    key_left = 1 << 7,
    key_up = 1 << 6,
    key_down = 1 << 5,
    key_select = 1 << 4,
    key_start = 1 << 3,
    key_right = 1 << 2,
    key_opt = 1 << 1,
    key_edit = 1
} keycodes_t;

static int fd;

int initialize_virtual_joystick() {
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if (fd < 0) {
        perror("open /dev/uinput");
        return 0;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY); // enable button/key handling

    ioctl(fd, UI_SET_KEYBIT, BTN_A);
    ioctl(fd, UI_SET_KEYBIT, BTN_B);
    ioctl(fd, UI_SET_KEYBIT, BTN_START);
    ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_UP);
    ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

    struct uinput_setup setup =
    {
        .name = "M8 Virtual Joystick",
        .id =
        {
            .bustype = BUS_USB,
            .vendor = 0x3,
            .product = 0x3,
            .version = 2,
        }
    };

    if (ioctl(fd, UI_DEV_SETUP, &setup)) {
        perror("UI_DEV_SETUP");
        return 0;
    }

    if (ioctl(fd, UI_DEV_CREATE)) {
        perror("UI_DEV_CREATE");
        return 0;
    }

    fprintf(stderr, "Virtual joystick initialized\n");

    return 1;
}

int destroy_virtual_joystick() {
    if (ioctl(fd, UI_DEV_DESTROY)) {
        printf("UI_DEV_DESTROY");
        return 0;
    }

    close(fd);
    fprintf(stderr, "Virtual joystick destroyed\n");
    return 1;
}

int send_virtual_joystick_message(uint8_t keycode) {
    struct input_event ev[9] = {0};

    ev[0].type = EV_KEY;
    ev[0].code = BTN_DPAD_UP;
    ev[0].value = (keycode & key_up) > 0;

    ev[1].type = EV_KEY;
    ev[1].code = BTN_DPAD_DOWN;
    ev[1].value = (keycode & key_down) > 0;

    ev[2].type = EV_KEY;
    ev[2].code = BTN_DPAD_LEFT;
    ev[2].value = (keycode & key_left) > 0;

    ev[3].type = EV_KEY;
    ev[3].code = BTN_DPAD_RIGHT;
    ev[3].value = (keycode & key_right) > 0;

    ev[4].type = EV_KEY;
    ev[4].code = BTN_A;
    ev[4].value = (keycode & key_edit) > 0;

    ev[5].type = EV_KEY;
    ev[5].code = BTN_B;
    ev[5].value = (keycode & key_opt) > 0;

    ev[6].type = EV_KEY;
    ev[6].code = BTN_START;
    ev[6].value = (keycode & key_start) > 0;

    ev[7].type = EV_KEY;
    ev[7].code = BTN_SELECT;
    ev[7].value = (keycode & key_select) > 0;

    // Sync message
    ev[8].type = EV_SYN;
    ev[8].code = SYN_REPORT;
    ev[8].value = 0;

    if (write(fd, &ev, sizeof ev) < 0) {
        perror("write");
        return 0;
    }

    return 1;
}
