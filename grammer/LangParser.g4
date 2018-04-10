parser grammar LangParser;

options {
	tokenVocab = LangLexer;
}

integerValue
  : INTEGER
  ;

floatValue
  : FLOAT
  ;

boolValue
  : BOOL
  ;

charValue
  : CHAR
  ;

stringValue
  : STRING
  ;

constantValue
  : integerValue
  | floatValue
  | boolValue
  | charValue
  | stringValue
  ;

globalIdentifier
  : GLOBAL_OBJ_PATH
  ;

localIdentifier
  : LOCAL_OBJ_PATH
  ;

identifier
  : localIdentifier
  | globalIdentifier
  ;

functionName
  : NAME
  ;

functionArgName
  : NAME
  ;

functionArg
  : identifier
  | constantValue
  ;

functionCall
  : functionName L_PAREN (functionArg (COMMA functionArg)*)? R_PAREN
  ;

assignmentStatement
  : identifier EQ constantValue
  | identifier EQ functionCall
  ;

conditionStatement
  : IF identifier statements END
  | IF identifier statements ELSE statement
  | IF identifier statements ELSE statements END
  ;

returnStatement
  : RETURN identifier
  ;

statement
  : assignmentStatement
  | conditionStatement
  | returnStatement
  ;

statements
  : statement+
  ;

function
  : DEF functionName L_PAREN (functionArgName (COMMA functionArgName)*)? R_PAREN statements END
  ;

file
  : function*
  ;