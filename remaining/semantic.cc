#include "semantic.hh"


semantic *type_checker = new semantic();


/* Used to check that all functions contain return statements.
   Static means that it is only visible inside this file.
   It is set to false in do_typecheck() (ie, every time we start type checking
   a new block) and set to true if we find an ast_return node. See below. */
static bool has_return = false;


/* Interface for type checking a block of code represented as an AST node. */
void semantic::do_typecheck(symbol *env, ast_stmt_list *body)
{
    // Reset the variable, since we're checking a new block of code.
    has_return = false;
    if (body) {
        body->type_check();
    }

    // This is the only case we need this variable for - a function lacking
    // a return statement. All other cases are already handled in
    // ast_return::type_check(); see below.
    if (env->tag == SYM_FUNC && !has_return) {
        // Note: We could do this by overloading the do_typecheck() method -
        // one for ast_procedurehead and one for ast_functionhead, but this
        // will do... Hopefully people won't write empty functions often,
        // since in that case we won't have position information available.
        if (body != NULL) {
            type_error(body->pos) << "A function must return a value.\n";
        } else {
            type_error() << "A function must return a value.\n";
        }
    }
}


/* Compare formal vs. actual parameters recursively. */
bool semantic::chk_param(ast_id *env,
                        parameter_symbol *formals,
                        ast_expr_list *actuals)
{
    /* Your code here */
	if(formals == NULL && actuals == NULL)
	{
		return true;
	}else if(actuals == NULL || formals == NULL){
		type_error(env->pos) << "Invalid number of parameters\n";
		return false;
	}

	if(actuals->last_expr != NULL)
	{
		if(formals->type == actuals->last_expr->type){
			return chk_param(env, formals->preceding, actuals->preceding);
		}
		else if(formals->type == real_type && actuals->last_expr->type == integer_type){
			actuals->preceding->last_expr = new ast_cast(actuals->last_expr->pos, actuals->last_expr);
			return chk_param(env, formals->preceding, actuals->preceding);
		}
		else if(formals->type == integer_type && actuals->last_expr->type != integer_type){
			type_error(env->pos) << "Type discrepancy between formal and actual parameters.\n";
		}
	}

	return false;
}


/* Check formal vs. actual parameters at procedure/function calls. */
void semantic::check_parameters(ast_id *call_id,
                                ast_expr_list *param_list)
{
    /* Your code here */
	if(param_list != NULL)
	{
		param_list->type_check();
	}
	symbol* tmp = sym_tab->get_symbol(call_id->sym_p);

	if(tmp->tag == SYM_FUNC){
		 function_symbol* par = tmp->get_function_symbol();
		 if(!chk_param(call_id, par->last_parameter, param_list)){
		     	//type_error(call_id->pos) << "Function call error\n";
		     }
	}else{
		procedure_symbol* par = tmp->get_procedure_symbol();
		if(!chk_param(call_id, par->last_parameter, param_list)){
				//type_error(call_id->pos) << "Procedure call error\n";
		    }
	}
}



/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
sym_index ast_node::type_check()
{
    fatal("Trying to type check abstract class ast_node.");
    return void_type;
}

sym_index ast_statement::type_check()
{
    fatal("Trying to type check abstract class ast_statement.");
    return void_type;
}

sym_index ast_expression::type_check()
{
    fatal("Trying to type check abstract class ast_expression.");
    return void_type;
}

sym_index ast_lvalue::type_check()
{
    fatal("Trying to type check abstract class ast_lvalue.");
    return void_type;
}

sym_index ast_binaryoperation::type_check()
{
    fatal("Trying to type check abstract class ast_binaryoperation.");
    return void_type;
}

sym_index ast_binaryrelation::type_check()
{
    fatal("Trying to type check abstract class ast_binaryrelation.");
    return void_type;
}



/* Type check a list of statements. */
sym_index ast_stmt_list::type_check()
{
	if (last_stmt != NULL){
		last_stmt->type_check();
	}
    if (preceding != NULL) {
        preceding->type_check();
    }
    return void_type;
}


/* Type check a list of expressions. */
sym_index ast_expr_list::type_check()
{
    /* Your code here */
	if (last_expr != NULL) {
		last_expr->type_check();
	}
    if (preceding != NULL) {
        preceding->type_check();
    }

    return void_type;
}



/* Type check an elsif list. */
sym_index ast_elsif_list::type_check()
{
    /* Your code here */
	if(this->preceding == NULL) return integer_type;
	this->last_elsif->body->type_check();
	sym_index cond_type = this->last_elsif->condition->type_check();
	if(cond_type != integer_type){
		return void_type;
	}

	return this->preceding->type_check();
}


