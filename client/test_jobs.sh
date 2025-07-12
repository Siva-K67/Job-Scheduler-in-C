#!/bin/bash

echo "📦 Auto-sending 5 test jobs..."

run_job() {
  echo -e "$1\n$2\n$3\n$4" | ./client.exe
}

run_job Alice weld    2 10
sleep 1
run_job Bob   paint   5 16
sleep 1
run_job Charlie drill 1 14
sleep 1
run_job David cut     3 15
sleep 1
run_job Eva   lathe   4 17
