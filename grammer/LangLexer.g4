lexer grammar LangLexer;

INTEGER
  : [1-9] [0-9]*
  | [0]
  | [\-] [1-9] [0-9]*
  ;

FLOAT
  : [0-9]+ '.' [0-9]+
  | [\-] [0-9]+ '.' [0-9]+
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

EQ
  : '='
  ;

DOT
  : '.'
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

// identifier tokens comes last!
NAME
  :  [a-zA-Z_] [a-zA-Z0-9_]*
  ;

GLOBAL_OBJ_PATH
  : 'global' (DOT NAME)*
  ;

LOCAL_OBJ_PATH
  : 'local' (DOT NAME)*
  ;

// skipping whitespaces
WHITESPACE_SKIP
  : [ \t\r\n]+ -> channel(HIDDEN)
  ;
