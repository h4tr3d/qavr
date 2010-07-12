#!/bin/bash

avrdude -p t2313 -c help 2>&1 | grep = | awk '{print "<< " "\\\"" $1"\\\""}' | xargs