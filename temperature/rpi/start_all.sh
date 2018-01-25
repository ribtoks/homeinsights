#!/bin/bash
pushd workdir

echo "Current PID: $$"

../tempserver/tempserver temps.db &
echo "Tempserver PID: $!"

../webserver/webserver -db-path temps.db &
echo "Webserver PID: $!"

popd

echo 'Done.'
