#!/bin/sh

SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

if [ $# -ne 2 ]; then
    echo "$(basename $0) <YAML API description> <output directory>"
    exit 1
fi

OUTPUT_DIR="$2"
API_FILE="$1"

if [ ! -d "${OUTPUT_DIR}" ]; then
    mkdir -p "${OUTPUT_DIR}"
fi

java -jar "${SCRIPT_PATH}/openapi-generator-cli.jar" generate -i "${API_FILE}" -g cpp-restbed-server -o "${OUTPUT_DIR}"

