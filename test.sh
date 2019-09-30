#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s foo.o
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

try 42 "int main(){return 42;}"
try 49 "int main(){int a; a=9; if(a>4){return 49;}else{return 5;}}"
try 10 "int main(){int i; i=0; while(i<10){i=i+1;}return i;}"
try 5 "int fact(int n){return n;} int main(){return fact(5);}"
try 120 "int fact(int n){int m;if(n==0){return 1;}m=fact(n-1);return n*m;} int main(){return fact(5);}"
try 21 "int fib(int num){
          if(num == 1){
            return 1;
          }
          if(num == 0){
            return 1;
          }
          return fib( num-1 ) + fib(num-2);
        }
        int main(){
          return fib(7);
        }"

try 4 "int main(){int x; int y; int *z;x=4;y=9;z=&y+1;return *z;}"
try 84 "int a(int x){
						return b(x);
					}
					int b(int y){
						return y+5;
					}
					int main(){
						a(79);
					}"
try 3 "int main(){int x; int *y; y = &x; *y = 3; return x;}"
try 9 "int main(){int x;return 5+sizeof(x);}"
try 4 "int main(){int *p; p=alloc4(1,2,4,8); int *q; q = p+ 2; return *q;}"
try 1 "int main(){int a[5];*a=1; return *a;}"
try 3 "int main(){int a[5];*a=1;*(a+1)=2; int *p; p=a; return *p+*(p+1);}"
echo OK