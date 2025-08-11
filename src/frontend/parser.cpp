/* frontend/parser.cpp by David Filiks */
/* The parser implementation for the PsL compiler */

#include <cassert>
#include <algorithm>
#include <memory>

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "../utils/error_types.hpp"

Parser::Parser(const Lexer& lexer)
: m_tokens(lexer.get_tokens()), m_source(lexer.get_source()), m_source_path(lexer.get_source_path()) /* Initialize private members */ {}

void Parser::execute()
{
	/* Execute the parse() function */
	parse();
}

void Parser::parse()
{
	/* Add the standard function declaration statements onto the beginning of AST */
	/* Populate the existing functions array with standard library functions */
	populate_stdlib_funcs();

	/* Parse the program until the end of file token */
	m_ast.m_body = parse_body_until({TokenType::END_OF_FILE});

	/* Parse function bodies on the second pass */
	parse_func_bodies_2nd_pass();

	/* Parse unresolved expressions on the second pass */
	parse_unresolved_exprs_2nd_pass();
}

[[nodiscard]] const AST& Parser::get_ast() const
{
	return m_ast; /* Return the AST */
}

[[nodiscard]] const std::vector<Token>& Parser::get_tokens() const
{
	return m_tokens; /* Return the used token stream */;
}

[[nodiscard]] const std::size_t& Parser::get_token_index() const
{
	return m_token_index; /* Return the current index within the token stream */
}

[[nodiscard]] const std::string& Parser::get_source() const
{
	return m_source; /* Return the source contents */
}

[[nodiscard]] const std::string& Parser::get_source_path() const
{
	return m_source_path; /* Return the source path */
}

[[nodiscard]] const std::vector<std::unique_ptr<VarStmt>>& Parser::get_existing_vars() const
{
	return m_existing_vars; /* Return the existing variables list */
}

[[nodiscard]] const std::vector<std::unique_ptr<FuncDeclStmt>>& Parser::get_existing_funcs() const
{
	return m_existing_funcs; /* Return the existing functions list */
}

[[nodiscard]] const std::vector<std::unique_ptr<RecordStmt>>& Parser::get_existing_records() const
{
	return m_existing_records; /* Return the existing records list */
}

[[nodiscard]] const std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>& Parser::get_unresolved_exprs() const
{
	return m_unresolved_exprs; /* Return the unresolved expressions list */
}

[[nodiscard]] const Stack<std::size_t>& Parser::get_var_scope_stack() const
{
	return m_var_scope_stack; /* Return the variable scope stack */
}

[[nodiscard]] Stmt Parser::parse_stmt()
{
	/* No need to try_peek() here as if control flow reaches this function, it means that peek() is valid */

	/* Switch through all statements we can deduce from only peeking at the current token */
	switch (peek().m_type)
	{
		/* If the current token type is CONSTANT */
		case (TokenType::CONSTANT):
			/* Parse the remainder as a variable statement */
			/* This is because only variables can be prefixed with CONSTANT */
			/* Uses std::move() because VarStmt contains non-copyable unique_ptrs */
			return Stmt{std::move(parse_var_stmt())};

		/* If the current token type is REPEAT */
		case (TokenType::REPEAT):
			/* Parse the remainder as a repeat until statement */
			return Stmt{parse_repeat_until_stmt()};

		/* If the current token type is WHILE */
		case (TokenType::WHILE):
			/* Parse the remainder as a while statement */
			return Stmt{parse_while_stmt()};

		/* If the current token type is IF */
		case (TokenType::IF):
			/* Parse the remainder as an if statement */
			return Stmt{parse_if_stmt()};

		/* If the current token type is OUTPUT */
		case (TokenType::OUTPUT):
			/* Parse the remainder as an output statement */
			return Stmt{parse_output_stmt()};

		/* If the current token type is RECORD */
		case (TokenType::RECORD):
			/* Parse the remainder as a record statement */
			return Stmt{parse_record_stmt()};

		/* If the current token type is SUB_ROUTINE */
		case (TokenType::SUB_ROUTINE):
			/* Parse the remainder as a function declaration statement */
			/* Uses std::move() because FuncDeclStmt contains non-copyable unique_ptrs */
			return Stmt{std::move(parse_func_decl_stmt())};

		/* In the case of no matches */
		default: /* Fall through in order to possibly deduce using more tokens */
	}

	/* Try to peek one token ahead, checking it's not end of file */
	try_peek(1);

	/* If the current token type is IDENTIFIER */
	if (peek().m_type == TokenType::IDENTIFIER)
	{
		/* Switch through all possible next tokens */
		switch (peek(1).m_type)
		{
			/* If the next token is DOT */
			case (TokenType::DOT):
				/* Parse the remainder as a field access statement */
				return Stmt{parse_field_access_stmt()};

			/* If the next token is COLON */
			case (TokenType::COLON):
				/* Parse the remainder as a field statement */
				return Stmt{parse_field_stmt()};

			/* If the next token is SQ_O_BRACKET */
			case (TokenType::SQ_O_BRACKET):
				/* Parse the remainder as a list access statement */
				return Stmt{parse_list_access_stmt()};

			/* If the next token is LESS_THAN */
			case (TokenType::LESS_THAN):
				/* Parse the remainder as a variable statement */
				/* Uses std::move() because VarStmt contains non-copyable unique_ptrs */
				return Stmt{std::move(parse_var_stmt())};

			/* If the next token is O_PAREN */
			case (TokenType::O_PAREN):
				/* Parse the remainder as a function call statement */
				return Stmt{parse_func_call_stmt()};

			/* In the case of no matches */
			default: /* Fall through to error */
		}
	}

	/* If the current token type is ELSE */
	if (peek().m_type == TokenType::ELSE)
	{
		/* If the next token is IF */
		if (peek(1).m_type == TokenType::IF)
		{
			/* Parse the remainder as an else if statement */
			return Stmt{parse_else_if_stmt()};
		}

		/* If the next token is not IF */
		else
		{
			/* Parse the remainder as an else statement */
			return Stmt{parse_else_stmt()};
		}
	}

	/* Try to peek two tokens ahead, checking it's not end of file */
	try_peek(2);

	/* If the current token type is FOR */
	/* If the token after the next token is of type IN */
	if (peek().m_type == TokenType::FOR)
	{
		/* If the token after the next token is of type IN */
		if (peek(2).m_type == TokenType::IN)
		{
			/* Parse the remainder as a for in statement */
			return Stmt{parse_for_in_stmt()};
		}

		/* If the token after the next token is not of type IN */
		else
		{
			/* Parse the remainder as a for to statement */
			return Stmt{parse_for_to_stmt()};
		}
	}

	/* If no statement was found */
	/* Error out */
	ParseError
	{
		"no statement found",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};

	/* Make the compiler stop giving warnings due to possible no return */
	return Stmt{};
}

