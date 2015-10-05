// StatementParser.cpp
// Code for parsing a mathematical expression for use in Thematic Maps and elsewhere
// Created from scratch 12/6/01 by Gary R. Huber
// Copyright 2001 by 3D Nature LLC

// the goal is to take a valid, ballanced math expression involving multiple inputs or constants
// and multiple sequential mathematical operators and derive a single floating point numeric result

// Input variables may come from database attributes or tables and must be user selectable

// there will be a way for the user to define the expression besides just typing

// typical expressions:
/*
		a * x + b
		(a(x) / (b + c)) * d + e

Operator - Add
	Input1 - Constant
	Input2 - Operator
		Operator - Multiply
			Input1 - Variable
			Input2 - Constant
*/

#include "stdafx.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "StatementParser.h"


double DivideOperator::Operate(void)
{
double V2;

if (Input1 && Input2)
	{
	V2 = Input2->Operate();
	if (V2 != 0.0)
		Value = Input1->Operate() / V2;
	else
		Value = 0.0;
	} // if
else
	Value = 0.0;
return (Value);

} // DivideOperator::Operate

/*===========================================================================*/

double SquareOperator::Operate(void)
{

Value = (Input1) ? Input1->Operate(): 0.0;
Value *= Value;
return (Value);

} // SquareOperator::Operate

/*===========================================================================*/

double SquareRootOperator::Operate(void)
{

Value = (Input1) ? Input1->Operate(): 0.0;
return (Value = (Value > 0.0 ? sqrt(Input1->Operate()): 0.0));

} // SquareRootOperator::Operate

/*===========================================================================*/

double ExponentOperator::Operate(void)
{

return (Value = (Input1) ? exp(Input1->Operate()): 0.0);

} // ExponentOperator::Operate

/*===========================================================================*/

double NaturalLogOperator::Operate(void)
{

Value = (Input1) ? Input1->Operate(): 0.0;
return (Value = (Value > 0.0) ? log(Value): 0.0);

} // NaturalLogOperator::Operate

/*===========================================================================*/

double Variable::Operate(void)
{
double TMValues[3], Lat = 0.0, Lon = 0.0;	// lat/lon need to be passed somehow

if (ValueSource)
	{
	if (VariableType == WCS_EXPRESSION_VARTYPE_THEMATICMAP)
		{
		// need some lat/lon data to evaluate
		return (Value = ((ThematicMap *)ValueSource)->Eval(TMValues, NULL));
		} // if thematic map
	else if (VariableType == WCS_EXPRESSION_VARTYPE_ANIMDOUBLE)
		{
		return (Value = ((AnimDouble *)ValueSource)->GetValue(0, Input1 ? Input1->Operate(): 0.0));
		} // else if
	} // if
return (0.0);

} // Variable::Operate

/*===========================================================================*/

Operator::~Operator()
{

if (Input1)
	delete Input1;
if (Input2)
	delete Input2;

} // Operator::~Operator

/*===========================================================================*/

Expression::~Expression()
{

if (OpChain)
	delete OpChain;

} // Expression::~Expression

/*===========================================================================*/

int Expression::CreateChain(char *InputStr)
{

if (OpChain)
	delete OpChain;
OpChain = NULL;

if (BalanceParentheses(InputStr))
	OpChain = ParseSymbolicString(InputStr);

return (OpChain ? 1: 0);

} // Expression::CreateChain

/*===========================================================================*/

