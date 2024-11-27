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

- **Erase Flash**: Erase the flash storage
- **Menuconfig**: Open the menuconfig for the selected product
- **Product Clean**: Clean the build system