/* "type check" an indentifier. We need to separate nametypes from other types
   here, since all nametypes are of type void, but should return an index to
   itself in the symbol table as far as typechecking is concerned. */
sym_index ast_id::type_check()
{
    if (sym_tab->get_symbol(sym_p)->tag != SYM_NAMETYPE) {
        return type;
    }
    return sym_p;
}


sym_index ast_indexed::type_check()
{
	sym_index id_type = this->id->type_check();
	sym_index index_type = this->index->type_check();
	this->type = id->type;

	if(index_type == integer_type){
		return id_type;
	} else {
		type_error(this->pos) << id_type << endl;
		type_error(this->pos) << index_type << endl;
		type_error(this->pos) << "index not of type integer\n";
	}
    /* Your code here */
    return void_type;
}



/* This convenience function is used to type check all binary operations
   in which implicit casting of integer to real is done: plus, minus,
   multiplication. We synthesize type information as well. */
sym_index semantic::check_binop1(ast_binaryoperation *node)
{
    sym_index lhs_type = node->left->type_check();
    sym_index rhs_type = node->right->type_check();

    if(lhs_type == integer_type && rhs_type == integer_type){
    	node->type = integer_type;
    	return integer_type;
    } else if(lhs_type == real_type && rhs_type == real_type){
    	node->type = real_type;
    	return real_type;
    }
    else if(lhs_type == integer_type){
    	node->left = new ast_cast(node->left->pos, node->left);
    	node->type = real_type;
    	return real_type;
    } else if(rhs_type == integer_type){
    	node->right = new ast_cast(node->right->pos, node->right);
    	node->type = real_type;
    	return real_type;
    }

    return void_type;
}

sym_index ast_add::type_check()
{
    /* Your code here */
	return type_checker->check_binop1(this);
}

sym_index ast_sub::type_check()
{
    /* Your code here */
	return type_checker->check_binop1(this);
}

sym_index ast_mult::type_check()
{
    /* Your code here */
	return type_checker->check_binop1(this);
}

/* Divide is a special case, since it always returns real. We make sure the
   operands are cast to real too as needed. */
sym_index ast_divide::type_check()
{
    this->left->type_check();
    this->right->type_check();
    this->type = real_type;

   	if(this->left->type == integer_type){
   		this->left = new ast_cast(this->left->pos, this->left);
   	} if(this->right->type == integer_type){
   		this->right = new ast_cast(this->right->pos, this->right);
   	}
   	return real_type;
}



/* This convenience method is used to type check all binary operations
   which only accept integer operands: AND, OR, MOD, DIV.
   The second argument is the name of the operator, so we can generate a
   good error message.
   All of these return integers, so we synthesize that.
   */
sym_index semantic::check_binop2(ast_binaryoperation *node, string s)
{
    /* Your code here */
	sym_index lhs_type = node->left->type_check();
	sym_index rhs_type = node->right->type_check();
	node->type = integer_type;

	if(lhs_type == rhs_type && lhs_type == integer_type){
		return integer_type;
	} else if(lhs_type == integer_type && rhs_type == real_type){
    	node->left = new ast_cast(node->left->pos, node->left);
    	return integer_type;
    } else if(rhs_type == integer_type && rhs_type == real_type){
    	node->right = new ast_cast(node->right->pos, node->right);
    	return integer_type;
    }

	type_error(node->pos) << "invalid condition in " << s << "-statement, all variables must be int\n";
    return void_type;
}

sym_index ast_or::type_check()
{
    /* Your code here */
	return type_checker->check_binop2(this, "OR");
}

sym_index ast_and::type_check()
{
    /* Your code here */
	return type_checker->check_binop2(this, "AND");
}

sym_index ast_idiv::type_check()
{
    /* Your code here */
	return type_checker->check_binop2(this, "IDIV");
}

sym_index ast_mod::type_check()
{
    /* Your code here */
	return type_checker->check_binop2(this, "MOD");
}



/* Convienience method for all binary relations, since they're all typechecked
   the same way. They all return integer types, 1 = true, 0 = false. */
