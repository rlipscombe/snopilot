#!/bin/sh

tail +2 < $1 | cut -d, -f3,4,5 | tr ',' ' ' | \
	awk '{print $2,$1,$3}'
