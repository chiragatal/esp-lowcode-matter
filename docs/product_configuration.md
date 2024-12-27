# Product Configuration

* [Product Configuration](#product-configuration)
* [Personalising the product](#personalising-the-product)
* [Customising the data model](#customising-the-data-model)

## Personalising the product

In the `esp-lowcode-matter/products/<your product>/configuration/product_info.json` file, you can change the manufacturer name, product name, Matter related IDs, model, and other details. These get reflected in the device setup process and in the device's display.

```text
vendor_name: "Espressif"        // Manufacturer/Vendor name
product_name: "Matter Product"  // Product name
vendor_id: 65521                // Matter Vendor ID
product_id: 32768               // Matter Product ID
origin_vendor_id: 65521         // Matter Origin Vendor ID
origin_product_id: 32768        // Matter Origin Product ID
device_type_id: 268             // Matter Device Type ID
```

## Customising the data model

The data model is defined in the `esp-lowcode-matter/products/<your product>/configuration/data_model.zap` file. You can use the [ZAP editor](https://product-configurator.espressif.com/) to edit the data model.

You can add new endpoints, clusters, and attributes to the device. You can also modify existing ones.

* It needs to be compliant with the **Matter specification**.
* It should have the **root node** on endpoint 0 and the other application nodes on other endpoints.
* Make sure you enable the required features and update the **feature_map** correctly.
* You can enable the **optional features** if you need them.
* You can also add **custom** clusters, attributes, etc. if you need them.
* Adding new endpoints, clusters, attributes, etc. require more **memory** internally. Do test the memory usage and the overall functionality if you are adding a lot of new features.
* Update the application code to match the new data model.

When you run "Upload Configuration", this will generate the updated `data_model.bin` and upload it to the device.

## Related Documents

* [Create LowCode Product](./create_product.md)
* [Production Considerations](./production_considerations.md)
* [All Documents](./all_documents.md)
