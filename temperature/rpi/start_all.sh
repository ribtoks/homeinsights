#!/bin/bash

WORKDIR="/home/pi/Projects/homeinsights/temperature/rpi/workdir"

pushd ${WORKDIR}

echo "Current PID: $$"

../tempserver/tempserver temps.db &
echo "Tempserver PID: $!"

echo "Waiting for tempserver to init DB..."
sleep 3s

../webserver/webserver -db-path temps.db &
echo "Webserver PID: $!"

popd

echo 'Done.'
