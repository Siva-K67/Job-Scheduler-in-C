#!/bin/bash

echo "📦 Auto-sending 5 test jobs..."

./client <<EOF
Alice
weld
2
10
EOF

sleep 1

./client <<EOF
Bob
paint
5
6
EOF

sleep 1

./client <<EOF
Charlie
drill
1
4
EOF

sleep 1

./client <<EOF
David
cut
3
5
EOF

sleep 1

./client <<EOF
Eva
lathe
4
7
EOF

