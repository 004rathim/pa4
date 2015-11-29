
#include "ast/ConstantType.h"
#include "ast/VariableType.h"
#include "ast/FunctionType.h"
#include "ast/expression.h"
#include "TypeInference.h"

SymbolTable sym;
vector<Expression*> lambIDs;
vector<Expression*> lambAssign;

TypeInference::TypeInference(Expression* e)
{
	this->program = e;
	lambIDs = vector<Expression*>();
	lambAssign = vector<Expression*>();
	sym.push();	
	Type* value = eval(program);
	cout << value->to_string() << endl;

}

//  enum expression_type {AST_BINOP*,  AST_IDENTIFIER*,
//  AST_INT*, AST_LAMBDA*, AST_LET*, AST_STRING*, AST_IDENTIFIER_LIST,
//  AST_EXPRESSION_LIST*, AST_BRANCH*, AST_NIL*, AST_LIST*, AST_UNOP*, AST_READ*};


// Ast Nodes:
// AstBinOp
//      case PLUS: return "+";  **DONE
//      case MINUS: return "-"; **DONE
//      case TIMES: return "*"; **DONE
//      case DIVIDE: return "/";**DONE
//      case AND: return "&"; **DONE
//      case OR: return "|"; **DONE
//      case EQ: return "="; **DONE
//      case NEQ: return "<>"; **DONE
//      case LT: return "<"; **DONE
//      case LEQ: return "<="; **DONE
//      case GT: return ">"; **DONE
//      case GEQ: return ">="; **DONE
//      case CONS: return "@";
// AstBranch **DONE
// AstExpressionList
// AstIdentifierE
// AstIdentifierList
// AstInt **DONE
// AstLambda
// AstLet
// AstList
// AstNil
// AstRead
// AstString**DONE
// AstUnOp
//		case !:
//		case #:
//		case IsNil:
//		case print:
// Expression

