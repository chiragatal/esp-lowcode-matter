# Low Code: Matter

LowCode lies in between ESP ZeroCode (<https://zerocode.espressif.com/>) which is the fastest way to go to production and ESP Matter (<https://github.com/espressif/esp-matter>) which provides complete flexibility to build Matter devices.

Checkout more about the what fits your need in the [comparison](./docs/comparison.md).

- Want ESP ZeroCode with slight customization? Use ESP LowCode
- ESP Matter is getting complex and want to prototype quickly? Use ESP LowCode

## Get Started

- [Get Started with Codespaces](#start-codespace) (recommended): Use the browser itself to start the development.

> Codespaces for LowCode currently only works with Chromium based browsers such as Chrome, Edge, etc.

Some other ways to start the development:

- [Get Started with Terminal](./docs/getting_started_terminal.md): Manually setup from terminal on your local machine
- [Get Started with VS Code](./docs/getting_started_vscode.md): VS Code setup on your local machine

### Start Codespace

- Go to <https://github.com/chiragatal/esp-lowcode-matter/> and make sure you are logged in to GitHub
- Click on the green "Code" button -> Codespaces -> Create Codespace on Main
- This might take about **5 minutes** to setup
- In the process, the codespace will **restart** a few times

### Start Creating

These **buttons** are available on the **bottom of the screen (status bar)**. There are also Codespaces commands (ctrl/cmd + shift + p) available with **"Lowcode:"** prefix for the same.

- **Select Product**: Start by selecting the product that you want to create
- **Select Port**: Connect your **esp32c6** board to your computer via USB, and select the port
- **Prepare Device**: This will erase the flash on the device and flash the prebuilt binaries to your esp32c6 board
- **Upload Configuration**: This will generate the required device certificates and the qr code for the device and flash them to the device
- **Upload Code**: This will build, flash and run the code on the device

## Next Steps

- [Device setup and control](./docs/device_setup.md)
- [Create and customize your own product](./docs/create_product.md)

## More

Some other commands to help with development:

- **Build**: Build the selected product
- **Flash**: Flash the built product to your esp32c6 board
- **Console**: Open the device console to view the logs
- **Erase Flash**: Erase the flash storage
- **Menuconfig**: Open the menuconfig for the selected product
- **Product Clean**: Clean the build system
