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

value
  : constantValue
  | identifier
  ;

functionName
  : NAME
  ;

functionArgName
  : NAME
  ;

functionArg
  : value
  ;

functionCall
  : functionName L_PAREN (functionArg (COMMA functionArg)*)? R_PAREN
  ;

assignmentStatement
  : identifier EQ constantValue
  | identifier EQ functionCall
  ;

conditionStatement
  : IF identifier thenStatements END
  | IF identifier thenStatements ELSE statement
  | IF identifier thenStatements ELSE elseStatements END
  ;

returnStatement
  : RETURN value
  ;

statement
  : assignmentStatement
  | conditionStatement
  | returnStatement
  ;

thenStatements
  : statement+
  ;

elseStatements
  : statement+
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