Type* TypeInference::eval(Expression* e)
{
//TODO: need to look over my use of sym push and pop
	ConstantType *integer =  ConstantType::make("Int");
	ConstantType *string =  ConstantType::make("String");
	ConstantType *listInteger =  ConstantType::make("ListInt");
	ConstantType *listString =  ConstantType::make("ListString");
	expression_type etype = e->get_type();

	// Base Cases
	if(e->get_type() == AST_NIL)
	{
		return integer; //TODO: is this correct?
	}
	if(e->get_type()== AST_INT){
		return integer;
	}
	if(e->get_type()== AST_STRING){
		return string;
	}
	if(e->get_type()== AST_IDENTIFIER){
		AstIdentifier* t = static_cast<AstIdentifier*>(e);
		Expression* id = sym.find(t);
		if(id != NULL)
		{
			Type* idType = eval(id);
			return idType;
		}
		return VariableType::make("x");
	}

	if(etype == AST_READ)
	{
		AstRead *r = static_cast<AstRead*>(e);
		if(r->read_integer())
			return integer;
		else
			return string;
	}

	if(etype == AST_LET)
	{
		AstLet *l = static_cast<AstLet*>(e);
		//TODO: not sure about the lambda part
		if(l->get_val()->get_type() == AST_LAMBDA)
		{
			Type *letType = eval(l->get_body());
			if(letType == integer)
			{
				AstInt* temp = AstInt::make(1);
				sym.add(l->get_id(), temp);
			}	
			else
			{
				AstString* temp = AstString::make("1");
				sym.add(l->get_id(), temp);
			}
			Type *bodyType = eval(l->get_val());
			return bodyType;
		}
		else
		{
			Type *letType = eval(l->get_val());
			if(letType == integer)
			{
				AstInt* temp = AstInt::make(1);
				sym.add(l->get_id(), temp);
			}
			else
			{
				AstString* temp = AstString::make("1");
				sym.add(l->get_id(), temp);
			}
			
			Type *bodyType = eval(l->get_body());
	//		if(bodyType == letType) //TODO: shouldn't check this, right?
				return bodyType;
	//		else
	//			assert(bodyType == letType);
		}
	}

	if(etype == AST_EXPRESSION_LIST)
	{
		AstExpressionList* list = static_cast<AstExpressionList*>(e);
		vector<Expression*> vect = list->get_expressions();

		if(lambIDs.size()+1 < vect.size())
		{
			lambAssign.push_back(vect[lambIDs.size()+1]);
		}
		
		Type* lambType = eval(vect[0]);
		return lambType;
	}

	if(etype == AST_LAMBDA)
	{
		AstLambda* lambda = static_cast<AstLambda*>(e);
		lambIDs.push_back(lambda->get_formal());
		AstIdentifier* temp;

		if(lambda->get_formal()->get_type() == AST_IDENTIFIER)
		{
			temp = static_cast<AstIdentifier*>(lambda->get_formal());
		}

		if(lambAssign.size() > 0 || lambIDs.size() > 0)
		{
			Expression* exp = lambAssign.back();
			sym.add(temp, exp);
			Type *lambType = eval(lambda->get_body());
			return lambType;
		}
		else
		{
			vector<Type*> val;
			val.push_back(VariableType::make("x"));
			Type *func = FunctionType::make("Lambda", val);
			return func;
		}
	}

	// Binary Operations
	if(etype == AST_BINOP)
	{
		AstBinOp *bin = static_cast<AstBinOp*>(e);
		binop_type btype = bin->get_binop_type();
		Expression *first = bin->get_first();
		Type *firstType = eval(first);
		Expression *second = bin->get_second();
		Type *secondType = eval(second);

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
		if( btype ==  CONS) // accepts only integers (and strings? -MR)
		{	
			if( firstType == integer && secondType == integer )	// if both int then return a list int
			{	
				listInteger->head = false;
				listInteger->tail = false;
				return listInteger;
			}
			else
			if( firstType == integer && secondType == listInteger )	// if one of them are listInt then set the corresponding boolean true
			{	
				listInteger->head = false;
				listInteger->tail = true;
				return listInteger;
			}
			else
			if( firstType == listInteger && secondType == integer )
			{	
				listInteger->head = true;
				listInteger->tail = false;
				return listInteger;
			}
			else
			if( firstType == listInteger && secondType == listInteger )
			{	
				listInteger->head = true;
				listInteger->tail = true;
				return listInteger;
			}
			else
			if( firstType == string && secondType == string )		// do same for strings
			{	
				listString->head = false;
				listString->tail = false;
				return listString;
			}
			else
			if( firstType == string && secondType == listString )
			{	
				listString->head = false;
				listString->tail = true;
				return listString;
			}
			else
			if( firstType == listString && secondType == string )
			{	
				listString->head = true;
				listString->tail = false;
				return listString;
			}
			else
			if( firstType == listString && secondType == listString )
			{	
				listString->head = true;
				listString->tail = true;
				return listString;
			}
			else
				assert(btype !=  CONS);
		}

	}
	
	// Unary Operations
	if(etype == AST_UNOP)
	{	
		AstUnOp *un = static_cast<AstUnOp*>(e);
		unop_type utype = un->get_unop_type();
		Expression *exp = un->get_expression();
		Type *type = eval(exp);

		if(utype == ISNIL)
		{
			return integer;
		}
		else
		if(utype == PRINT)
		{
			return integer;
		}
		else
		if(utype == HD)
		{
			cout << type->to_string() << endl;
			if(type == listInteger || type == listString)
			{
				//TODO: seg fault on listHead?
				AstList* list = static_cast<AstList*>(exp);
				Expression* listHead = list->get_hd();
				cout << listHead->to_value() << endl;
				Type* headType = eval(listHead);
				return headType;
			}
			else
			if(type == integer || string)
			{
				return type;
			}
			else
				assert(utype != HD);
		}
		else
		if(utype == TL)
		{
			if(type == listInteger || type == listString)
			{
				//TODO: get_tl returning wrong value
				AstList* list = static_cast<AstList*>(exp);
				Expression* listTail = list->get_tl();
				cout << listTail->to_value() << endl;
				Type* tailType = eval(listTail);
				return tailType;
			}
			else
			if(type == integer || string)
			{
				return eval(AstNil::make()); 
			}
			else
				assert(utype != TL);
		}

	}
	
	// Conditional
	if( etype == AST_BRANCH)
	{
		AstBranch *conditional = static_cast<AstBranch*>(e);
		Type *predicate = eval(conditional->get_pred());
		
		if(predicate != integer) // predicate can only be an integer
			assert(predicate == integer);
		
		Type *exp1 = eval(conditional->get_then_exp());
		Type *exp2 = eval(conditional->get_else_exp());

		if(exp1 != exp2)		// then and else should be same type
			assert(exp1 == exp2);
		
		return exp1;
	}

}

/*TypeInference::TypeInference(Expression * e)
{
	this->program = e;

	Type* t1 = ConstantType::make("Int");
	Type* t2 = ConstantType::make("String");
	Type* t3 = ConstantType::make("Int");
	Type* var1 = VariableType::make("x");
	cout << t1->to_string() << " " << t1 << endl;
	cout << t2->to_string() << " " << t2 << endl;
	cout << t3->to_string() << " " << t3 << endl;
	vector<Type*> v1;
	v1.push_back(var1);
	v1.push_back(t2);

	vector<Type*> v2;
	v2.push_back(t3);
	v2.push_back(t2);

	Type* t4 = FunctionType::make("fun", v1);
	Type* t5 = FunctionType::make("fun", v2);
	cout << t4->to_string() << " " << t4 << endl;
	cout << t5->to_string() << " " << t5 << endl;

	//prints all types & reps in table
	Type::print_all_types();

	{
		Type* t1 = t4;
		Type* t2 = t5;

		cout << "Type 1:" << t1->to_string() << "Rep: " << t1->find()->to_string() << endl;
		cout <<	"Type 2:" << t2->to_string() << "Rep: " << t2->find()->to_string() << endl;

		cout << "unify: " << t1->unify(t2) << endl;

		cout << "Type 1:" << t1->to_string() << "Rep: " << t1->find()->to_string() << endl;
		cout <<	"Type 2:" << t2->to_string() << "Rep: " << t2->find()->to_string() << endl;
	}
		cout << "Type 1:" << t1->to_string() << "Rep: " << t1->find()->to_string() << endl;
		cout <<	"Type 2:" << var1->to_string() << "Rep: " << var1->find()->to_string() << endl;


	Type::print_all_types();



}*/
