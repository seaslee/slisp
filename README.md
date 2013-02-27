slisp
=====

Lisp is a functional language with a long history and Scheme is one of the two main dialects of Lisp.
slisp is a simple interpreter implementation implenmentation of Scheme covered in SICP with C language.
It may can't pass the standard Scheme interpreter test such as R6RS. It just a toy language for fun and 
study.

###1.Data type
slisp support boolean,int,float,symbol and pair.Every data type has it's corresponding operation.
- boolean. It's represented by the value #t,#T,#f,#F. It supports logic operations 'and','or','not' and the
  relation operations '>' '>=' '<' '<=' '==' return boolean type value.
- int,float. It supports arithmatic operations '+','-','*','/','%' and relation operation '>' '>=' '<' '<=' '=='.
  In arithmatic operations, if all are int, it will return int type value; otherwise, it will return float type value.
- symbol.
- pair. It supoorts 'cons','car','cdr' operations.

###2.Variables
The style of identifier of variables is C-like, not Scheme. It starts with alpha and '_' and the following can be
alpha,number and '_'. You can define a variable with the 'define' procedure.

###3.Control flow
- slisp support 'if' 'cond' expression to make choice. It is describled in SICP.
- slisp support recursion (not tail recursion).You can implementation iteration with the recursion way.
- slisp also support to define function.It is also describled in SICP.

###4.Examples
- Expressions:

```scheme
66
;value: 66
(+ 137 349)
;value: 486
(+ 10 5)
;value: 2
'a
;value: a
''a
;value: (quote a)
(define (abs x)
  (if (< x 0)
      (- x)
      x))
;value: ()
(abs 3)
;value: 3
(define (abs x)
  (cond ((< x 0)(- x))
      (else x)))
;value: ()
(abs 3)
;value: 3
(abs (- 3))
;value: -3
```
- Define variable and procedure:

```scheme
(define size 2)
;value: ()
size
;value: 2
(define (square x) (* x x))
;value: ()
(square 1024)
;value: 1048576
```
- Recursion:

```scheme
(define (factorial n)
  (if (== n 1)
      1
      (* n (factorial (- n 1)))))
;value: ()
(factorial 9)
;value: 362880
```

slisp can support many(I can't test all) examples of Scheme code in SICP. I think it's will be better!
