#!/bin/sh

SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
SOURCE_PATH="${SOURCE_PATH:-/usr/src/gb}"
USER_NAME="${USER_NAME:-developer}"
EXT_UID=${EXT_UID:-0}

export DISPLAY=":0"

if [ "${EXT_UID}" -ne 0 ]; then
    groupadd -g "${EXT_GID}" "${USER_NAME}" && \
        useradd -m -u "${EXT_UID}" -g "${EXT_GID}" -Gsudo,root "${USER_NAME}" && \
        chown "${USER_NAME}:${USER_NAME}" "${SOURCE_PATH}" && \
        echo "${USER_NAME} ALL=(ALL) NOPASSWD: ALL" >> "/etc/sudoers.d/${USER_NAME}" && \
        ln -s "${SOURCE_PATH}/src" "/home/${USER_NAME}" && \
        exec gosu "${USER_NAME}" "$@"
fi

exec "$@"
