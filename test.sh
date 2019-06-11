#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./nvcc "$input" > tmp.s
    gcc -static -g -o tmp tmp.s tmp-plus.o
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$expected expected, but got $actual"
	exit 1
    fi
}

echo "int plus(int x, int y) { return x + y; }" | gcc -xc -c -o tmp-plus.o -

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

try 2  "main() { if(1) return 2; else return 3; }"
try 3  "main() { if(0) return 2; else return 3; }"
try 10 "main() { int i; i = 0; while(i < 10) i = i + 1; return i; }"
try 10 "main() { int i; for(i = 0; i < 10; i = i + 1) i = i; return i;}"

try 3  "main() { int a;a = 3; return a; }"
try 10 "main() { int a; int b; a = 1; b = 9; return a + b; }"
try 5  "main() { return plus(3, 2); }"
try 42 "foo() { return 42; } main() { return foo();}"
try 5  "foo() { int a; int b; a = 3; b = 4; return a + b; } main() { int a; a = 2; return foo() - a; }"
try 7  "sub(int a, int b) { return a - b; } main() { return sub(10, 3); }"

echo OK
