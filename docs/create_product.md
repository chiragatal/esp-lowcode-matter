# Create and Customise your own product

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
