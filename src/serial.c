// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

// Contains portions of code from libserialport's examples released to the
// public domain

#include <libserialport.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/serial.h"

struct sp_port *m8_port = NULL;

// Helper function for error handling
static int check(enum sp_return result);

/**
 * Detects if a given serial port corresponds to an M8 USB serial device.
 *
 * @param m8_port The serial port to be checked.
 * @return Returns 1 if the port is an M8 USB serial device, otherwise returns 0.
 */
static int detect_m8_serial_device(const struct sp_port *m8_port) {
    // Check the connection method - we want USB serial devices
    const enum sp_transport transport = sp_get_port_transport(m8_port);

    if (transport == SP_TRANSPORT_USB) {
        // Get the USB vendor and product IDs.
        int usb_vid, usb_pid;
        sp_get_port_usb_vid_pid(m8_port, &usb_vid, &usb_pid);

        if (usb_vid == 0x16C0 && usb_pid == 0x048A)
            return 1;
    }

    return 0;
}

/**
 * Checks the serial ports to determine if the connected M8 USB serial device is present.
 *
 * @return Returns 1 if the M8 USB serial device is found on the specified port, otherwise returns 0.
 */
int check_serial_port() {
    int device_found = 0;

    /* A pointer to a null-terminated array of pointers to
     * struct sp_port, which will contain the ports found.*/
    struct sp_port **port_list;

    /* Call sp_list_ports() to get the ports. The port_list
     * pointer will be updated to refer to the array created. */
    const enum sp_return result = sp_list_ports(&port_list);

    if (result != SP_OK) {
        fprintf(stderr, "sp_list_ports() failed!\n");
        abort();
    }

    /* Iterate through the ports. When port_list[i] is NULL
     * this indicates the end of the list. */
    for (int i = 0; port_list[i] != NULL; i++) {
        const struct sp_port *port = port_list[i];

        if (detect_m8_serial_device(port)) {
            if (strcmp(sp_get_port_name(port), sp_get_port_name(m8_port)) == 0)
                device_found = 1;
        }
    }

    sp_free_port_list(port_list);
    return device_found;
}

/**
 * Initializes the serial connection by searching for M8 USB serial devices and configuring the port.
 *
 * @param verbose If non-zero, additional debug information will be printed to stderr.
 * @param preferred_device A string representing the preferred device name; if found, iteration stops early.
 * @return Returns 1 if the serial port initialization is successful; otherwise returns 0.
 */
int initialize_serial(const int verbose, const char *preferred_device) {
    if (m8_port != NULL) {
        // Port is already initialized
        return 1;
    }
    /* A pointer to a null-terminated array of pointers to
     * struct sp_port, which will contain the ports found.*/
    struct sp_port **port_list;

    if (verbose)
        fprintf(stderr, "Looking for USB serial devices.\n");

    /* Call sp_list_ports() to get the ports. The port_list
     * pointer will be updated to refer to the array created. */
    enum sp_return result = sp_list_ports(&port_list);

    if (result != SP_OK) {
        fprintf(stderr, "sp_list_ports() failed!\n");
        abort();
    }

    /* Iterate through the ports. When port_list[i] is NULL
     * this indicates the end of the list. */
    for (int i = 0; port_list[i] != NULL; i++) {
        const struct sp_port *port = port_list[i];

        if (detect_m8_serial_device(port)) {
            char *port_name = sp_get_port_name(port);
            fprintf(stderr, "Found M8 in %s\n", port_name);
            sp_copy_port(port, &m8_port);
            if (preferred_device != NULL && strcmp(preferred_device, port_name) == 0) {
                fprintf(stderr, "Found preferred device, breaking");
                break;
            }
        }
    }

    sp_free_port_list(port_list);

    if (m8_port != NULL) {
        // Open the serial port and configure it
        fprintf(stderr, "Opening port\n");

        result = sp_open(m8_port, SP_MODE_READ_WRITE);
        if (check(result) != SP_OK)
            return 0;

        result = sp_set_baudrate(m8_port, 115200);
        if (check(result) != SP_OK)
            return 0;

        result = sp_set_bits(m8_port, 8);
        if (check(result) != SP_OK)
            return 0;

        result = sp_set_parity(m8_port, SP_PARITY_NONE);
        if (check(result) != SP_OK)
            return 0;

        result = sp_set_stopbits(m8_port, 1);
        if (check(result) != SP_OK)
            return 0;

        result = sp_set_flowcontrol(m8_port, SP_FLOWCONTROL_NONE);
        if (check(result) != SP_OK)
            return 0;
    } else {
        if (verbose) {
            fprintf(stderr, "Cannot find a M8.\n");
        }
        return 0;
    }

    return 1;
}

