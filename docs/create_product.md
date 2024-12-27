# Create and Customise your own product

* [Create and Customise your own product](#create-and-customise-your-own-product)
* [Understanding the folder structure](#understanding-the-folder-structure)
* [Create a new product](#create-a-new-product)
* [Customise the data model](#customise-the-data-model)
* [Add new drivers and components](#add-new-drivers-and-components)
* [Porting from Arduino](#porting-from-arduino)

With LowCode, you can either customise an existing product or create a completely new one.

## Understanding the folder structure

The folder structure is as follows:

```text
esp-lowcode-matter/
    products/                               // Products directory
        <product_name>/
            configuration/
                data_model.zap              // Data model for the product
                product_config.json         // Additional product configuration
                product_info.json           // Additional product information
                ...
            main/
                app_main.cpp                // Main application code
                app_driver.cpp              // Driver code
                app_priv.h                  // Application private header file
                CMakeLists.txt              // Application CMakeLists.txt file
            CMakeLists.txt                  // Top level CMakeLists.txt file
            ...
        ...
    components/                             // Components and drivers directory
        <component_name>/
            CMakeLists.txt                  // Component CMakeLists.txt file
            ...                             // Component files
        ...
    docs/                                   // LowCode documentation
        ...
```

These are the primary files and directories that you will be working with for customising your product.

## Create a new product

For creating a new product, you can use the `LowCode: Create Product` command. It will prompt you for the product name and the reference of an existing product. It is recommended to choose a reference product that is similar/closest to the product you are creating. If you are not sure, you can choose the `template` product as a reference and start from scratch.

This will create a new product in the `products` directory with the required files and directories.

## Customise the data model

Refer to [product_configuration.md](product_configuration.md) for more information on how to customise the data model.

## Add new drivers and components

For adding new drivers and components, you can add them to the `components` directory. It needs a `.c` file, a `.h` file and a `CMakeLists.txt` file. Make sure to add this new component to the `CMakeLists.txt` file in the `main` directory to use them in the product.

## Porting from Arduino

We’ve restructured some of the core Arduino functions to make them more descriptive and aligned with lowcode platform. Here’s a table mapping commonly used Arduino functions to their equivalents on lowcode platform:

| **Arduino Function**      | **LowCode Function**  | **Description**                        |
|---------------------------|----------------------------|----------------------------------------|
| `pinMode(pin, mode)`       | `system_set_pin_mode(pin, mode)`   | Configures the specified pin as INPUT or OUTPUT. |
| `digitalWrite(pin, value)` | `system_digital_write(pin, level)` | Writes a value to a digital pin (HIGH or LOW). |
| `digitalRead(pin)`         | `system_digital_read(pin)`     | Reads the value from a digital pin.    |
| `delay(ms)`                | `system_delay_ms(ms)`       | Pauses the program for a specified number of milliseconds. |
| `analogWrite`              | `TODO`       | Writes an analog value (PWM wave) to a pin. |
| `analogRead`               | `TODO`       | Reads the value from the specified analog pin. |

## Related Documents

* [Product Configuration](./product_configuration.md)
* [Debugging](./debugging.md)
* [Programmer's Model](./programmer_model.md)
* [Getting Started: Codespaces](../README.md)
* [All Documents](./all_documents.md)
