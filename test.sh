#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./nvcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$expected expected, but got $actual"
	exit 1
    fi
}

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
try 24 "b=c=2+15*2-8;"
try 0 "a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=x=w=x=y=z=0;"
try 0 "foo=0;"
try 6 "a=1;b=2;c=3;a+b+c;"
try 60 "a=3;b=4;c=5;d=a*b*c;"
echo;

echo --return statement--
try 0 "return 0;"
try 23 "return 10*2-1+2*2;"
echo;

echo --control flow--
try 1 "if(1) return 1;"
try 4 "a=1;b=2;if(1) a=b*2; return a;"
try 1 "a=1;b=2;if(0) a=b*2; return a;"
echo;

echo OK
