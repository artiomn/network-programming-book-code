#!/bin/sh

SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
IMAGE_TAG=artiomn/nb-qt-creator-image

docker build -t "${IMAGE_TAG}" "${SCRIPT_PATH}" || exit 1

[ "$1" = "-p" ] && docker push "${IMAGE_TAG}"
