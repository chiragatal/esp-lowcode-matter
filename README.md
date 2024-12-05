# Low Code: LP Core

## Get Started

There are multiple ways to start the development.

- Manually setup the from terminal on your local machine: [Get Started with Terminal](./docs/getting_started_terminal.md)
- VS Code setup on your local machine: [Get Started with VS Code](./docs/getting_started_vscode.md)
- (Recommended) Using Codespaces on GitHub: [Get Started with Codespaces](#start-codespace) (Continue reading below.)

### Start Codespace

- Go to <https://github.com/chiragatal/esp-lowcode-matter/> and make sure you are logged in to GitHub
- Click on the green "Code" button -> Codespaces -> Create Codespace on Main
- This might take about **5 minutes** to setup
- In the process, the codespace will **restart** a few times

### Start Creating

There are a few commands avaliable with "Lowcode:" prefix. There are also **buttons on the bottom** of the screen.

- **Select Product**: Start by selecting the product that you want to create
- **Select Port**: Connect your **esp32c6** board to your computer via USB, and select the port
- **Prepare Device**: Erase the flash on the device and flash the prebuilt binaries to your esp32c6 board
- **Upload Configuration**: Generate the required device certificates and the qr code for the device and flash them to the device
- **Upload Code**: Build, flash and run the code on the device

Some other commands to help with development:

- **Build**: Build the selected product
- **Flash**: Flash the built product to your esp32c6 board
- **Console**: Open the device console to view the logs
- **Erase Flash**: Erase the flash storage
- **Menuconfig**: Open the menuconfig for the selected product
- **Product Clean**: Clean the build system
