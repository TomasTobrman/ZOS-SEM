#!/bin/sh
docker run \
	-v $(pwd):/data/entry \
	-v $(pwd)/res:/data/res \
	-v $(pwd)/aux:/data/aux \
	tirea-tex $1 $2
