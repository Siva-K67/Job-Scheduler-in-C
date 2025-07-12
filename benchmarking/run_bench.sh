#!/usr/bin/env bash

# Usage: ./run_bench.sh [secs] [stress-cpu-load%]
DURATION=${1:-60}
LOAD=${2:-70}

mkdir -p ../logs

echo "▶ Spawning stress-ng ($LOAD% for $DURATION s)…"
stress-ng --cpu 4 --cpu-load "$LOAD" --timeout "${DURATION}s" &

STRESS_PID=$!
sleep 2  # let stress-ng warm up

echo "▶ Starting perf recording for server.exe…"
perf record -e task-clock,context-switches,cpu-migrations -g -o ../logs/perf.data -- ../server.exe &

SERVER_PID=$!
echo "Server PID $SERVER_PID – now run clients in another shell."
echo "Hit ENTER to stop."
read

kill $SERVER_PID 2>/dev/null
wait $SERVER_PID
kill $STRESS_PID 2>/dev/null

echo "✔ Bench complete. Perf data in ../logs/perf.data"
