# Low Code: LP Core

## Get Started

There are multiple ways to start the development.

- Manually setup the from terminal on your local machine: [Get Started with Terminal](./docs/getting_started_terminal.md)
- VS Code setup on your local machine: [Get Started with VS Code](./docs/getting_started_vscode.md)
- Using Codespaces on GitHub: [Get Started with Codespaces](#start-codespace) (Recommended. Continue reading below.)

### Start Codespace

- Go to <https://github.com/chiragatal/esp-lowcode-matter/> and make sure you are logged in to GitHub
- Click on the green "Code" button -> Codespaces -> Create Codespace on Main
- This might take about **5 minutes** to setup
- In the process, the codespace will **restart** a few times

### Start Creating

There are a few commands avaliable with "Lowcode:" prefix. There are also **buttons on the bottom** of the screen.

- **Select Product**: Start by selecting the product that you want to create

These needs to be done only once for each device. But needs to be done separately for each device.

- **Select Port**: Connect your **esp32c6** board to your computer via USB, and select the port
- **Flash Prebuilt Binaries**: Flash the prebuilt binaries to your esp32c6 board
- **Generate Per Device Data**: Generate the required device certificates and the qr code for the device
- **Flash Per Device Data**: Flash the generated device certificates and qr code data
- **QR Code**: Open the QR code in a new tab

> Note: The QR code is only available when the product side code is flashed to the device.

The product side code being light weight, the edit, build, debug cycle is fast.

- **Build**: Build the selected product
- **Flash**: Flash the built product to your esp32c6 board
- **Console**: Open the device console to view the logs

Some other commands to help with development:

- **Setup**: Setup the complete development environment
- **Erase Flash**: Erase the flash storage
- **Menuconfig**: Open the menuconfig for the selected product
- **Product Clean**: Clean the build system
