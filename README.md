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


slisp can support many(I can't test all) examples of Scheme code in SICP. I think it's will be better!