sym_index semantic::check_binrel(ast_binaryrelation *node)
{
    /* Your code here */
    sym_index lhs_type = node->left->type_check();
    sym_index rhs_type = node->right->type_check();
    node->type = integer_type;

    if(lhs_type == integer_type && rhs_type == integer_type){
    	return integer_type;
    } else if(lhs_type == integer_type){
    	node->left = new ast_cast(node->left->pos, node->left);
    	return integer_type;
    } else if(rhs_type == integer_type){
    	node->right = new ast_cast(node->right->pos, node->right);
    	return integer_type;
    }

    return void_type;
}

sym_index ast_equal::type_check()
{
    /* Your code here */
	return type_checker->check_binrel(this);
}

sym_index ast_notequal::type_check()
{
    /* Your code here */
	return type_checker->check_binrel(this);
}

sym_index ast_lessthan::type_check()
{
    /* Your code here */
	return type_checker->check_binrel(this);
}

sym_index ast_greaterthan::type_check()
{
    /* Your code here */
	return type_checker->check_binrel(this);
}



/*** The various classes derived from ast_statement. ***/

sym_index ast_procedurecall::type_check()
{
	type_checker->check_parameters(this->id, this->parameter_list);
	return void_type;
}


sym_index ast_assign::type_check()
{
    /* Your code here */

	sym_index lhs_type = this->lhs->type_check();
	sym_index rhs_type = this->rhs->type_check();

	if(lhs_type == rhs_type){
		return this->lhs->type;
	} else if(lhs_type == real_type && rhs_type == integer_type){
		this->rhs = new ast_cast(this->rhs->pos, this->rhs);
		return lhs_type;
	}else {
		type_error(this->pos) 	<< "lhs and rhs must be of same type in assignment. "
								<< "lhs: " << lhs_type << " rhs: " << rhs_type << "\n";
		return void_type;
	}
}


sym_index ast_while::type_check()
{
    if (condition->type_check() != integer_type) {
        type_error(condition->pos) << "while predicate must be of integer "
                                   << "type.\n";
    }

    if (body != NULL) {
        body->type_check();
    }
    return void_type;
}


sym_index ast_if::type_check()
{
    /* Your code here */
	sym_index condition_type = this->condition->type_check();
	this->body->type_check();

	if(condition_type != integer_type){
		type_error(condition->pos) << "if predicate must be of integer type.\n";
		return void_type;
	}

	if(this->elsif_list != NULL){
		this->elsif_list->type_check();
	}

	if(this->else_body != NULL){
		this->else_body->type_check();
	}

	return void_type;
}


sym_index ast_return::type_check()
{
    // This static global (meaning it is global for all methods in this file,
    // but not visible outside this file) variable is reset to 0 every time
    // we start type checking a new block of code. If we find a return
    // statement, we set it to 1. It is checked in the do_typecheck() method
    // of semantic.cc.
    has_return = true;

    // Get the current environment. We don't yet know if it's a procedure or
    // a function.
    symbol *tmp = sym_tab->get_symbol(sym_tab->current_environment());
    if (value == NULL) {
        // If the return value is NULL,
        if (tmp->tag != SYM_PROC)
            // ...and we're not inside a procedure, something is wrong.
        {
            type_error(pos) << "Must return a value from a function.\n";
        }
        return void_type;
    }

    sym_index value_type = value->type_check();

    // The return value is not NULL,
    if (tmp->tag != SYM_FUNC) {
        // ...so if we're not inside a function, something is wrong too.
        type_error(pos) << "Procedures may not return a value.\n";
        return void_type;
    }

    // Now we know it's a function and can safely downcast.
    function_symbol *func = tmp->get_function_symbol();

    // Must make sure that the return type matches the function's
    // declared return type.
    if (func->type != value_type) {
        type_error(value->pos) << "Bad return type from function.\n";
    }

    return void_type;
}


sym_index ast_functioncall::type_check()
{
    /* Your code here */
	type_checker->check_parameters(this->id, this->parameter_list);
    return this->id->type;
}

sym_index ast_uminus::type_check()
{
    /* Your code here */
	this->type = this->expr->type_check();
	return this->type;
}

sym_index ast_not::type_check()
{
    /* Your code here */
	this->expr->type_check();

	if(this->expr->type == integer_type){
		return integer_type;
	}

	type_error(this->expr->pos) << "negated expr must be of type integer\n";
    return void_type;
}


sym_index ast_elsif::type_check()
{
    /* Your code here */
	//Condition must be integer
	if( condition->type == integer_type ){
				return integer_type;
			} else {
				type_error(condition->pos) << "Condition is not of type integer.\n";
			}

	return void_type;
}



sym_index ast_integer::type_check()
{
    return integer_type;
}

sym_index ast_real::type_check()
{
    return real_type;
}