[[nodiscard]] VarStmt& Parser::parse_var_stmt()
{
	/* Create a new variable statement pointer */
	std::unique_ptr<VarStmt> var_stmt{std::make_unique<VarStmt>()};

	/* If the current token type is CONSTANT */
	if (peek().m_type == TokenType::CONSTANT)
	{
		/* Set the is constant boolean to true */
		var_stmt->m_is_constant = true;

		/* Consume and advance past the CONSTANT keyword */
		try_consume(TokenType::CONSTANT);
	}

	/* Try to consume and store a variable statement identifier */
	var_stmt->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Try to consume tokens indicating an left arrow "<-" symbol */
	try_consume(TokenType::LESS_THAN);
	try_consume(TokenType::SUBTRACTION);

	/* Try to peek the current token, checking it's not end of file */
	try_peek();

	/* If the token value after the arrow symbol matches with an existing record name */
	if (is_record(peek().m_value))
	{
		/* Store the record name for potential use later on */
		var_stmt->m_record_name = peek().m_value;
	}

	/* If the token type after the arrow symbol is SQ_O_BRACKET */
	else if (peek().m_type == TokenType::SQ_O_BRACKET)
	{
		/* Safely assume that the user is creating a one dimensional list */
		var_stmt->m_is_1d_list = true;

		/* Try to peek one token ahead, checking it's not end of file */
		try_peek(1);

		/* If the token type after the SQ_O_BRACKET is also SQ_O_BRACKET */
		if (peek(1).m_type == TokenType::SQ_O_BRACKET)
		{
			/* Safely assume that the user is creating a two dimensional list */
			var_stmt->m_is_2d_list = true;

			/* Try to peek two tokens ahead, checking it's not end of file */
			try_peek(2);

			/* If the there are three tokens in a row with a type of SQ_O_BRACKET */
			if (peek(2).m_type == TokenType::SQ_O_BRACKET)
			{
				/* Indicates that the user is trying to create a three dimensional list */
				/* E.g. list <- [[[...]]] */
				/* Error out */
				ParseError
				{
					"three dimensional lists are not supported",
					m_tokens[m_token_index].m_row,
					m_tokens[m_token_index].m_col,
					m_source
				};
			}
		}
	}

	/* If the variable with given name was previously defined */
	if (is_var_defined(var_stmt->m_name))
	{
		/* If the variable was constant */
		if (existing_var_lookup(var_stmt->m_name)->m_is_constant)
		{
			/* Set reassignment statement to a constant as well, for easier error detection in the generation stage */
			var_stmt->m_is_constant = true;
		}

		/* Set the previous variable expression to the existing variable's current expression */
		var_stmt->m_previous_expr = std::make_unique<Expr>(existing_var_lookup(var_stmt->m_name)->m_expr);

		/* Set reassignment boolean to true, which helps in the generation stage */
		var_stmt->m_is_reassignment = true;

		/* Parse and set the new variable expression */
		var_stmt->m_expr = std::make_unique<Expr>(parse_expr());

		/* Return the variable statement */
		return *var_stmt;
	}

	/* If the variable was not previously defined */
	else
	{
		/* Parse the variable expression */
		var_stmt->m_expr = std::make_unique<Expr>(parse_expr());

		/* Append the variable pointer to the existing variables array */
		m_existing_vars.push_back(std::move(var_stmt));

		/* Return the variable statement from list */
		return *m_existing_vars.back();
	}
}

[[nodiscard]] FieldAccessStmt Parser::parse_field_access_stmt()
{
	/* Create a new field access statement object */
	FieldAccessStmt field_access_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is IDENTIFIER */
	/* Consume and store a variable identifier */
	field_access_stmt.m_name = consume().m_value;

	/* No try_consume() for the same reason as before */
	/* Consume a token of type DOT */
	consume();

	/* Try to consume and store the field name which the programmer is trying to access */
	field_access_stmt.m_field_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Try to consume tokens indicating an left arrow "<-" symbol */
	try_consume(TokenType::LESS_THAN);
	try_consume(TokenType::SUBTRACTION);

	/* Parse the new field expression */
	field_access_stmt.m_expr = std::make_unique<Expr>(parse_expr());

	/* Return the field access statement */
	return field_access_stmt;
}

[[nodiscard]] OutputStmt Parser::parse_output_stmt()
{
	/* Create a new output statement object */
	OutputStmt output_stmt{};

	/* No consume of the OUTPUT token as it is done in parse_cse() */
	/* Parse the output statement arguments as comma separated values */
	output_stmt.m_args.m_exprs = parse_cse();

	/* Return the output statement */
	return output_stmt;
}

[[nodiscard]] FuncDeclStmt& Parser::parse_func_decl_stmt()
{
	/* Create a new function declaration statement pointer */
	std::unique_ptr<FuncDeclStmt> func_decl_stmt{std::make_unique<FuncDeclStmt>()};

	/* Consume the SUBROUTINE token */
	/* No need to use try_consume() as entering this function implies that the current token is SUBROUTINE */
	consume();

	/* Try to consume and store a function name */
	func_decl_stmt->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse the function declaration parameters */
	func_decl_stmt->m_params = parse_func_decl_params();

	/* No need to use try_consume() for same reason as before */
	/* Consume a C_PAREN token */
	consume();

	/* Save index position and skip over function body to parse it on the second pass */
	func_decl_stmt->m_token_index_start = m_token_index;
	skip_over_function_body();

	/* No need to use try_consume() for the same reasons as before */
	/* Consume a ENDSUBROUTINE token */
	consume();

	/* Append the function to the existing functions list */
	m_existing_funcs.push_back(std::move(func_decl_stmt));

	/* Return the function declaration statement */
	return *m_existing_funcs.back();
}

[[nodiscard]] FuncCallStmt Parser::parse_func_call_stmt()
{
	/* Create a new function call statement object */
	FuncCallStmt func_call_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is IDENTIFIER */
	/* Consume and store the function name */
	func_call_stmt.m_name = consume().m_value;

	/* No try_consume() for the same reason as before */
	/* Consume a token of type O_PAREN */
	consume();

	/* Parse the function call arguments */
	func_call_stmt.m_args = parse_args();

	/* Check that the function has been declared */
	check_func_declared(func_call_stmt.m_name);

	/* Check that the function call argument count matches up with the function declaration */
	check_arg_count_matches(func_call_stmt.m_name, func_call_stmt.m_args);

	/* Deduce the types of the function declaration parameters from the types of the arguments used in this call */
	deduce_func_decl_param_types_from_args(func_call_stmt.m_name, func_call_stmt.m_args);

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the function call statement */
	return func_call_stmt;
}

[[nodiscard]] RepeatUntilStmt Parser::parse_repeat_until_stmt()
{
	/* Create a new repeat until statement object */
	RepeatUntilStmt repeat_until_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is REPEAT */
	/* Consume a token of type REPEAT */
	consume();

	/* Parse the repeat until statement body */
	repeat_until_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::UNTIL}));

	/* No try_consume() used here as the function above exits upon a UNTIL token */
	/* Consume a token of type UNTIL */
	consume();

	/* Parse and store the condition expression */
	repeat_until_stmt.m_condition_expr = std::make_unique<Expr>(parse_expr());

	/* Return the repeat until statement */
	return repeat_until_stmt;
}

