//
// Created by jonne on 9/15/24.
//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/uinput.h>

#include "virtualjoystick.h"
#include "include/command.h"
#include "include/serial.h"
#include "include/slip.h"

enum application_state { ERROR, QUIT, RUN };

enum application_state state = QUIT;

int empty_packet_counter = 0;

// Handles CTRL+C / SIGINT
void intHandler() { state = QUIT; }

int main(const int argc, char *argv[]) {

    // allocate memory for serial buffer
    uint8_t *serial_buf = calloc(serial_read_size, sizeof(uint8_t));
    static uint8_t slip_buffer[serial_read_size] = {0};

    // settings for the slip packet handler
    static const slip_descriptor_s slip_descriptor = {
        .buf = slip_buffer,
        .buf_size = sizeof(slip_buffer),
        .recv_message = process_command, // the function where complete slip
        // packets are processed further
    };

    static slip_handler_s slip;

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);
#ifdef SIGQUIT
    signal(SIGQUIT, intHandler);
#endif
    slip_init(&slip, &slip_descriptor);

    if (initialize_serial(1, NULL) && enable_and_reset_display() && initialize_virtual_joystick()) {
        state = RUN;
    } else {
        state = ERROR;
    }

    while (state == RUN) {
        while (1) {
            empty_packet_counter = 0;
            // read serial port
            const int bytes_read = serial_read(serial_buf, serial_read_size);
            if (bytes_read < 0) {
                fprintf(stderr, "Error %d reading serial.", bytes_read);
                state = QUIT;
                break;
            }
            if (bytes_read > 0) {
                // input from device: reset the zero byte counter and create a
                // pointer to the serial buffer
                const uint8_t *cur = serial_buf;
                const uint8_t *end = serial_buf + bytes_read;
                while (cur < end) {
                    // process the incoming bytes into commands and draw them
                    const int n = slip_read_byte(&slip, *cur++);
                    if (n != SLIP_NO_ERROR) {
                        if (n == SLIP_ERROR_INVALID_PACKET) {
                            reset_display();
                        } else {
                            fprintf(stderr, "SLIP error %d\n", n);
                        }
                    }
                }
            } else {
                // zero byte packet, increment counter
                empty_packet_counter++;
                if (empty_packet_counter > 512) {
                    empty_packet_counter = 0;

                    // try opening the serial port to check if it's alive
                    if (check_serial_port()) {
                        // the device is still there, carry on
                        break;
                    }
                    state = ERROR;
                    disconnect();
                    /* we'll make one more loop to see if the device is still there
                     * but just sending zero bytes. if it doesn't get detected when
                     * resetting the port, it will disconnect */
                }
                break;
            }
        }
        usleep(1000);
    }

    free(serial_buf);
    destroy_virtual_joystick();
    if (state == ERROR) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
