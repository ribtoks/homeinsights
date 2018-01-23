#!/bin/bash
pushd workdir

../tempserver/tempserver temps.db &

../webserver/webserver temps.db &

popd

echo 'Done.'
