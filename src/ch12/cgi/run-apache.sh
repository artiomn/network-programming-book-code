#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export APACHE_SERVER_ROOT="${SCRIPT_DIR}"

echo "Root: ${APACHE_SERVER_ROOT}"
httpd -f "${SCRIPT_DIR}/httpd.conf" -d "${APACHE_SERVER_ROOT}" -DFOREGROUND || echo "Error!"

