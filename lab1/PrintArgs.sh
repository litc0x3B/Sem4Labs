#!/bin/bash

args=( "$@" )

for i in $(seq  $(($#-1)) -1 0); do
  echo -n "${args[$i]}"
  if [ 0 -ne $i ]; then
    echo -n " "
  fi
done

if [ $# -ne 0 ]; then echo; fi
