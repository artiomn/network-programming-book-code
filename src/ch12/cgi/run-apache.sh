#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export APACHE_SERVER_ROOT="${SCRIPT_DIR}"

SERVER_BINARY="$(command -v apache2)"
if [ -z "${SERVER_BINARY}" ]; then
    SERVER_BINARY=httpd
    MODULES_DIR="$(readlink /etc/httpd/modules)"
else
    MODULES_DIR="/usr/lib/apache2/modules"
fi

export MODULES_DIR
export APACHE_USER="$(id -un)"
export APACHE_GROUP="$(id -gn)"

echo "Root: ${APACHE_SERVER_ROOT}"
"${SERVER_BINARY}" -f "${SCRIPT_DIR}/httpd.conf" -d "${APACHE_SERVER_ROOT}" -DFOREGROUND || echo "Error!"

