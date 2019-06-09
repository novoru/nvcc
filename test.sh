#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./nvcc "$input" > tmp.s
    gcc -static -g -o tmp tmp.s
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$expected expected, but got $actual"
	exit 1
    fi
}

try 42 "main() { return 42; }"
try 5  "main() { return 3 + 2; }"
try 4  "main() { return 7 - 3; }"
try 8  "main() { return 2 * 4; }"
try 4  "main() { return 12 / 3; }"
try 14 "main() { return (3 + 4) * 2; }"

try 1  "main() { return 1 < 3; }"
try 1  "main() { return 3 > 1; }"
try 0  "main() { return 0 == 1; }"
try 1  "main() { return 1 == 1; }"
try 0  "main() { return 0 != 0; }"
try 1  "main() { return 0 != 1; }"
try 1  "main() { return 1 <= 3;}"
try 1  "main() { return 3 >= 1;}"

echo OK
