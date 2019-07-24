#!/bin/bash

objdumpCommand="$1"
options="$2"
objfile="$3"
outputFile="$4"

"$objdumpCommand" $options "$objfile" > "$outputFile"
