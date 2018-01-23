#!/bin/bash

pushd tempserver
make clean
make
popd

pushd webserver
go build -o webserver
popd

echo 'Done.'
