#!/bin/bash

avrdude -p help -c avrisp 2>&1 | grep = | awk '{print "<< " "\\\"" $1"\\\""}' | xargs