[[nodiscard]] WhileStmt Parser::parse_while_stmt()
{
	/* Create a new while statement object */
	WhileStmt while_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is WHILE */
	/* Consume a token of type WHILE */
	consume();

	/* Parse and store the condition expression */
	while_stmt.m_condition_expr = std::make_unique<Expr>(parse_expr());

	/* Parse and store the while statement body */
	while_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::END_WHILE}));

	/* No try_consume() used here as the function above exits upon a ENDWHILE token */
	/* Consume a token of type END_WHILE */
	consume();

	/* Return the while statement */
	return while_stmt;
}

[[nodiscard]] IfStmt Parser::parse_if_stmt()
{
	/* Create a new if statement object */
	IfStmt if_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is IF */
	/* Consume a token of type IF */
	consume();

	/* Parse and store the condition expression */
	if_stmt.m_condition_expr = std::make_unique<Expr>(parse_expr());

	/* Try to consume a token of type THEN */
	try_consume(TokenType::THEN);

	/* Parse and store the if statement body */
	/* An if statement can end in two ways, either with ENDIF or ELSE IF/ELSE */
	if_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::ELSE, TokenType::END_IF}));

	/* If the current token has type END_IF*/
	/* This function is responsible for handling it, unlike if the current token was ELSE */
	if (peek().m_type == TokenType::END_IF)
	{
		/* Consume and advance past the ENDIF token*/
		consume();
	}

	/* Return the if statement */
	return if_stmt;
}

[[nodiscard]] ElseIfStmt Parser::parse_else_if_stmt()
{
	/* Create a new else if statement object */
	ElseIfStmt else_if_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is ELSE and the next token is IF */
	/* Consume the ELSE and IF tokens */
	consume(2);

	/* Parse and store the condition expression */
	else_if_stmt.m_condition_expr = std::make_unique<Expr>(parse_expr());

	/* Try to consume a token of type THEN */
	try_consume(TokenType::THEN);

	/* Parse and store the else if statement body */
	else_if_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::ELSE, TokenType::END_IF}));

	/* If the current token has type END_IF*/
	/* This function is responsible for handling it, unlike if the current token was ELSE */
	if (peek().m_type == TokenType::END_IF)
	{
		/* Consume and advance past the ENDIF token*/
		consume();
	}

	/* Return the else if statement */
	return else_if_stmt;
}

[[nodiscard]] ElseStmt Parser::parse_else_stmt()
{
	/* Create a new else statement object */
	ElseStmt else_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is ELSE */
	/* Consume an ELSE token */
	consume();

	/* Parse and store the else statement body */
	else_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::END_IF}));

	/* No try_consume() used here as the function above exits upon a ENDIF token */
	/* Consume a token of type END_IF */
	consume();

	/* Return the else statement */
	return else_stmt;
}

[[nodiscard]] ForToStmt Parser::parse_for_to_stmt()
{
	/* Create a new for to statement object */
	ForToStmt for_to_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is FOR */
	/* Consume a FOR token */
	consume();

	/* Parse and set a variable statement as part of the condition */
	for_to_stmt.m_var_stmt = std::move(parse_var_stmt());

	/* Try to consume a token of type TO */
	try_consume(TokenType::TO);

	/* Parse and store the loop boundary expression */
	for_to_stmt.m_boundary = std::make_unique<Expr>(parse_expr());

	/* If the current token is of type STEP */
	if (peek().m_type == TokenType::STEP)
	{
		/* Consume and advance past the current token */
		consume();

		/* Parse and store the step expression */
		for_to_stmt.m_step = std::make_unique<Expr>(parse_expr());
	}

	/* Parse and store the for to statement body */
	for_to_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::END_FOR}));

	/* No try_consume() used here as the function above exits upon a ENDFOR token */
	/* Consume a token of type END_FOR */
	consume();

	/* Now that the loop is over, remove the declared variable, so that it cannot be used outside of the loop */
	remove_var(for_to_stmt.m_var_stmt.m_name);

	/* Return the for to statement */
	return for_to_stmt;
}

[[nodiscard]] ForInStmt Parser::parse_for_in_stmt()
{
	/* Create a new for in statement object */
	ForInStmt for_in_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is FOR */
	/* Consume a FOR token */
	consume();

	/* Allocate heap memory for the loop declaration */
	for_in_stmt.m_declaration = std::make_unique<VarStmt>();

	/* If the reason for try_consume() here confuses you, see the implementation of parse_stmt() */
	for_in_stmt.m_declaration->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* No try_consume() for the same reason as before */
	/* Consume a IN token */
	consume();

	/* Parse and store the loop range expression */
	for_in_stmt.m_range = std::make_unique<Expr>(parse_expr());

	/* Make the declaration variable act as a constant */
	for_in_stmt.m_declaration->m_is_constant = true;

	/* Allocate heap memory for the declaration's expression */
	for_in_stmt.m_declaration->m_expr = std::make_unique<Expr>();

	/* If the range type is unresolved */
	if (for_in_stmt.m_range->m_type == DataType::UNRESOLVED) 
	{
		/* This means that the declaration is also unresolved, and must be resolved later on */
		/* The declaration is paired to the range as they need to be of the same type */
		m_unresolved_exprs.emplace_back(*for_in_stmt.m_declaration->m_expr, *for_in_stmt.m_range);
	}

	/* If the range type is known */
	else
	{
		/* Set the declaration type to the same type as the range */
		for_in_stmt.m_declaration->m_expr->m_type = for_in_stmt.m_range->m_type;
	}

	/* Add the declaration variable to the existing variables list */
	m_existing_vars.push_back(std::move(for_in_stmt.m_declaration));

	/* Parse and store the for in statement body */
	for_in_stmt.m_body = std::make_unique<Body>(parse_body_until({TokenType::END_FOR}));

	/* No try_consume() used here as the function above exits upon a ENDFOR token */
	/* Consume a token of type END_FOR */
	consume();

	/* Now that the loop is over, remove the declared variable, so that it cannot be used outside of the loop */
	remove_var(for_in_stmt.m_declaration->m_name);

	/* Return the for in statement */
	return for_in_stmt;
}

[[nodiscard]] RecordStmt& Parser::parse_record_stmt()
{
	/* Create a new record statement pointer */
	std::unique_ptr<RecordStmt> record_stmt{std::make_unique<RecordStmt>()};

	/* No need to use try_consume() as entering this function implies the current token is RECORD */
	/* Consume a RECORD token */
	consume();

	/* Try to consume and store the record name */
	record_stmt->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Parse and store the record fields */
	record_stmt->m_fields = parse_fields_until({TokenType::END_RECORD});

	/* No try_consume() used here as the function above exits upon a ENDRECORD token */
	/* Consume the ENDRECORD token*/
	consume();

	/* Add the record to the existing records list */
	m_existing_records.push_back(std::move(record_stmt));

	/* Return the record statement */
	return *m_existing_records.back();
}

