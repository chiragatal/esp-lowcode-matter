# ESP LowCode: Matter

* [ESP LowCode: Matter](#esp-lowcode-matter)
* [VSCode](#vscode)
* [Installing Extensions](#installing-extensions)
* [Setup Environment](#setup-environment)
* [Start Creating](#start-creating)
* [Next Steps](#next-steps)
* [More](#more)

If you don't want to use [Codespaces](../README.md#start-codespace) (recommended), you can manually setup the development environment on your local machine.

You can either use VS Code directly (continue reading below) or also use [Terminal](./getting_started_terminal.md) to setup the development environment.

## VSCode

### Installing Extensions

* Click on the extensions icon on the left sidebar
* Search for "Matter LowCode"
* Click on "Install"

### Setup Environment

* **Setup Environment**: Click on the "Setup" button on the bottom of the screen.
* This will clone the required repositories and install the dependencies and tools.
* This might take upto **30 minutes** to complete.

### Start Creating

These **buttons** are available on the **bottom of the screen (status bar)**. There are also VS Code commands (ctrl/cmd + shift + p) available with **"Lowcode:"** prefix for the same.

* **Select Product**: Start by selecting the product that you want to create
* **Select Port**: Connect your **esp32c6** board to your computer via USB, and select the port
* **Prepare Device**: This will erase the flash on the device and flash the prebuilt binaries to your esp32c6 board
* **Upload Configuration**: This will generate the required device certificates and the qr code for the device and flash them to the device
* **Upload Code**: This will build, flash and run the code on the device

## Next Steps

* [Device setup and control](device_setup.md)
* [Create and customize your own product](create_product.md)

## More

Some other commands to help with development:

* **Build**: Build the selected product
* **Flash**: Flash the built product to your esp32c6 board
* **Console**: Open the device console to view the logs
* **Erase Flash**: Erase the flash storage
* **Menuconfig**: Open the menuconfig for the selected product
* **Product Clean**: Clean the build system

## Related Documents

* [Getting Started: Codespaces](../README.md)
* [Create LowCode Product](./create_product.md)
* [Product Configuration](./product_configuration.md)
* [All Documents](./all_documents.md)