/**
 * Checks the result of a serial port operation and prints an appropriate error message.
 *
 * @param result The result of the serial port operation, which is of type enum sp_return.
 * @return The same result that was passed in.
 */
static int check(const enum sp_return result) {
    char *error_message;

    switch (result) {
        case SP_ERR_ARG:
            fprintf(stderr, "Error: Invalid argument.\n");
            break;
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            fprintf(stderr, "Error: Failed: %s\n", error_message);
            sp_free_error_message(error_message);
            break;
        case SP_ERR_SUPP:
            fprintf(stderr, "Error: Not supported.\n");
            break;
        case SP_ERR_MEM:
            fprintf(stderr, "Error: Couldn't allocate memory.\n");
            break;
        case SP_OK:
        default:
            break;
    }
    return result;
}

/**
 * Resets the M8 display by sending a reset command to the serial port.
 *
 * @return Returns 1 if the display reset command was successfully written to the serial port, otherwise returns 0.
 */
int reset_display() {
    fprintf(stderr, "Reset display\n");

    const char buf[1] = {'R'};
    const int result = sp_blocking_write(m8_port, buf, 1, 5);
    if (result != 1) {
        fprintf(stderr, "Error resetting M8 display, code %d", result);
        return 0;
    }
    return 1;
}

/**
 * Enables the M8 display and then resets it.
 *
 * @return Returns 1 if both enabling and resetting the display are successful, otherwise returns 0.
 */
int enable_and_reset_display() {
    fprintf(stderr, "Enabling and resetting M8 display\n");

    const char buf[1] = {'E'};
    int result = sp_blocking_write(m8_port, buf, 1, 5);
    if (result != 1) {
        fprintf(stderr, "Error enabling M8 display, code %d", result);
        return 0;
    }

    result = reset_display();

    return result;
}

/**
 * Disconnects the M8 device by sending a disconnect signal and closing the serial port.
 *
 * @return Returns 1 if the disconnect command is successfully sent, otherwise returns 0.
 */
int disconnect() {
    fprintf(stderr, "Disconnecting M8\n");

    const char buf[1] = {'D'};

    int result = sp_blocking_write(m8_port, buf, 1, 5);
    if (result != 1) {
        fprintf(stderr, "Error sending disconnect, code %d", result);
        result = 0;
    }
    sp_close(m8_port);
    sp_free_port(m8_port);
    m8_port = NULL;
    return result;
}

/**
 * Reads data from a non-blocking serial port into the provided buffer.
 *
 * @param serial_buf A pointer to a buffer where the read data will be stored.
 * @param count The number of bytes to attempt to read from the serial port.
 * @return The number of bytes read, or a negative value if there is an error.
 */
int serial_read(uint8_t *serial_buf, const int count) {
    return sp_nonblocking_read(m8_port, serial_buf, count);
}

/**
 * Sends a control message to the controller via a serial port.
 *
 * @param input The input byte to be sent to the controller.
 * @return Returns 1 if the message is successfully sent, otherwise returns -1.
 */
int send_msg_controller(const uint8_t input) {
    const char buf[2] = {'C', input};
    const size_t nbytes = 2;
    const int result = sp_blocking_write(m8_port, buf, nbytes, 5);
    if (result != nbytes) {
        fprintf(stderr, "Error sending input, code %d", result);
        return -1;
    }
    return 1;
}

/**
 * Sends a keyjazz message to the M8 device over a serial connection.
 *
 * @param note The MIDI note value to be sent.
 * @param velocity The velocity of the note to be sent. Value is clamped to a maximum of 0x7F.
 * @return Returns 1 on successful transmission, otherwise returns -1 on failure.
 */
int send_msg_keyjazz(const uint8_t note, uint8_t velocity) {
    if (velocity > 0x7F)
        velocity = 0x7F;
    const char buf[3] = {'K', note, velocity};
    const size_t nbytes = 3;
    const int result = sp_blocking_write(m8_port, buf, nbytes, 5);
    if (result != nbytes) {
        fprintf(stderr, "Error sending keyjazz, code %d", result);
        return -1;
    }

    return 1;
}