#!/bin/bash
cd $(dirname $0)
export DYLD_LIBRARY_PATH=$PWD
./screencast
