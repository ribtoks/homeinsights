#!/bin/bash

pushd tempserver
make clean
make
popd

pushd webserver
go build -o webserver
popd

pushd workdir

../tempserver/tempserver temps.db &

../webserver/webserver temps.db &

popd

echo 'Done.'
