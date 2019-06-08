#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./nvcc "$input" > tmp.s
    gcc -static -g -o tmp tmp.s tmp-foo.o tmp-bar.o tmp-plus.o tmp-sub.o
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$expected expected, but got $actual"
	exit 1
    fi
}

echo "void foo() { printf(\"OK\\n\"); }" | gcc -xc -c -o tmp-foo.o -
echo "void bar(int a) { printf(\"%d\\n\", a); }" | gcc -xc -c -o tmp-bar.o -
echo "void plus(int a, int b) { printf(\"%d\\n\", a+b); }" | gcc -xc -c -o tmp-plus.o -
echo "int sub(int a, int b) { printf(\"%d\\n\", a - b); return a - b; }" | gcc -xc -c -o tmp-sub.o -

echo --number--
try 0 "0;"
try 42 "42;"
echo;

echo --arithmetic operator--
try 21 '5+20-4;'
try 41 " 12 + 34 - 5;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 2 "-3+5;"
try 5 "+10-5;"
echo;

echo --comparison operator--
try 1 "-1<3;"
try 0 "10<-3;"
try 1 "12>4;"
try 1 "7==7;"
try 0 "3==4;"
try 1 "6!=1;"
try 0 "5!=5;"
try 0 "-12>-3;"
try 1 "-2<=7;"
try 0 "3<=0;"
try 1 "9>=-1;"
try 0 "-3>=-2;"
echo;

echo --container--
try 100 "1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1;"
echo;

echo --local variables--
try 1 "a=1;"
try 2 "a=1;a+1;"
try 24 "b=c=2+15*2-8;"
try 0 "a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=x=w=x=y=z=0;"
try 0 "foo=0;"
try 6 "a=1;b=2;c=3;a+b+c;"
try 60 "a=3;b=4;c=5;d=a*b*c;"
echo;

echo --return statement--
try 0 "return 0;"
try 23 "return 10*2-1+2*2;"
try 3 "a=1;b=2;return a+b;"
echo;

echo --control flow--
try 1 "if(1) return 1;"
try 4 "a=1;b=2;if(1) a=b*2; return a;"
try 1 "a=1;b=2;if(0) a=b*2; return a;"
try 2 "a=1;b=2;if(0) a=b*2; return b;"
try 8 "a=1;b=2;if(1) a=b*2; if(1) a=a*2;"
try 2 "a=1;b=2;if(0) a=b*2; if(1) a=a*2;"
try 1 "if(0) return 0; else return 1;"
try 0 "if(1) return 0; else return 1;"
try 10 "a=0;while(a<10) a=a+1; return a;"
try 0  "a=0;while(0)a=a+1;return a;"
try 20 "a=0;for(i=0;i<10;i=i+1)a=a+2;return a;"
try 20 "a=0;i=0;for(;i<10;i=i+1)a=a+2;return a;"
try 10 "i=0;for(;i<10;)i=i+1;return i;"
try 1 "i=1;{} return i;"
try 1 "{i=0;i=i+1;} return i;"
try 2 "i=0;if(1) {i=1;i=i+1;} return i;"
try 6 "i=0;if(0) {i=1;i=i+1;} else {i=3;i=i*2;} return i;"
try 20 "a=0;b=2;for(i=0;i<10;i=i+1) {a=a+b;} return a;"
echo;

echo --call function--
try 0 "foo();return 0;"
try 0 "bar(4);return 0;"
try 0 "plus(1,2);return 0;"
try 3 "return sub(5,2);"
try 11 "return sub(5,2) + sub(10,2);"
echo;

echo OK
