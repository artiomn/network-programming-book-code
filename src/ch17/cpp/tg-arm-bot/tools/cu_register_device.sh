#!/bin/sh -x

curl -X POST \
    -w " %{http_code}\n" \
    -d "{\"name\": \"New Device\", \"id\": \"device-unique-id\"}" \
    -H "Content-Type: application/json" \
    "http://localhost:8080/artiomn/robotic_arm_server/1.0.0/devices"
