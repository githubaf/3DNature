// StatementParser.cpp
// Code for parsing a mathematical expression for use in Thematic Maps and elsewhere
// Created from scratch 12/6/01 by Gary R. Huber
// Copyright 2001 by 3D Nature LLC


class Operator
	{
	public:
		double Value;
		Operator *Input1, *Input2;

		Operator()	{Value = 0.0; Input1 = Input2 = NULL;};
		~Operator();
		virtual double Operate(void) = 0;

	}; // class Operator

class Expression
	{
	public:
		Operator *OpChain;

		Expression()	{OpChain = NULL;};
		~Expression();
		int CreateChain(char *InputStr);
		double Evaluate(void);
		Operator *ParseSymbolicString(char *&InputStr);
		int BalanceParentheses(char *IDStr);
		int IDOpenParenthesis(char *&IDStr);
		int IDCloseParenthesis(char *&IDStr);
		int IDMathOp(char *&IDStr);
		int IDConstant(char *&IDStr, double &dValue);
		int IDVariable(char *&IDStr, char *VarName, int &VarLen);

	}; // class Expression

enum
	{
	WCS_EXPRESSION_OPERATOR_ADD = 1,
	WCS_EXPRESSION_OPERATOR_SUBTRACT,
	WCS_EXPRESSION_OPERATOR_MULTIPLY,
	WCS_EXPRESSION_OPERATOR_DIVIDE,
	WCS_EXPRESSION_OPERATOR_SQUARE,
	WCS_EXPRESSION_OPERATOR_SQUAREROOT,
	WCS_EXPRESSION_OPERATOR_EXPONENT,
	WCS_EXPRESSION_OPERATOR_NATURALLOG
	}; // math operators

class AddOperator : public Operator
	{
	public:

		virtual double Operate(void)
			{return (Value = (Input1 ? Input1->Operate(): 0.0) + (Input2 ? Input2->Operate(): 0.0));};

	}; // class Operator

class SubtractOperator : public Operator
	{
	public:

		virtual double Operate(void)
			{return (Value = (Input1 ? Input1->Operate(): 0.0) - (Input2 ? Input2->Operate(): 0.0));};

	}; // class SubtractOperator

class MultiplyOperator : public Operator
	{
	public:

		virtual double Operate(void)
			{return (Value = (Input1 && Input2) ? Input1->Operate() * Input2->Operate(): 0.0);};

	}; // class MultiplyOperator

class DivideOperator : public Operator
	{
	public:

		virtual double Operate(void);

	}; // class DivideOperator

class SquareOperator : public Operator
	{
	public:

		virtual double Operate(void);

	}; // class SquareOperator

class SquareRootOperator : public Operator
	{
	public:

		virtual double Operate(void);

	}; // class SquareRootOperator

class ExponentOperator : public Operator
	{
	public:

		virtual double Operate(void);

	}; // class ExponentOperator

class NaturalLogOperator : public Operator
	{
	public:

		virtual double Operate(void);

	}; // class NaturalLogOperator

enum
	{
	WCS_EXPRESSION_VARTYPE_THEMATICMAP,
	WCS_EXPRESSION_VARTYPE_ANIMDOUBLE
	}; // variable types for expressions

class Variable : public Operator
	{
	public:
		void *ValueSource;	// could be a thematic map or AnimDouble
		char VariableType, VariableName[32];

		Variable()	{ValueSource = NULL; VariableType = WCS_EXPRESSION_VARTYPE_THEMATICMAP;};
		virtual double Operate(void);

	}; // class Variable

class Table : public Operator
	{
	public:
		double *Data;
		char Dimensions;

		Table()	{Data = NULL; Dimensions = 1;};
		virtual double Operate(void)
			{return (Value = Data ? Lookup(Input1 ? Input1->Operate(): 0.0, Input2 ? Input2->Operate(): 0.0): 0.0);};
		double Lookup(double A, double B);

	}; // class Table

class Constant : public Operator
	{
	public:
		virtual double Operate(void)	{return (Value);};

	}; // class Constant


