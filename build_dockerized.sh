#!/bin/sh

SCRIPT_PATH="$(dirname -- "${0}")"
SCRIPT_PATH="$(cd -- "${SCRIPT_PATH}" && pwd)"

if [ "${INSIDE_NB_DOCKER_CONTAINER}" = "1" ]; then
    echo "Probably you're inside the Docker container. \"${SCRIPT_PATH}/build.sh\" will be run."
    exec "${SCRIPT_PATH}/build.sh"
fi

if ! command -v docker &>/dev/null; then
    echo "Docker binary was not found. You must to install it. Exiting..." >&2
    exit 1
fi

docker run --rm -v "$PWD:/usr/src/gb" -e "EXT_UID=$(id -u)" -e "EXT_GID=$(id -g)" -w /usr/src/gb artiomn/nb-build-image ./build.sh $*
