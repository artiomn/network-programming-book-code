#!/bin/sh

SCRIPT_DIR=$(dirname $(readlink -f $0))

export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
# afl-fuzz -i "${SCRIPT_DIR}/seeds" -o "${SCRIPT_DIR}/afl-output" -- "${SCRIPT_DIR}/build/bin/incorrect-udp-client" localhost 2000
afl-fuzz -t 2000 -i "${SCRIPT_DIR}/http_seeds" -o "${SCRIPT_DIR}/afl-output" -- "${SCRIPT_DIR}/build/bin/tcp-client" google.com 80

