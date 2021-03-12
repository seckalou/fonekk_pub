#!/bin/bash

export MWM_RESOURCES_DIR=./data/
export MWM_WRITABLE_DIR=./data/

 app=MAPS.ME
if [[ "$OSTYPE" == "darwin"* ]]; then
	app=MAPS.ME.app/Contents/MacOS/MAPS.ME
fi
../omim-build-debug/$app
