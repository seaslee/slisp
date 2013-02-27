slisp
=====

Lisp is a functional language with a long history and Scheme is one of the two main dialects of Lisp.
slisp is a simple interpreter implementation implenmentation of Scheme covered in SICP with C language.
It may can't pass the standard Scheme interpreter test such as R6RS. It just a toy language for fun and 
study.

# 1.Data type
slisp support boolean,int,float,symbol and pair.Every data type has it's corresponding operation.
- boolean. It support 'and','or','not' logic operation and the relation operation '>' '>=' '<' '<=' '=='
  return boolean type value.
- int,float. It support arithmatic operations '+','-','*','/','%' and relation operation '>' '>=' '<' '<=' '=='.
  In arithmatic operations, if all are int, it will return int type value; otherwise, it will return float type value.
- symbol. 