[[nodiscard]] FieldStmt Parser::parse_field_stmt()
{
	/* Create a new field statement object */
	FieldStmt field_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is IDENTIFIER */
	/* Consume an IDENTIFIER token */
	field_stmt.m_name = consume().m_value;

	/* No try_consume() used for the same reason as before */
	/* Consume a COLON token */
	consume();

	/* Try to peek the current token, checking it's not end of file */
	try_peek();

	/* Set the field to a valid data type */
	/* tt_to_dt() errors out if the token is unable to represent a valid data type */
	field_stmt.m_type = tt_to_dt(consume());

	/* Return the field statement */
	return field_stmt;
}

[[nodiscard]] ListAccessStmt Parser::parse_list_access_stmt()
{
	/* Create a new list access statement object */
	ListAccessStmt list_access_stmt{};

	/* No need to use try_consume() as entering this function implies the current token is IDENTIFIER */
	/* Consume and store the name of the list */
	list_access_stmt.m_name = consume().m_value;

	/* No try_consume() use due to the same reason as before */
	/* Consume a token of type SQ_O_BRACKET */
	consume();

	/* Parse and store the access row expression */
	list_access_stmt.m_row = std::make_unique<Expr>(parse_expr());

	/* Try to consume a token of type SQ_C_BRACKET */
	try_consume(TokenType::SQ_C_BRACKET);

	/* If the list is a two dimensional list */
	if (existing_var_lookup(list_access_stmt.m_name)->m_is_2d_list)
	{
		/* Try to consume a token of type SQ_O_BRACKET */
		try_consume(TokenType::SQ_O_BRACKET);

		/* Parse and store the access col expression */
		list_access_stmt.m_col = std::make_unique<Expr>(parse_expr());

		/* Try to consume a token of type SQ_C_BRACKET */
		try_consume(TokenType::SQ_C_BRACKET);
	}

	/* Try to consume tokens indicating an left arrow "<-" symbol */
	try_consume(TokenType::LESS_THAN);
	try_consume(TokenType::SUBTRACTION);

	/* Parse the list index's new expression */
	list_access_stmt.m_expr = std::make_unique<Expr>(parse_expr());

	/* Return the list access statement */
	return list_access_stmt;
}

void Parser::skip_over_function_body()
{
	/* Loop until the current token is not of type END_SUB_ROUTINE */
	while (peek().m_type != TokenType::END_SUB_ROUTINE)
	{
		/* Try to peek to the current token, checking if index has reached end of file */
		try_peek();

		/* Consume anything in the way */
		consume();
	}
}

[[nodiscard]] Params Parser::parse_func_decl_params()
{
	/* Create parameter list */
	std::vector<Param> params{};

	/* Loop until the parameter list is over, and a token of type C_PAREN is reached */
	while (peek().m_type != TokenType::C_PAREN)
	{
		/* Try to peek to the current token, checking if index has reached end of file */
		try_peek();

		/* Switch between the current token type */
		switch (peek().m_type)
		{
			/* If the current token type is COMMA */
			case (TokenType::COMMA):
				/* Consume the comma */
				consume();

				/* Break from the switch case */
				break;

			/* If the current token type is IDENTIFIER */
			case (TokenType::IDENTIFIER):
				/* Add a parameter with the identifier to the parameter list */
				params.push_back(Param{.m_name = peek().m_value});

				/* Consume the identifier */
				consume();

				/* Break from the switch case */
				break;

			/* If no valid parameter tokens were received */
			default:
				/* Error out */
				ParseError
				{
					"expected an identifier as function parameter",
					m_tokens[m_token_index].m_row,
					m_tokens[m_token_index].m_col,
					m_source
				};
		}
	}

	/* Return the parameter list as type Params */
	return Params{params};
}

[[nodiscard]] Args Parser::parse_args()
{
	/* Create argument list */
	std::vector<Expr> args{};

	/* Loop until the argument list is over, and a token of type C_PAREN is reached */
	while (peek().m_type != TokenType::C_PAREN)
	{
		/* Try to peek to the current token, checking if index has reached end of file */
		try_peek();

		/* Switch through the current token type */
		switch (peek().m_type)
		{
			/* If the current token is of type COMMA */
			case (TokenType::COMMA):
				/* Consume the comma */
				consume();

				/* Break from the switch case */
				break;

			/* If the current token is not of type COMMA */
			default:
				/* Parse the expression and add it to the argument list */
				args.push_back(parse_expr());
		}
	}

	/* Return the argument list as type Args */
	return Args{args};
}

void Parser::parse_func_bodies_2nd_pass()
{
	/* Loop through all of the existing functions */
	for (auto& func : m_existing_funcs)
	{
		/* If the function is not a standard library function */
		if (!is_stdlib(func->m_name))
		{
			/* Go to the start of the function body */
			m_token_index = func->m_token_index_start;

			/* Push the current amount of variables in the program onto the scope stack */
			m_var_scope_stack.push(m_existing_vars.size());

			/* Loop through all of the function parameters */
			for (const auto& param : func->m_params.m_params)
			{
				/* Create a new variable statement for each parameter */
				std::unique_ptr<VarStmt> var_stmt{std::make_unique<VarStmt>()};

				/* Set the parameter name */
				var_stmt->m_name = param.m_name;

				/* Allocate heap memory for the variable expression */
				var_stmt->m_expr = std::make_unique<Expr>();

				/* Set the type of the expression to the parameter type */
				var_stmt->m_expr->m_type = param.m_type;

				/* Add the parameter to the existing variables */
				m_existing_vars.push_back(std::move(var_stmt));
			}

			/* Parse and store the function body */
			func->m_body = std::make_unique<Body>(parse_body_until({TokenType::RETURN, TokenType::END_SUB_ROUTINE}));

			/* If the type of the current token is RETURN */
			if (peek().m_type == TokenType::RETURN)
			{
				/* Consume the RETURN token */
				consume();

				/* Parse and store the return expression */
				func->m_return.m_return_expr = std::make_unique<Expr>(parse_expr());

				/* Set member boolean to false to indicate that the function is not void */
				func->m_is_void = false;
			}

			/* Try to consume a token of type END_SUB_ROUTINE */
			try_consume(TokenType::END_SUB_ROUTINE);

			/* Resize the existing variables list to the most recent size in the scope stack */
			/* Use a pop operation on the scope stack */
			m_existing_vars.resize(m_var_scope_stack.pop());
		}
	}
}

void Parser::parse_unresolved_exprs_2nd_pass()
{
	/* Loop through all of the unresolved expressions */
	for (auto& unresolved_expr_pair : m_unresolved_exprs)
	{
		/* Resolve the type of the first element from the type of the second element */
		unresolved_expr_pair.first->m_type = unresolved_expr_pair.second->m_type;
	}
}

