#!/bin/sh

tail +2 < $1 | cut -d, -f8,9 | tr ',' ' ' | \
	proj +proj=merc | awk '{print $2,$1}'
