# m8js - the premium LSDJ gamepad

The **m8js** project translates keypad commands from a Dirtywave M8 tracker to a virtual joystick in Linux.

Just connect the device to your computer via USB and start the application, a new joystick device called "M8 Virtual
Joystick" should appear.

I suggest to remove the M8 SD card before using this to not mess up your songs by accident.

## Prerequisites

- **CMake:** Ensure you have CMake installed on your system.
- **libserialport:** You will need `libserialport` along with its development headers.

## Building the Project

To build the project, follow these steps:

1. **Clone the Repository:**
    ```sh
    git clone https://github.com/laamaa/m8js.git
    cd m8js
    ```

2. **Generate Build Files using CMake:**
    ```sh
    cmake .
    ```

3. **Build the Project:**
    ```sh
    make
    ```

## Contributing

Contributions are welcome! If you want to contribute to this project, please follow these steps:

1. Fork the repository.
2. Create a new branch: `git checkout -b my-feature-branch`
3. Make your changes and commit them: `git commit -m 'Add some feature'`
4. Push to the branch: `git push origin my-feature-branch`
5. Create a pull request on GitHub.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Thank you **Trash80** for creating the wonderful M8 Tracker.
- Thanks to **Grant Edwards** for providing a brilliant demo of the Linux uinput
  system: https://github.com/GrantEdwards/uinput-joystick-demo
- Special thanks to the contributors of the **m8c** project which was used as a base for this project.