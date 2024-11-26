#!/usr/bin/env bash

set -e

# Function to create a status JSON file
create_status_json() {
    local product_folder="$1"
    local status="$2"
    local description="$3"
    local details="$4"
    local status_file="$product_folder/configuration/output/status.json"

    echo "{\"status\": \"$status\", \"description\": \"$description\", \"details\": \"$details\"}" > "$status_file"
}

# Take the path to product folder as an input
if [ $# -eq 0 ]; then
    echo "Error: Please provide the path to the product folder as an argument."
    echo "Usage: $0 <path_to_product_folder>"
    exit
fi

product_folder="$1"

# Validate if the provided path exists and is a directory
if [ ! -d "$product_folder" ]; then
    echo "Error: The provided path '$product_folder' is not a valid directory."
    exit
fi

# Print the product folder path for confirmation
product_folder=$(realpath "$product_folder")
echo "Using product folder: $product_folder"
mkdir -p "$product_folder/configuration/output"

# Check if ESP_MATTER_PATH is set
if [ -z "$ESP_MATTER_PATH" ]; then
    echo "Error: ESP_MATTER_PATH is not set. Please set it before running this script."
    echo "You can set it by running: export ESP_MATTER_PATH=/path/to/esp-matter"
    create_status_json "$product_folder" "Failure" "ESP_MATTER_PATH is not set" "Please set it by running: export ESP_MATTER_PATH=/path/to/esp-matter"
    exit
fi

# Find the first .zap file in the product folder
zap_file=$(find "$(realpath "$product_folder/configuration")" -name "*.zap" -print -quit)

# Check if a .zap file was found
if [ -z "$zap_file" ]; then
    echo "Error: No .zap file found in $product_folder/configuration"
    create_status_json "$product_folder" "Failure" "No .zap file found" "Check the logs for more details"
    exit
fi

echo "Using .zap file: $zap_file"
cp "$zap_file" "$product_folder/configuration/output/data_model.zap" 2>/dev/null || true
zap_file=$(realpath "$product_folder/configuration/output/data_model.zap")

# Use the found .zap file as input for the generate.py script
$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/scripts/tools/zap/generate.py "$zap_file"

# Find the first .matter file in the product folder
matter_file=$(find "$(realpath "$product_folder/configuration/output")" -name "*.matter" -print -quit)

# Check if a .matter file was found
if [ -z "$matter_file" ]; then
    echo "Error: No .matter file found in $product_folder/configuration/output"
    create_status_json "$product_folder" "Failure" "No .matter file found" "Check the logs for more details"
    exit
fi

echo "Using .matter file: $matter_file"

# Call main.py from any_device
cd "$ESP_MATTER_PATH/examples/any_device/python-tools"
python3 main.py "$matter_file"

# Check if the script executed successfully
if [ $? -ne 0 ]; then
    echo "Error: Failed to execute main.py"
    create_status_json "$product_folder" "Failure" "Failed to execute main.py" "Check the logs for more details"
    exit
fi

cd -
echo "Successfully generated binary and JSON files from .matter file"

cp "$product_folder/configuration/output/data_model.bin" "$product_folder/configuration/data_model.bin" 2>/dev/null || true

# Get the current script's path
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SCRIPT_DIR/../mfg

if [ -z "$2" ]; then
    python3 mfg_gen.py --product configuration --products_path $product_folder --test --output_path $product_folder/configuration/output --local_claim --no_rainmaker --no_signature --no_ota_decryption || exit_code=$?
else
    chip="$2"
    mac_address="$3"
    python3 mfg_gen.py --product configuration --products_path $product_folder --output_path $product_folder/configuration/output --local_claim --no_rainmaker --no_signature --no_ota_decryption --not_connected_device_details $chip $mac_address || exit_code=$?
fi
if [ $exit_code -ne 0 ]; then
    echo "Error: mfg_gen.py execution failed"
    create_status_json "$product_folder" "Failure" "mfg_gen.py execution failed" "Check the logs for more details"
    exit
fi
cd -
create_status_json "$product_folder" "Success" "Successfully generated binary and JSON files from .matter file" "Check the logs for more details"
