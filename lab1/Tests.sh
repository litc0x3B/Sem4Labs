#!/bin/bash

readonly testProgram=./PrintArgs.sh
failedCounter=0

TestPassed () {
  echo "[  OK ] $1: test is passed"
}

TestFailed () {
  let failedCounter++
  echo "[ FAIL] $1: test is not passed"
}

TestStart () {
  echo "[START] $1: test is started"
}

PerformTest () {
  local name=$1
  local expected=$2
  shift 2
  local args=( "$@" )

  TestStart $name 
  local result=$($testProgram ${args[@]})
  
  # echo "$result $expected"
  if [ "$result" = "$expected" ]
    then
      TestPassed $name
    else 
     TestFailed $name
     echo -e "\tgot \"$result\" but \"$expected\" expected" 
  fi
}

PerformTest Test1 "c b a" a b c
PerformTest Test2 "b a" a b
PerformTest Empty ""

[ $failedCounter -eq 0 ] && echo "all tests were passed succesfully" || echo "number of failed tests: $failedCounter"
