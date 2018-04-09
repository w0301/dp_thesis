lexer grammar LangLexer;

fragment WHITESPACE
  : [ \t]+
  ;

INTEGER
  : [1-9] [0-9]*
  | [0]
  | WHITESPACE [\-] [1-9] [0-9]*
  ;

FLOAT
  : [0-9]+ '.' [0-9]+
  | WHITESPACE [\-] [0-9]+ '.' [0-9]+
  ;

BOOL
  : ('true' | 'false')
  ;

CHAR
  : '\'' . '\''
  ;

STRING
  : '"' .*? '"'
  ;

OBJ
  : 'null'
  ;

NAME
  :  [a-zA-Z] [a-zA-Z0-9]*
  ;

DOT
  : '.'
  ;

OBJ_PATH
  : NAME (DOT NAME)*
  ;

EQ
  : '='
  ;

COMMA
  : ','
  ;

COLON
  : ':'
  ;

L_PAREN
  : '('
  ;

R_PAREN
  : ')'
  ;

GLOBAL
  : 'global'
  ;

LOCAL
  : 'local'
  ;

DEF
  : 'def'
  ;

IF
  : 'if'
  ;

ELSE
  : 'else'
  ;

RETURN
  : 'return'
  ;

END
  : 'end'
  ;

WHITESPACE_SKIP
  : [\r\n \t] -> channel(HIDDEN)
  ;
