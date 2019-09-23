#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 "12 + 34 -5; "
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4  "(3+5)/2;"
try 10 "-10+20;"
try 5  "-10/(-2);"
try 60 "-15*(-4);"
try 1  "50>20;"
try 0  "-10>4;"
try 0  "50<20;"
try 1  "-10<4;"
try 0  "41>=41+84;"
try 0  "12>=9*6;"
try 0  "50<=20;"
try 0  "10<=-4;"
try 1  "10<=15;"

try 1  "50!=20;"
try 1  "15==15;"
try 0  "50!=20+30;"
try 0  "15==15-1;"

try 2  "a=1;a+1;"
try 72  "a=81;b=9;a-b;"
try 81  "a=9;b=9;a*b;"
try 1  "a=9;b=9;a==b;"

echo OK