#!/bin/bash

pushd tempserver
echo "Building [tempserver]..."
make clean
make
popd

pushd webserver
echo "Building [webserver]..."
go build -o webserver
popd

echo 'Done.'
