
#include "ast/ConstantType.h"
#include "ast/VariableType.h"
#include "ast/FunctionType.h"
#include "ast/expression.h"
#include "TypeInference.h"


TypeInference::TypeInference(Expression* e)
{
	this->program = e;
	Type* value = eval(program);
	cout << value->to_string() << endl;

}          

Type* TypeInference::eval(Expression* e)
{

	
	ConstantType *integer =  ConstantType::make("Int");	
	ConstantType *string =  ConstantType::make("String");
	expression_type etype = e->get_type();

	if(etype == AST_BINOP)
	{
		AstBinOp *bin = static_cast<AstBinOp*>(e);
		binop_type btype = bin->get_binop_type();
		Expression *first = bin->get_first();
		Type *firstType = eval(first);
		Expression *second = bin->get_second();
		Type *secondType = eval(second);	
				
		if(btype == PLUS)
			if(firstType == integer && secondType == integer)
				return integer;
			else
				assert(false);
	}
	if(e->get_type()== AST_INT){
                return ConstantType::make("Int");
        }
        if(e->get_type()== AST_STRING){
                return ConstantType::make("String");
        }
        if(e->get_type()== AST_IDENTIFIER){
                return VariableType::make("x");
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

