#!/bin/bash
while true; do
  ./test_map
  if [ $? -ne 0 ]; then
    exit
  fi
done
