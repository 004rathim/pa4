
#include "ast/ConstantType.h"
#include "ast/VariableType.h"
#include "ast/FunctionType.h"
#include "ast/expression.h"
#include "TypeInference.h"
#include <stdlib.h>
#include <stdio.h>
#include <map>

SymbolTable sym;
vector<Expression*> lambIDs;
vector<Expression*> lambAssign;
std::map<AstIdentifier*,Type*> functionReturnTypes;
TypeInference::TypeInference(Expression* e)
{
	this->program = e;
	lambIDs = vector<Expression*>();
	lambAssign = vector<Expression*>();
	sym.push();	
	Type* value = eval(program);
	cout << value->to_string() << endl;
}

Type* TypeInference::eval(Expression* e)	// eval function is overloaded to store function types
{
	return eval(e, NULL);
}
Type* TypeInference::eval(Expression* e, AstIdentifier* astIdentifierParameter) // the second parameter is the function identifier, we understandd
{
	ConstantType *integer =  ConstantType::make("Int");
	ConstantType *string =  ConstantType::make("String");
	ConstantType *list =  ConstantType::make("List");
	ConstantType *nil = ConstantType::make("Nil");
	ConstantType *lambda = ConstantType::make("Lambda");
	expression_type etype = e->get_type();
	
	// Base Cases
	if(etype == AST_NIL)	// If expression is nil return nil
	{
		return nil; 
	}
	if(etype == AST_INT){	// If expression is int return int
		return integer;
	}
	if(etype == AST_STRING){	// If expression is string return string
		return string;
	}
	
	if(etype == AST_IDENTIFIER){
		AstIdentifier* t = static_cast<AstIdentifier*>(e);
		Expression* id = sym.find(t);	// Loop up the value of the identifier from the symbol table
		if(id != NULL)					// If it is on the table then evaluate it 
		{
			Type* idType = eval(id, astIdentifierParameter);
			return idType;
		}
		return VariableType::make(t->to_string().substr(0, t->to_string().size()-1) );	// If not on the table make a new variable type
	}

	if(etype == AST_READ)			// READ_INT returns integer, READ_STRING returns string
	{
		AstRead *r = static_cast<AstRead*>(e);
		if(r->read_integer())
			return integer;
		else
			return string;
	}

	if(etype == AST_LET)			
	{
		AstLet *let = static_cast<AstLet*>(e);
		AstIdentifier *id = let->get_id();			// Get the identifier of let
		Expression *val = let->get_val();			// Get the value of let
		Expression *body = let->get_body();			// Get body of the let
		
		Type *letIdType = eval(val, astIdentifierParameter);

		if(letIdType == integer)		// add identifier and its type to the symbol table
		{
			AstInt* temp = AstInt::make(1);
			sym.add(id, temp);
		}
		else
		if(letIdType == string)			// add identifier and its type to the symbol table
		{
			AstString* temp = AstString::make("1");
			sym.add(id, temp);
		}
		else
		if(letIdType == list)			// add identifier and its type to the symbol table
		{
			ConstantType *temp = static_cast<ConstantType*> (letIdType);
			AstList* tempList = static_cast<AstList*> (AstList::make(temp->listInfo->head, temp->listInfo->tail) );
			sym.add(id, tempList);
		}
		else
		if(letIdType == lambda)			// add Lambda expression to the symbol table
		{
			ConstantType *temp = static_cast<ConstantType*> (letIdType); 
			AstLambda* tempLambda = AstLambda::make(  temp->formal , temp->body );
			sym.add(id, tempLambda);
		}
		else
		if(letIdType == nil)			// // add identifier and its type to the symbol table
		{
			AstNil* temp = AstNil::make();
			sym.add(id, temp);
		} 
		else
			assert(false);				// Something went wrong

		Type *letBodyType = eval(body, astIdentifierParameter);
		

		return letBodyType;	
	}
	
if(etype == AST_EXPRESSION_LIST)		// evaluate applications
	{
		vector<Expression*> expList = static_cast<AstExpressionList*>(e)->get_expressions();
		
		if(expList.size() == 1)			// Base case: if exp list has one element evaluate it
			return eval(expList[0], astIdentifierParameter);
		AstIdentifier *functionID = NULL;		// initial value (NULL) of function id, if it is not NULL then we are trying to find the function's type
		AstLambda* lambda;
		if(expList[0] -> get_type() == AST_IDENTIFIER)		// if first element is identifier, it is either function or let&lambda 
		{
			functionID = static_cast<AstIdentifier*>(expList[0]);			// get the function id if available
			if( functionReturnTypes.find(functionID) != functionReturnTypes.end() )	// if it is in the map, retrieve its value
	 		{	
	 			ConstantType* functionType = static_cast<ConstantType*>(functionReturnTypes.find(functionID)->second);
	 			return functionType;
	 		}
 			lambda = static_cast<AstLambda*>(sym.find(static_cast<AstIdentifier*>(expList[0]) ));	// if it is not in the function->type map, then look up it on symbol table for let lambda expression
		}
		else		
			lambda = static_cast<AstLambda*>(expList[0]);		// otherwise it is a lambda expression application
 		
 		AstIdentifier *id = lambda->get_formal();					// get lambda id and body
		Expression* lambdaBody = lambda->get_body();
		Expression* lambdasubbed =  lambdaBody->substitute(id, expList[1]);	// substitute value with the identifier Exp[x/var]

		vector<Expression*> expList2;
		expList2.push_back(lambdasubbed);									// push substituted lambda application to a new expression list 
		for(unsigned int c = 2; c < expList.size(); c++)
			expList2.push_back(expList[c]);									// push rest of the elements to the list
 		return eval(AstExpressionList::make(expList2), functionID);			// evaluate the expression
	}

	if(etype == AST_LAMBDA)				// Lambda evaluates to Constant Type Lambda, if it is not an application do not evaluate the body
	{
		AstLambda* lambda2 = static_cast<AstLambda*>(e);
		lambda->formal = lambda2->get_formal();
		lambda->body = lambda2->get_body();
		
		return lambda;
	}

	// Binary Operations
	if(etype == AST_BINOP)
	{
		AstBinOp *bin = static_cast<AstBinOp*>(e);			// Get binop type, first and second expressions
		binop_type btype = bin->get_binop_type();
		Expression *first = bin->get_first();
		Type *firstType = eval(first,astIdentifierParameter);
		Expression *second = bin->get_second();
		Type *secondType = eval(second, astIdentifierParameter);
 
		if( btype ==  PLUS)   // accepts integers and strings
		{					
			if(firstType == integer && secondType == integer)
				return integer;
			else
			if(firstType == string && secondType == string)
				return string;
			else
				assert(btype !=  PLUS);		
		}
		else
		if( btype ==  MINUS) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  MINUS);
		}
        else
		if( btype ==  TIMES) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  TIMES);
		}
        else
		if( btype ==  DIVIDE) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  DIVIDE);
		}
		else
		if( btype ==  AND) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  AND);
		}
		else
		if( btype ==  OR) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  OR);
		}
		else		
		if( btype ==  EQ)   // accepts integers and strings
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
			if(firstType == string && secondType == string)
				return string;
			else
				assert(btype !=  EQ);
		}
		else		
		if( btype ==  NEQ)   // accepts integers and strings
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
			if(firstType == string && secondType == string)
				return string;
			else
				assert(btype !=  NEQ);
		}
		else
		if( btype ==  LT) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  LT);
		}
		else
		if( btype ==  LEQ) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  LEQ);
		}
		else
		if( btype ==  GT) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  GT);
		}
		else
		if( btype ==  GEQ) // accepts only integers
		{	
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(btype !=  GEQ);
		}
		else
		if( btype ==  CONS) // accepts integer, strings and lists -- WE ALLOW POLYMORPHIC LISTS !!! :)
		{				
			list->listInfo = new ConstantType::node; // allocate memory for the list data structure 
			list->listInfo->head = bin->get_first();	// get head of the list and store it
			list->listInfo->tail = bin->get_second();	// get tail of the list and store it
			
			if(secondType == nil)	// if second element is nil (2@nil) return the first element
				return firstType;
			return list;		
		}
	}
	
	// Unary Operations
	if(etype == AST_UNOP)
	{	
		AstUnOp *un = static_cast<AstUnOp*>(e);				// get the type and expression of unop
		unop_type utype = un->get_unop_type();
		Expression *exp = un->get_expression();
		Type *type = eval(exp,astIdentifierParameter);

		if(utype == ISNIL)									// isnil returns an integer
		{
			return integer;
		}
		else
		if(utype == PRINT)									// print returns an integer
		{
			return integer;
		}
		else
		if(utype == HD)										// get head of a list
		{
			if(type != list)	// if ! operation used anything other than list return itself
				return type;
			
			ConstantType *tempList = static_cast<ConstantType*>(type);	
			return eval(tempList->listInfo->head, astIdentifierParameter);	// return head of the list
		}
		else
		if(utype == TL)				// get tail of a list
		{
			if(type != list)		// if # operation used anything other than list return nil 
				return nil;
	
			ConstantType *tempList = static_cast<ConstantType*>(type);		
			return eval(tempList->listInfo->tail,astIdentifierParameter);	// return tail of the list
		}
	}
	
	// Conditional
	if( etype == AST_BRANCH)
	{
		AstBranch *conditional = static_cast<AstBranch*>(e);
		Type *predicate = eval(conditional->get_pred(), astIdentifierParameter);
		
		if(predicate != integer) // predicate can only be an integer
			assert(predicate == integer);

		Type *exp1 = eval(conditional->get_then_exp(), astIdentifierParameter);
		if(exp1 == nil)		// then and else should be the same type
			assert(exp1 != nil);

		if(astIdentifierParameter != NULL &&  functionReturnTypes.find(astIdentifierParameter) == functionReturnTypes.end() ) // 
			functionReturnTypes.insert(std::make_pair(astIdentifierParameter, exp1)); 		
		Type *exp2 = eval(conditional->get_else_exp(), astIdentifierParameter);
		
		if(exp1 != exp2)		// then and else should be same type
			assert(exp1 == exp2);
		
		return exp1;
	}
}
