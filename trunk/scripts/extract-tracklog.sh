#!/bin/sh

tail +2 < $1 | cut -d, -f5,8,9 | tr ',' ' ' | \
	awk '{print $3,$2,$1}'