void Parser::check_arg_count_matches(const std::string_view name, const Args &args)
{
	/* Create a pointer to the function declaration statement with the matching name */
	FuncDeclStmt* func_decl_stmt{existing_func_lookup(name)};

	/* If the function declaration parameter count is not equal to the argument count */
	if (func_decl_stmt->m_params.m_params.size() != args.m_exprs.size())
	{
		/* Error out */
		ParseError
		{
			"wrong number of arguments provided",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}
}

[[nodiscard]] Expr Parser::parse_expr()
{
	/* Create the lhs expression object */
	Expr lhs{};

	/* If the type of the current token is SQ_O_BRACKET */
	if (peek().m_type == TokenType::SQ_O_BRACKET)
	{
		/* Create a list expression object */
		ListExpr list_expr{};

		/* No consume of the SQ_O_BRACKET token as it is done in parse_cse() */
		/* Parse the list expression as comma separated expressions */
		list_expr.m_exprs = parse_cse();

		/* Try to consume a token of type SQ_C_BRACKET */
		try_consume(TokenType::SQ_C_BRACKET);

		/* If the list expression is not empty */
		if (!list_expr.m_exprs.empty())
		{
			/* Set lhs type to the type of the first expression in the list expression */
			lhs.m_type = list_expr.m_exprs[0].m_type;
		}

		/* If the list expression is empty */
		else
		{
			/* Set lhs type to UNRESOLVED */
			lhs.m_type = DataType::UNRESOLVED;
		}

		/* Set lhs to the list expression */
		lhs = Expr{list_expr, lhs.m_type};
	}

	/* If the type of the current token is O_PAREN */
	else if (peek().m_type == TokenType::O_PAREN)
	{
		/* Create a parentheses expression object */
		ParenExpr paren_expr{};

		/* Consume the O_PAREN token */
		consume();

		/* Parse and store the parentheses expression */
		paren_expr.m_expr = std::make_unique<Expr>(parse_expr());

		/* Try to consume a token of type C_PAREN */
		try_consume(TokenType::C_PAREN);

		/* Set lhs to the parentheses expression */
		lhs = Expr{std::move(paren_expr), lhs.m_type};
	}

	/* If the type of the current token indicates it is a unary operator */
	else if (is_unary(peek().m_type))
	{
		/* Set lhs to the unary operator expression */
		lhs = Expr{parse_unary_op_expr(), lhs.m_type};
	}
	
	/* If the type of the current token is IDENTIFIER */
	else if (peek().m_type == TokenType::IDENTIFIER)
	{
		/* If the current token has a value that matches a record name */
		if (is_record(peek().m_value)) 
		{
			/* Set the lhs type to USER_DEFINED_TYPE */
			lhs.m_type = DataType::USER_DEFINED_TYPE;
		}
		
		/* If the current token has a value that matches a standard library function name */
		else if (is_stdlib(peek().m_value))
		{
			/* Set the lhs type to the return type of that standard library function */
			lhs.m_type = existing_func_lookup(peek().m_value)->m_return.m_return_expr->m_type;
		}
		
		/* Try to peek one token ahead, checking it's not end of file */
		try_peek(1);

		/* If the token ahead is of type O_PAREN */
		if (peek(1).m_type == TokenType::O_PAREN)
		{
			/* Set lhs type to UNRESOLVED */
			lhs.m_type = DataType::UNRESOLVED;

			/* Add lhs to the unresolved exprs list, as it is a function call */
			m_unresolved_exprs.emplace_back(lhs, peek().m_value);
		}

		/* If the token ahead is of type DOT */
		else if (peek(1).m_type == TokenType::DOT)
		{
			/* Try to peek two tokens ahead, checking it's not end of file */
			try_peek(2);

			/* Deduce the field type from a field access */
			/* A field access expression is something such as - OUTPUT jake.age */
			lhs.m_type = deduce_field_type_from_access(peek(), peek(2));
		}

		/* Set the lhs to the atom expression */
		lhs = Expr{parse_atom(), lhs.m_type};
	}

	/* If the expression type can be deduced easily */
	else
	{
		/* Deduce the expression type from the current token */
		lhs.m_type = deduce_expr_type(peek());

		/* Set the lhs to the atom expression */
		lhs = Expr{parse_atom(), lhs.m_type};
	}

	/* Loop while the current token type indicates a binary operator expression */
	while (is_bin_op(peek().m_type))
	{
		/* Create a binary operator expression object */
		BinOpExpr bin_op_expr{};

		/* Set the binary operator expression lhs to the current lhs */
		bin_op_expr.m_lhs = std::make_unique<Expr>(lhs);

		/* Allocate heap memory for the binary operator expression rhs */
		bin_op_expr.m_rhs = std::make_unique<Expr>();

		/* Determine and store the type of operator used */
		bin_op_expr.m_op = determine_op();

		/* If the current token is a unary operator */
		if (is_unary(peek().m_type))
		{
			/* Parse the rhs as a unary operator */
			bin_op_expr.m_rhs = std::make_unique<Expr>(parse_unary_op_expr());
		}
		
		/* If the current token is not a unary operator*/
		else
		{
			/* Deduce the type of the rhs expression */
			bin_op_expr.m_rhs->m_type = deduce_expr_type(peek());

			/* Parse the rhs as an atom expression */
			bin_op_expr.m_rhs = std::make_unique<Expr>(parse_atom());
		}

		/* Set the lhs to the binary operator expression */
		lhs = Expr{std::move(bin_op_expr), lhs.m_type};
	}

	/* Return the lhs expression */
	return lhs;
}

[[nodiscard]] AtomExpr Parser::parse_atom()
{
	/* No need to try_peek() here as if control flow reaches this function, it means that peek() is valid */

	/* If the current token type is INT */
	if (peek().m_type == TokenType::INT)
	{
		/* Parse the remainder as an integer expression */
		return AtomExpr{parse_int_expr()};
	}

	/* If the current token type is REAL */
	else if (peek().m_type == TokenType::REAL)
	{
		/* Parse the remainder as a real expression */
		return AtomExpr{parse_real_expr()};
	}

	/* If the current token type is STRING */
	else if (peek().m_type == TokenType::STRING)
	{
		/* Parse the remainder as a string expression */
		return AtomExpr{parse_str_expr()};
	}

	/* If the current token type is USER_INPUT */
	else if (peek().m_type == TokenType::USER_INPUT)
	{
		/* Parse the remainder as a user input expression */
		return AtomExpr{parse_user_input_expr()};
	}

	/* If the current token type is LEN */
	else if (peek().m_type == TokenType::LEN)
	{
		/* Parse the remainder as a LEN() standard library call expression */
		return AtomExpr{parse_len_call_expr()};
	}

	/* If the current token type is POSITION */
	else if (peek().m_type == TokenType::POSITION)
	{
		/* Parse the remainder as a POSITION() standard library call expression */
		return AtomExpr{parse_position_call_expr()};
	}

	/* If the current token type is SUBSTRING */
	else if (peek().m_type == TokenType::SUBSTRING)
	{
		/* Parse the remainder as a SUBSTRING() standard library call expression */
		return AtomExpr{parse_sub_str_call_expr()};
	}

	/* If the current token type is STRING_TO_INT */
	else if (peek().m_type == TokenType::STRING_TO_INT)
	{
		/* Parse the remainder as a STRING_TO_INT() standard library call expression */
		return AtomExpr{parse_str_to_int_call_expr()};
	}

	/* If the current token type is STRING_TO_REAL */
	else if (peek().m_type == TokenType::STRING_TO_REAL)
	{
		/* Parse the remainder as a STRING_TO_REAL() standard library call expression */
		return AtomExpr{parse_str_to_real_call_expr()};
	}

	/* If the current token type is INT_TO_STRING */
	else if (peek().m_type == TokenType::INT_TO_STRING)
	{
		/* Parse the remainder as an INT_TO_STRING() standard library call expression */
		return AtomExpr{parse_int_to_str_call_expr()};
	}

	/* If the current token type is REAL_TO_STRING */
	else if (peek().m_type == TokenType::REAL_TO_STRING)
	{
		/* Parse the remainder as a REAL_TO_STRING() standard library call expression */
		return AtomExpr{parse_real_to_str_call_expr()};
	}

	/* If the current token type is CHAR_TO_CODE */
	else if (peek().m_type == TokenType::CHAR_TO_CODE)
	{
		/* Parse the remainder as a CHAR_TO_CODE() standard library call expression */
		return AtomExpr{parse_char_to_code_call_expr()};
	}

	/* If the current token type is CODE_TO_CHAR */
	else if (peek().m_type == TokenType::CODE_TO_CHAR)
	{
		/* Parse the remainder as a CODE_TO_CHAR() standard library call expression */
		return AtomExpr{parse_code_to_char_call_expr()};
	}

	/* If the current token type is RANDOM_INT */
	else if (peek().m_type == TokenType::RANDOM_INT)
	{
		/* Parse the remainder as a RANDOM_INT() standard library call expression */
		return AtomExpr{parse_random_int_call_expr()};
	}

	/* If the current token value corresponds to a record name */
	else if (is_record(peek().m_value))
	{
		/* Parse the remainder as an object creation expression */
		return AtomExpr{parse_object_creation_expr()};
	}

	/* Try to peek one token ahead, checking it's not end of file */
	try_peek(1);

	/* If the current token type is IDENTIFIER */
	if (peek().m_type == TokenType::IDENTIFIER)
	{
		/* Switch through all the possible next token types */
		switch (peek(1).m_type)
		{
			/* If the next token is DOT */
			case (TokenType::DOT):
				/* Parse the remainder as a field access expression */
				return AtomExpr{parse_field_access_expr()};

			/* If the next token is SQ_O_BRACKET */
			case (TokenType::SQ_O_BRACKET):
				/* Parse the remainder as a list access expression */
				return AtomExpr{parse_list_access_expr()};

			/* If the next token is O_PAREN */
			case (TokenType::O_PAREN):
				/* Parse the remainder as a function call expression */
				return AtomExpr{parse_func_call_expr()};

			/* If no other match, must be a variable expression */
			default:
				/* Parse the remainder as a variable expression */
				return AtomExpr{parse_var_expr()};
		}
	}

	/* If no atom expression was found */
	ParseError
	{
		"no atom expression found",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};

	/* Make the compiler stop giving warnings due to possible no return */
	return AtomExpr{};
}

[[nodiscard]] IntExpr Parser::parse_int_expr()
{
	return IntExpr{std::stoi(try_consume(TokenType::INT).m_value)};
}

[[nodiscard]] RealExpr Parser::parse_real_expr()
{
	return RealExpr{std::stod(try_consume(TokenType::REAL).m_value)};
}

[[nodiscard]] StrExpr Parser::parse_str_expr()
{
	return StrExpr{try_consume(TokenType::STRING).m_value};
}

[[nodiscard]] VarExpr Parser::parse_var_expr()
{
	return VarExpr{try_consume(TokenType::IDENTIFIER).m_value};
}

[[nodiscard]] UnaryOpExpr Parser::parse_unary_op_expr()
{
	UnaryOpExpr unary_op_expr{};

	unary_op_expr.m_op = determine_op();

	unary_op_expr.m_unary_expr = std::make_unique<Expr>(*parse_expr());

	return unary_op_expr;
}

[[nodiscard]] FieldAccessExpr Parser::parse_field_access_expr()
{
	FieldAccessExpr field_access_expr{};

	field_access_expr.m_name = try_consume(TokenType::IDENTIFIER).m_value;

	try_consume(TokenType::DOT);

	field_access_expr.m_field_name = try_consume(TokenType::IDENTIFIER).m_value;

	return field_access_expr;
}

[[nodiscard]] ListAccessExpr Parser::parse_list_access_expr()
{
	ListAccessExpr list_access_expr{};

	list_access_expr.m_name = consume().m_value;

	consume();

	list_access_expr.m_row = parse_expr();

	try_consume(TokenType::SQ_C_BRACKET);

	if (existing_var_lookup(list_access_expr.m_name).m_is_2d_list && peek().m_type == TokenType::SQ_O_BRACKET)
	{
		try_consume(TokenType::SQ_O_BRACKET);

		list_access_expr.m_col = parse_expr();

		try_consume(TokenType::SQ_C_BRACKET);
	}

	return list_access_expr;
}

[[nodiscard]] FuncCallExpr Parser::parse_func_call_expr()
{
	FuncCallExpr func_call_expr{};

	func_call_expr.m_name = try_consume(TokenType::IDENTIFIER).m_value;

	try_consume(TokenType::O_PAREN);

	func_call_expr.m_args = parse_args();

	check_arg_count_matches(func_call_expr.m_name, func_call_expr.m_args);

	deduce_func_decl_param_types_from_expr(func_call_expr);

	try_consume(TokenType::C_PAREN);

	return func_call_expr;
}

[[nodiscard]] UserInputExpr Parser::parse_user_input_expr()
{
	try_consume(TokenType::USER_INPUT);
	return UserInputExpr{};
}

[[nodiscard]] ObjectCreationExpr Parser::parse_object_creation_expr()
{
	ObjectCreationExpr object_creation_expr{};

	object_creation_expr.m_record_name = try_consume(TokenType::IDENTIFIER).m_value;

	try_consume(TokenType::O_PAREN);

	object_creation_expr.m_args = parse_args();

	try_consume(TokenType::C_PAREN);

	return object_creation_expr;
}

[[nodiscard]] LenCallExpr Parser::parse_len_call_expr()
{
	LenCallExpr len_call_expr{};

	try_consume(TokenType::LEN);
	try_consume(TokenType::O_PAREN);

	len_call_expr.m_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return len_call_expr;
}

[[nodiscard]] PositionCallExpr Parser::parse_position_call_expr()
{
	PositionCallExpr position_call_expr{};

	try_consume(TokenType::POSITION);
	try_consume(TokenType::O_PAREN);

	position_call_expr.m_str_expr = parse_expr();

	try_consume(TokenType::COMMA);

	position_call_expr.m_char_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return position_call_expr;
}

[[nodiscard]] SubStrCallExpr Parser::parse_sub_str_call_expr()
{
	SubStrCallExpr sub_str_call_expr{};

	try_consume(TokenType::SUBSTRING);
	try_consume(TokenType::O_PAREN);

	sub_str_call_expr.m_num1_expr = parse_expr();

	try_consume(TokenType::COMMA);

	sub_str_call_expr.m_num2_expr = parse_expr();

	try_consume(TokenType::COMMA);

	sub_str_call_expr.m_str_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return sub_str_call_expr;
}

[[nodiscard]] StrToIntCallExpr Parser::parse_str_to_int_call_expr()
{
	StrToIntCallExpr str_to_int_call_expr{};

	try_consume(TokenType::STRING_TO_INT);
	try_consume(TokenType::O_PAREN);

	str_to_int_call_expr.m_str_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return str_to_int_call_expr;
}

[[nodiscard]] StrToRealCallExpr Parser::parse_str_to_real_call_expr()
{
	StrToRealCallExpr str_to_real_call_expr{};

	try_consume(TokenType::STRING_TO_REAL);
	try_consume(TokenType::O_PAREN);

	str_to_real_call_expr.m_str_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return str_to_real_call_expr;
}

[[nodiscard]] IntToStrCallExpr Parser::parse_int_to_str_call_expr()
{
	IntToStrCallExpr int_to_str_call_expr{};

	try_consume(TokenType::INT_TO_STRING);
	try_consume(TokenType::O_PAREN);

	int_to_str_call_expr.m_int_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return int_to_str_call_expr;
}

[[nodiscard]] RealToStrCallExpr Parser::parse_real_to_str_call_expr()
{
	RealToStrCallExpr real_to_str_call_expr{};

	try_consume(TokenType::REAL_TO_STRING);
	try_consume(TokenType::O_PAREN);

	real_to_str_call_expr.m_real_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return real_to_str_call_expr;
}

[[nodiscard]] CharToCodeCallExpr Parser::parse_char_to_code_call_expr()
{
	CharToCodeCallExpr char_to_code_call_expr{};

	try_consume(TokenType::CHAR_TO_CODE);
	try_consume(TokenType::O_PAREN);

	char_to_code_call_expr.m_char_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return char_to_code_call_expr;
}

[[nodiscard]] CodeToCharCallExpr Parser::parse_code_to_char_call_expr()
{
	CodeToCharCallExpr code_to_char_call_expr{};

	try_consume(TokenType::CODE_TO_CHAR);
	try_consume(TokenType::O_PAREN);

	code_to_char_call_expr.m_int_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return code_to_char_call_expr;
}

[[nodiscard]] RandomIntCallExpr Parser::parse_random_int_call_expr()
{
	RandomIntCallExpr random_int_call_expr{};

	try_consume(TokenType::RANDOM_INT);
	try_consume(TokenType::O_PAREN);

	random_int_call_expr.m_int1_expr = parse_expr();

	try_consume(TokenType::COMMA);

	random_int_call_expr.m_int2_expr = parse_expr();

	try_consume(TokenType::C_PAREN);

	return random_int_call_expr;
}

[[nodiscard]] Body Parser::parse_body_until(const std::initializer_list<TokenType>& stop_tokens)
{
	/* Create a list of statement objects */
	std::vector<Stmt> stmts{};

	/* Loop until the current token is not of any types listed in the initializer list */
	while (std::find(stop_tokens.begin(), stop_tokens.end(), peek().m_type) == stop_tokens.end())
	{
		/* Try to peek the current token, checking it's not end of file */
		try_peek();

		/* Parse statement and add it to the statement list */
		stmts.push_back(parse_stmt());
	}

	/* Return the statement list as type Body */
	return Body{stmts};
}

[[nodiscard]] Fields Parser::parse_fields_until(const std::initializer_list<TokenType>& stop_tokens)
{
	std::vector<FieldStmt> field_stmts{};

	while (std::find(stop_tokens.begin(), stop_tokens.end(), peek().m_type) == stop_tokens.end() &&
		   peek().m_type != TokenType::END_OF_FILE)
	{
		field_stmts.push_back(parse_field_stmt());
	}

	return Fields{field_stmts};
}

[[nodiscard]] std::vector<Expr> Parser::parse_cse()
{
	/* Create a list of expression objects */
	std::vector<Expr> cse{};

	/* Do until the type of the current token is not COMMA */
	do
	{
		/* Consume the current token */
		consume();

		/* Parse and store the expressions separated by commas */
		cse.push_back(parse_expr());

	} while (peek().m_type == TokenType::COMMA); /* Loop condition */

	/* Return the parsed comma separated expressions */
	return cse;
}

[[nodiscard]] Operator Parser::determine_op()
{
	if (peek(1).m_type == TokenType::END_OF_FILE)
	{
		ParseError("eof reached when looking for relational op");
	}

	if (peek().m_type == TokenType::PLUS)
	{
		consume();
		return Operator::PLUS;
	}
	else if (peek().m_type == TokenType::MINUS)
	{
		consume();
		return Operator::MINUS;
	}
	else if (peek().m_type == TokenType::MULTIPLY)
	{
		consume();
		return Operator::MULTIPLY;
	}
	else if (peek().m_type == TokenType::DIVIDE)
	{
		consume();
		return Operator::DIVIDE;
	}
	else if (peek().m_type == TokenType::DIV)
	{
		consume();
		return Operator::DIV;
	}
	else if (peek().m_type == TokenType::MOD)
	{
		consume();
		return Operator::MOD;
	}
	else if (peek().m_type == TokenType::LESS_THAN && peek(1).m_type != TokenType::EQUALS)
	{
		consume();
		return Operator::LESS_THAN;
	}
	else if (peek().m_type == TokenType::LESS_THAN && peek(1).m_type == TokenType::EQUALS)
	{
		consume(2);
		return Operator::LESS_THAN_OET;
	}
	else if (peek().m_type == TokenType::GREATER_THAN && peek(1).m_type != TokenType::EQUALS)
	{
		consume();
		return Operator::GREATER_THAN;
	}
	else if (peek().m_type == TokenType::GREATER_THAN && peek(1).m_type == TokenType::EQUALS)
	{
		consume(2);
		return Operator::GREATER_THAN_OET;
	}
	else if (peek().m_type == TokenType::EXCLAIMATION && peek(1).m_type == TokenType::EQUALS)
	{
		consume(2);
		return Operator::NOT_EQUALS;
	}
	else if (peek().m_type == TokenType::EQUALS)
	{
		consume();
		return Operator::EQUALS;
	}
	else if (peek().m_type == TokenType::AND)
	{
		consume();
		return Operator::AND;
	}
	else if (peek().m_type == TokenType::OR)
	{
		consume();
		return Operator::OR;
	}
	else if (peek().m_type == TokenType::NOT)
	{
		consume();
		return Operator::NOT;
	}
}

[[nodiscard]] const Token& Parser::peek(const std::size_t distance) const
{
	if (m_token_index + distance <= m_tokens.size())
	{
		ParseError
		{
			"peek offset went out of range",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	return m_tokens[m_token_index + distance];
}

const Token& Parser::try_peek(const std::size_t distance) const
{
	if (peek(distance).m_type == TokenType::END_OF_FILE)
	{
		ParseError
		{
			"unexpected end of line - cannot advance further",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}
	else
	{
		return peek(distance);
	}
}

const Token& Parser::consume(const std::size_t distance)
{
	if (m_token_index + distance <= m_tokens.size())
	{
		ParseError
		{
			"consume offset went out of range",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	m_token_index += distance;

	return m_tokens[m_token_index - distance];
}

const Token& Parser::try_consume(const TokenType type)
{
	if (peek().m_type != type)
	{
		ParseError
		{
			"expected '" + tt_to_string(type) + "' got '" + to_string(peek().m_type) + "'",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}
	else
	{
		return consume();
	}
}

void Parser::remove_var(const std::string_view name)
{
	for (auto it{m_existing_vars.begin()}; it != m_existing_vars.end(); it++) {
		if (it->m_name == name) {
			m_existing_vars.erase(it);
			break;
		}
	}
}

[[nodiscard]] bool Parser::is_var_defined(const std::string_view name)
{
	std::size_t stop_index{};

	if (!m_var_scope_stack.empty())
	{
		stop_index = m_var_scope_stack.back();
	}

	for (std::size_t i = m_existing_vars.size(); i-- > stop_index;)
	{
		if (m_existing_vars[i].m_name == name)
		{
			return true;
		}
	}

	return false;
}

[[nodiscard]] VarStmt* Parser::existing_var_lookup(const std::string_view name)
{
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++)
	{
		if (it->get()->m_name == name)
		{
			return it->get();
		}
	}

	ParseError("couldn't match name with existing variables");
}

[[nodiscard]] FuncDeclStmt* Parser::existing_func_lookup(const std::string_view name)
{
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
	{
		if (it->get()->m_name == name)
		{
			return it->get();
		}
	}

	ParseError("func doesnt exist");
}

[[nodiscard]] bool Parser::is_record(const std::string_view name)
{
	for (const auto& i : m_existing_records)
	{
		if (i.m_name == name)
		{
			return true;
		}
	}

	return false;
}

[[nodiscard]] bool Parser::is_stdlib(const std::string_view name)
{
	return name == "LEN" ||
		   name == "POSITION" ||
		   name == "SUBSTRING" ||
		   name == "STRING_TO_INT" ||
		   name == "STRING_TO_REAL" ||
		   name == "INT_TO_STRING" ||
		   name == "REAL_TO_STRING" ||
		   name == "CHAR_TO_CODE" ||
		   name == "CODE_TO_CHAR" ||
		   name == "RANDOM_INT";
}

[[nodiscard]] bool Parser::is_stmt(TokenType token_type)
{
	return token_type == TokenType::IF ||
		   token_type == TokenType::OUTPUT ||
		   token_type == TokenType::CONSTANT ||
		   token_type == TokenType::SUB_ROUTINE;
}

void Parser::deduce_func_decl_param_types_from_args(const std::string_view name, const Args& args)
{
	FuncDeclStmt& func_decl{existing_func_lookup(name)};

	if (!func_decl.m_is_called)
	{
		for (std::size_t i{}; i < func_decl.m_params.m_params.size(); i++)
		{
			func_decl.m_params.m_params[i].m_type = args.m_exprs[i].m_type;
		}

		func_decl.m_is_called = true;
	}
}

[[nodiscard]] DataType Parser::deduce_expr_type(const Token token)
{
	if (token.m_type == TokenType::REAL || token.m_type == TokenType::INT ||
	    token.m_type == TokenType::STRING || token.m_type == TokenType::NOT)
	{
		return tt_to_dt(token.m_type);
	}
	else if (token.m_type == TokenType::IDENTIFIER)
	{
		return existing_var_lookup(token.m_value).m_expr->m_type;
	}
	else if (token.m_type == TokenType::USER_INPUT)
	{
		return DataType::STRING;
	}
	else
	{
		ParseError("couldn't match expression type");
	}
}

[[nodiscard]] DataType Parser::deduce_field_type_from_access(Token name, Token field)
{
	const std::string_view record_name{existing_var_lookup(name.m_value).m_record_name};

	for (const auto& i : m_existing_records)
	{
		if (i.m_name == record_name)
		{
			for (const auto& j : i.m_fields)
			{
				if (j.m_name == field.m_value)
				{
					return j.m_type;
				}
			}
		}
	}
}

[[nodiscard]] DataType Parser::tt_to_dt(const Token token)
{
	if (token.m_type == TokenType::REAL || token.m_type == TokenType::REAL_TYPE)
	{
		return DataType::REAL;
	}
	else if (token.m_type == TokenType::INT || token.m_type == TokenType::INT_TYPE)
	{
		return DataType::INT;
	}
	else if (token.m_type == TokenType::STRING || token.m_type == TokenType::STRING_TYPE)
	{
		return DataType::STRING;
	}
	ParseError("no match type");
}

[[nodiscard]] bool Parser::is_bin_op(TokenType token_type)
{
	return token_type == TokenType::ADDITION ||
		   token_type == TokenType::SUBTRACTION ||
		   token_type == TokenType::MULTIPLICATION ||
		   token_type == TokenType::DIVISION ||
		   token_type == TokenType::DIV ||
		   token_type == TokenType::MOD ||
		   token_type == TokenType::LESS_THAN ||
		   token_type == TokenType::GREATER_THAN ||
		   token_type == TokenType::EQUALS ||
		   token_type == TokenType::EXCLAMATION ||
		   token_type == TokenType::OR ||
		   token_type == TokenType::AND;
}

[[nodiscard]] bool Parser::is_unary(TokenType token_type)
{
	return token_type == TokenType::NOT ||
	       token_type == TokenType::SUBTRACTION;
}

void Parser::populate_stdlib_funcs()
{
	add_stdlib_func("LEN",            {DataType::STRING}, DataType::INT);
	add_stdlib_func("POSITION",       {DataType::STRING, DataType::CHAR}, DataType::INT);
	add_stdlib_func("SUBSTRING",      {DataType::INT, DataType::INT, DataType::STRING}, DataType::STRING);
	add_stdlib_func("STRING_TO_INT",  {DataType::STRING}, DataType::INT);
	add_stdlib_func("STRING_TO_REAL", {DataType::STRING}, DataType::REAL);
	add_stdlib_func("INT_TO_STRING",  {DataType::INT}, DataType::STRING);
	add_stdlib_func("REAL_TO_STRING", {DataType::REAL}, DataType::STRING);
	add_stdlib_func("CHAR_TO_CODE",   {DataType::CHAR}, DataType::INT);
	add_stdlib_func("CODE_TO_CHAR",   {DataType::INT}, DataType::STRING);
	add_stdlib_func("RANDOM_INT",     {DataType::INT, DataType::INT}, DataType::INT);
}

void Parser::add_stdlib_func(const std::string_view name, const std::initializer_list<DataType>& param_types, const DataType return_type)
{
	std::unique_ptr<FuncDeclStmt> func{std::make_unique<FuncDeclStmt>()};

	func->m_name = name;

	for (const auto& param_type : param_types)
	{
		func->m_params.m_params.push_back(Param{"", param_type});
	}

	func->m_is_void = false;

	func->m_return.m_return_expr = std::make_unique<Expr>();
	func->m_return.m_return_expr->m_type = return_type;

	m_existing_funcs.push_back(std::move(func));
}