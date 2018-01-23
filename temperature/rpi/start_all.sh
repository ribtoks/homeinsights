#!/bin/bash
pushd workdir

../tempserver/tempserver temps.db &

../webserver/webserver -db-path temps.db &

popd

echo 'Done.'
