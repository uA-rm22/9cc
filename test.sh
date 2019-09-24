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

try 42 "42;"
try 21 "5+20-4;"
try 41 "12 + 34 -5; "
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4  "(3+5)/2;"
try 10 "-10+20;"
try 1  "50>20;"
try 0  "-10>4;"
try 0  "50<20;"
try 1  "-10<4;"
try 0  "41-84>=41;"
try 0  "12>=9*6;"
try 0  "50<=20;"
try 0  "10<=-4;"
try 1  "10<=15;"

try 1  "50!=20;"
try 1  "15==15;"
try 0  "50!=20+30;"
try 0  "15-1==15;"

try 81  "a=9;b=9;a*b;"
try 1  "a=9;b=9;a==b;"

try 5 "return 5;"
try 7 "returnx = 7; return returnx;"

try 9 "if(5>1) 9;"
try 7 "if(-7>1) 9;else 7;"

try 9 "a=9;if(a>7){return a;} 8;"
try 30 "a=0;while(a<30){a=a+1;} return a;"
try 16 "boo=0;for(i=0; i<8; i=i+1){boo = boo+1; boo = boo + 1; } boo;"

echo OK