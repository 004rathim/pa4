

#ifndef TYPE_INFERENCE_H_
#define TYPE_INFERENCE_H_


#include "SymbolTable.h"

class Expression;

class TypeInference {
private:
	Expression* program;
	Type* eval(Expression* e);
	Type* eval(Expression* e, AstIdentifier *ide);
public:
	TypeInference(Expression* p);

};

#endif /* TYPE_INFERENCE_H_ */
