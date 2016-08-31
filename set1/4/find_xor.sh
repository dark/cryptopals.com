#!/bin/bash

while read line; do
  if ! echo -n "$line" | ./xorcipher.bin &> /dev/null; then
    # quietly skip lines with no results
    continue
  fi
  echo "Trying for: $line"
  echo -n "$line" | ./xorcipher.bin
done < input
