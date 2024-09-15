// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#include "command.h"
#include "virtualjoystick.h"

#include <stdio.h>
#include <string.h>

enum m8_command_bytes {
    draw_rectangle_command = 0xFE,
    draw_rectangle_command_min_datalength = 5,
    draw_rectangle_command_max_datalength = 12,
    draw_character_command = 0xFD,
    draw_character_command_datalength = 12,
    draw_oscilloscope_waveform_command = 0xFC,
    draw_oscilloscope_waveform_command_mindatalength = 1 + 3,
    draw_oscilloscope_waveform_command_maxdatalength = 1 + 3 + 480,
    joypad_keypressedstate_command = 0xFB,
    joypad_keypressedstate_command_datalength = 3,
    system_info_command = 0xFF,
    system_info_command_datalength = 6
};

/**
 * Decodes a 16-bit integer from a byte array starting at a specified position.
 *
 * This function takes a pointer to a byte array and an offset, combining two
 * adjacent bytes starting from the given position to form a 16-bit integer.
 *
 * @param data Pointer to the byte array containing the data.
 * @param start Offset position in the byte array to start decoding.
 * @return Returns the decoded 16-bit integer value.
 */
static uint16_t decodeInt16(const uint8_t *data, const uint8_t start) {
    return data[start] | (uint16_t) data[start + 1] << 8;
}

static void dump_packet(const uint32_t size, const uint8_t *recv_buf) {
    for (uint16_t a = 0; a < size; a++) {
        fprintf(stderr, "0x%02X ", recv_buf[a]);
    }
    fprintf(stderr, "\n");
}

/**
 * Processes incoming command packets and handles them based on their type.
 *
 * This function takes packet data and its size, then performs various actions
 * depending on the command type specified in the first byte of the packet.
 * Supported commands include joypad key press state, system information,
 * character drawing, oscilloscope waveform drawing, and rectangle drawing.
 *
 * @param data Pointer to the packet data.
 * @param size Size of the packet data.
 * @return Returns 1 if the command was successfully processed, 0 otherwise.
 */
int process_command(uint8_t *data, uint32_t size) {
    uint8_t rx_buffer[size + 1];

    memcpy(rx_buffer, data, size);
    rx_buffer[size] = 0;

    switch (rx_buffer[0]) {
        case joypad_keypressedstate_command: {
            if (size != joypad_keypressedstate_command_datalength) {
                printf(
                    "Invalid joypad keypressed state packet: expected length %d, "
                    "got %d\n",
                    joypad_keypressedstate_command_datalength, size);
                dump_packet(size, rx_buffer);
                return 0;
            }

            if (send_virtual_joystick_message(rx_buffer[1])) {
                return 1;
            }
            return 0;
        }

        case system_info_command: {
            if (size != system_info_command_datalength) {
                fprintf(stderr,
                        "Invalid system info packet: expected length %d, got %d\n",
                        system_info_command_datalength, size);
                dump_packet(size, rx_buffer);
                break;
            }

            const char *hw_type[4] = {"Headless", "Beta M8", "Production M8", "Production M8 Model:02"};

            static int system_info_printed = 0;

            if (system_info_printed == 0) {
                fprintf(stderr, "** Hardware info ** Device type: %s, Firmware ver %d.%d.%d\n", hw_type[rx_buffer[1]],
                        rx_buffer[2], rx_buffer[3], rx_buffer[4]);
                system_info_printed = 1;
            }

            return 1;
        }
        case draw_character_command:
        case draw_oscilloscope_waveform_command:
        case draw_rectangle_command:
            // These are not needed
            break;

        default:
            fprintf(stderr, "Invalid packet");
            dump_packet(size, rx_buffer);
            return 0;
    }
    return 1;
}
