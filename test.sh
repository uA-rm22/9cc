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

#./9cc "foo();" > tmp.s
#gcc -o tmp tmp.s foo.o
#./tmp

#./9cc "a=6; b = 8; boo(2, 4, a, b);" > tmp.s
#gcc -o tmp tmp.s foo.o
#./tmp

try 42 "main(){return 42;}"
try 49 "main(){a=9;if(a>4){return 49;}else{return 5;}}"
try 10 "main(){i=0;while(i<10){i=i+1;}return i;}"
try 5 "fact(n){return n;} main(){return fact(5);}"
try 120 "fact(n){if(n==0){return 1;}m=fact(n-1);return n*m;} main(){return fact(5);}"
try 21 "fib(num){
          if(num == 1){
            return 1;
          }
          if(num == 0){
            return 1;
          }
          return fib( num-1 ) + fib(num-2);
        }
        main(){
          return fib(7);
        }"
echo OK