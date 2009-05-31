#!/bin/sh

tail +2 < $1 | cut -d, -f8,9 | tr ',' ' ' | \
	proj +proj=merc | awk '{print $2,$1}' > $2

tail +2 < $3 | cut -d, -f3,4,5 | tr ',' ' ' | proj +proj=merc | \
	awk '{print $2,$1,$3}' > $4
