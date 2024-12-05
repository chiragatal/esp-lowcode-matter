# Low Code: LP Core

## Using VSCode

### Installing Extensions

- Click on the extensions icon on the left sidebar
- Search for "Matter LowCode"
- Click on "Install"

### Setup

- **Setup**: Click on the "Setup" button on the bottom of the screen.
- This will clone the required repositories and install the dependencies and tools.
- This might take upto **30 minutes** to complete.

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
