# ScriptLang

This is a simple scripting language written in C++17. This project was a way to improve my knowledge on developing programming languages, stack-based virtual machines, and even C++. In fact, the goal of this project is not to create a fast and efficient scripting programming language, but to learn how to parse and compile source code and how to output detailed error messages when one of these steps fails.

```
defun fib(n) {
      if n <= 1 { return 1; }
      return fib(n - 1) + fib(n - 2);
}

print fib(9);

// Output: 34

```


For this project I took inspiration from these projects:

- [GitHub - google/closure-compiler: A JavaScript checker and optimizer.](https://github.com/google/closure-compiler)

- [GitHub - wren-lang/wren: The Wren Programming.](https://github.com/wren-lang/wren)



### Features

- **Global and local variables**

- **If statements**

- **While loop**

- **Functions**

- **Print statement**

- **All classical binary operations + exponentiation**



### Data Types

- **Nil**

- **Booleans**

- **Strings**

- **Numbers**



> ![WARNING]
> 
> This language don't have structs or similar construcuts and don't support closures.

## Grammar

```
declaration ::= function-decl
            | variable-decl

variable-decl ::= 'let' IDENTIFIER '=' expression ';'

function-parameters ::= '(' IDENTIFIER (',' IDENTIFIER)* ')'
function-decl ::= 'defun' IDENTIFIER function-parameters block

block ::= '{' declaration* '}'

statement ::= if-statement
          | expression-statement
          | while-statement
          | for-statement
          | break-statement
          | continue-statement
          | return-statement
          | block

print-statement ::= 'print' expression ';'
if-statement ::= 'if' expression block ('else' block)*
while-statement ::= 'while' expression block
break-statement ::= 'break' ';'
continue-statement ::= 'continue' ';'
return-statement ::= 'return' expression? ';'

expression-statement ::= expression ';'

expression ::= assignment 
assignment ::= (call '.')? IDENTIFIER '=' assignment | logic-or
logic-or ::= logic-and ('or' logic-and)*
logic-and ::= equality ('and' equality)*
equality ::= comparison (('==' | '!=') comparison)*
comparison ::= term (('>' | '<' | '<=' | '>=') term)*
term ::= factor (('+' | '-' factor)*
factor ::= unary (('*' | '/' unary)*
unary ::= ('-' | '+' | 'not') unary | exponent
exponent ::= call ('**' factor)*

call-arguments ::= expression (',' expression)*
call ::= primary ( '(' call-arguments+ ')' | '.' IDENTIFIER)?
primary ::= NUMBER_LITERAL 
        | STRING_LITERAL 
        | IDENTIFIER
        | 'true'
        | 'false'
        | 'nil'
        | '(' expression ')'

```

# 




