Operator *Expression::ParseSymbolicString(char *&InputStr)
{
double ConstValue;
Operator *RootOp = NULL, *TempRoot, **NextOp;
char *NewVarName = NULL;
int MathOp, NewVarLen;

if (InputStr && InputStr[0])
	{
	NextOp = &RootOp;
	while (InputStr[0])
		{
		if (IDCloseParenthesis(InputStr))
			{
			return (RootOp);
			} // if
		else if (IDOpenParenthesis(InputStr))
			{
			*NextOp = ParseSymbolicString(InputStr);
			} // else if
		else if (MathOp = IDMathOp(InputStr))
			{
			TempRoot = RootOp;
			switch (MathOp)
				{
				case WCS_EXPRESSION_OPERATOR_ADD:
					{
					if (RootOp = new AddOperator())
						{
						RootOp->Input1 = TempRoot;
						NextOp = &RootOp->Input2;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_SUBTRACT:
					{
					if (RootOp = new SubtractOperator())
						{
						RootOp->Input1 = TempRoot;
						NextOp = &RootOp->Input2;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_MULTIPLY:
					{
					if (RootOp = new MultiplyOperator())
						{
						RootOp->Input1 = TempRoot;
						NextOp = &RootOp->Input2;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_DIVIDE:
					{
					if (RootOp = new DivideOperator())
						{
						RootOp->Input1 = TempRoot;
						NextOp = &RootOp->Input2;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_SQUARE:
					{
					if (*NextOp = new SquareOperator())
						{
						NextOp = &(*NextOp)->Input1;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_SQUAREROOT:
					{
					if (*NextOp = new SquareRootOperator())
						{
						NextOp = &(*NextOp)->Input1;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_EXPONENT:
					{
					if (*NextOp = new ExponentOperator())
						{
						NextOp = &(*NextOp)->Input1;
						} // if
					break;
					} // add
				case WCS_EXPRESSION_OPERATOR_NATURALLOG:
					{
					if (*NextOp = new NaturalLogOperator())
						{
						NextOp = &(*NextOp)->Input1;
						} // if
					break;
					} // add
				default:
					break;
				} // switch
			} // else if
		else if (IDConstant(InputStr, ConstValue))
			{
			if (*NextOp = new Constant())
				{
				(*NextOp)->Value = ConstValue;
				NextOp = &RootOp;
				} // if
			} // else if
		else if (IDVariable(InputStr, NewVarName, NewVarLen))
			{
			if (*NextOp = new Variable())
				{
				NewVarLen = max(NewVarLen, 31);
				strncpy(((Variable *)(*NextOp))->VariableName, NewVarName, NewVarLen);
				((Variable *)(*NextOp))->VariableName[NewVarLen] = 0;
				NextOp = &RootOp;
				} // if
			} // else if
		else
			{
			// this is an error condition
			break;
			} // else
		} // while
	} // if

return (RootOp);

} // Expression::ParseSymbolicString

/*===========================================================================*/

int Expression::BalanceParentheses(char *IDStr)
{
int OpenPar = 0, ClosePar = 0;	// count open and close parentheses

while (IDStr[0])
	{
	if (IDStr[0] == '(' || IDStr[0] == '[' || IDStr[0] == '{')
		{
		OpenPar ++;
		} // if
	if (IDStr[0] == ')' || IDStr[0] == ']' || IDStr[0] == '}')
		{
		ClosePar ++;
		} // if
	IDStr ++;
	} // while

return (OpenPar == ClosePar);

} // Expression::BalanceParentheses

/*===========================================================================*/

int Expression::IDOpenParenthesis(char *&IDStr)
{
// if string is numeric

while (IDStr[0] == ' ')
	IDStr ++;

if (IDStr[0])
	{
	if (IDStr[0] == '(' || IDStr[0] == '[' || IDStr[0] == '{')
		{
		IDStr ++;
		return (1);
		} // if
	} // if

return (0);

} // Expression::IDOpenParenthesis

/*===========================================================================*/

int Expression::IDCloseParenthesis(char *&IDStr)
{
// if string is numeric

while (IDStr[0] == ' ')
	IDStr ++;

if (IDStr[0])
	{
	if (IDStr[0] == ')' || IDStr[0] == ']' || IDStr[0] == '}')
		{
		IDStr ++;
		return (1);
		} // if
	} // if

return (0);

} // Expression::IDCloseParenthesis

/*===========================================================================*/

int Expression::IDMathOp(char *&IDStr)
{
// if string is +, -, *, /, exp, log, sq, sqrt

while (IDStr[0] == ' ')
	IDStr ++;

if (IDStr[0])
	{
	switch (IDStr[0])
		{
		case '+':
			{
			IDStr ++;
			return (WCS_EXPRESSION_OPERATOR_ADD);
			} // add
		case '-':
			{
			IDStr ++;
			return (WCS_EXPRESSION_OPERATOR_SUBTRACT);
			} // subtract
		case '*':
			{
			IDStr ++;
			return (WCS_EXPRESSION_OPERATOR_MULTIPLY);
			} // multiply
		case '/':
			{
			IDStr ++;
			return (WCS_EXPRESSION_OPERATOR_DIVIDE);
			} // divide
		case 's':
		case 'S':
			{
			if (IDStr[1] == 'q' || IDStr[1] == 'Q')
				{
				if ((IDStr[2] == 'r' || IDStr[2] == 'R') && (IDStr[3] == 't' || IDStr[3] == 'T') && (IDStr[4] == '(' || IDStr[4] == ' '))
					{
					IDStr += 4;
					return (WCS_EXPRESSION_OPERATOR_SQUAREROOT);
					} // if
				else if ((IDStr[2] == '(' || IDStr[2] == ' '))
					{
					IDStr += 2;
					return (WCS_EXPRESSION_OPERATOR_SQUARE);
					} // else if
				} // if
			break;
			} // add
		case 'e':
		case 'E':
			{
			if ((IDStr[1] == 'x' || IDStr[1] == 'X') && (IDStr[2] == 'p' || IDStr[2] == 'P') && (IDStr[3] == '(' || IDStr[3] == ' '))
				{
				IDStr += 3;
				return (WCS_EXPRESSION_OPERATOR_SQUAREROOT);
				} // if
			break;
			} // exponent
		case 'l':
		case 'L':
			{
			if ((IDStr[1] == 'o' || IDStr[1] == 'O') && (IDStr[2] == 'g' || IDStr[2] == 'G') && (IDStr[3] == '(' || IDStr[3] == ' '))
				{
				IDStr += 3;
				return (WCS_EXPRESSION_OPERATOR_NATURALLOG);
				} // if
			break;
			} // log
		default:
			break;
		} // switch
	} // if

return (0);

} // Expression::IDMathOp

/*===========================================================================*/

int Expression::IDConstant(char *&IDStr, double &dValue)
{
// if string is numeric
char *StrStash;
char StrCopy[256];

while (IDStr[0] == ' ')
	IDStr ++;

StrStash = IDStr;

if (IDStr[0])
	{
	while (isdigit(IDStr[0]) || IDStr[0] == '.')
		IDStr ++;
	} // if

if (IDStr > StrStash)
	{
	strncpy(StrCopy, StrStash, IDStr - StrStash);
	StrCopy[IDStr - StrStash] = 0;
	dValue = atof(StrStash);
	return (1);
	} // if
return (0);

} // Expression::IDConstant

/*===========================================================================*/

int Expression::IDVariable(char *&IDStr, char *VarName, int &VarLen)
{
char *StrStash;

// if string is not a math operator or constant

while (IDStr[0] == ' ')
	IDStr ++;

StrStash = IDStr;

if (IDStr[0])
	{
	while (IDStr[0] != ' ' && IDStr[0] != '+' && IDStr[0] != '-' && IDStr[0] != '*' && IDStr[0] != '/')
		IDStr ++;
	} // if

if (IDStr > StrStash)
	{
	VarLen = IDStr - StrStash;
	VarName = StrStash;
	return (1);
	} // if
return (0);

} // Expression::IDVariable

/*===========================================================================*/

double Expression::Evaluate(void)
{

if (OpChain)
	{
	OpChain->Operate();
	return (OpChain->Value);
	} // if

return (0.0);

} // Expression::Evaluate
