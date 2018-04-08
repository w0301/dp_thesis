lexer grammar LangLexer;

fragment WHITESPACE
  : [ \t]+
  ;

INTEGER
  : [1-9] [0-9]*
  | [0]
  | WHITESPACE [\-] [1-9] [0-9]*
  ;
