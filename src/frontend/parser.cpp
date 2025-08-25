/* frontend/parser.cpp by David Filiks */
/* The parser implementation for the PsL compiler */

#include "parser.hpp"

Parser::Parser(const Lexer& lexer)
/* Initialize private members */
: m_tokens(lexer.get_tokens()),
  m_source(lexer.get_source()),
  m_source_path(lexer.get_source_path()) {}

void Parser::execute()
{
	/* Execute the parse() function */
	parse();
}

void Parser::parse()
{
	/* Add the standard function definition statements onto the beginning of AST */
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
	/* Return the AST */
	return m_ast;
}

[[nodiscard]] const std::vector<Token>& Parser::get_tokens() const
{
	/* Return the used token stream */
	return m_tokens;
}

[[nodiscard]] const std::size_t& Parser::get_token_index() const
{
	/* Return the current index within the token stream */
	return m_token_index;
}

[[nodiscard]] const std::string& Parser::get_source() const
{
	/* Return the source contents */
	return m_source;
}

[[nodiscard]] const std::string& Parser::get_source_path() const
{
	/* Return the source path */
	return m_source_path;
}

[[nodiscard]] const std::vector<std::shared_ptr<VarStmt>>& Parser::get_existing_vars() const
{
	/* Return the existing variables list */
	return m_existing_vars;
}

[[nodiscard]] const std::vector<std::shared_ptr<FuncDefStmt>>& Parser::get_existing_funcs() const
{
	/* Return the existing functions list */
	return m_existing_funcs;
}

[[nodiscard]] const std::vector<std::shared_ptr<RecordStmt>>& Parser::get_existing_records() const
{
	/* Return the existing records list */
	return m_existing_records;
}

[[nodiscard]] const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& Parser::get_unresolved_exprs() const
{
	/* Return the unresolved expressions list */
	return m_unresolved_exprs;
}

[[nodiscard]] const std::vector<std::pair<std::shared_ptr<Expr>, const std::string>>& Parser::get_unresolved_decls() const
{
	/* Return the unresolved declarations list */
	return m_unresolved_exprs;
}

[[nodiscard]] const Stack<std::size_t>& Parser::get_var_scope_stack() const
{
	/* Return the variable scope stack */
	return m_var_scope_stack;
}

[[nodiscard]] Stmt Parser::parse_stmt()
{
	/* Switch through all statements we can deduce from only peeking at the current token */
	switch (peek().m_type)
	{
		/* If the current token type is CONSTANT */
		case (TokenType::CONSTANT):
			/* Parse the remainder as a variable statement */
			/* This is because only variables can be prefixed with CONSTANT */
			return Stmt{parse_var_stmt()};

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
			/* Parse the remainder as a function definition statement */
			return Stmt{parse_func_def_stmt()};

		/* If the current token type is SUB_ROUTINE */
		case (TokenType::RETURN):
			/* Parse the remainder as a return statement */
			return Stmt{parse_return_stmt()};

		/* In the case of no matches */
		default:
			/* Fall through in order to possibly deduce using more tokens */
			break;
	}

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
				return Stmt{parse_var_stmt()};

			/* If the next token is O_PAREN */
			case (TokenType::O_PAREN):
				/* Parse the remainder as a function call statement */
				return Stmt{parse_func_call_stmt()};

			/* In the case of no matches */
			default:
				/* Fall through to error */
				break;
		}
	}

	/* If the current token type is ELSE */
	if (peek().m_type == TokenType::ELSE)
	{
		/* If the next token is IF and previous token is on the same row (line) */
		if (peek(1).m_type == TokenType::IF && peek().m_row == peek(1).m_row)
		{
			/* Parse the remainder as an else if statement */
			return Stmt{parse_else_if_stmt()};
		}

		/* If the next token is not IF or not on the same row (line) */
		else
		{
			/* Parse the remainder as an else statement */
			return Stmt{parse_else_stmt()};
		}
	}

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
	/* Throw parse error */
	throw ParseError
	{
		"no matching statement found for '" + std::string{tt_to_string(peek().m_type)} + "'",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};

	/* Make the compiler stop giving warnings due to possible no return */
	return Stmt{};
}

[[nodiscard]] VarStmt Parser::parse_var_stmt()
{
	/* Create a new variable statement pointer */
	std::shared_ptr<VarStmt> var_stmt{std::make_shared<VarStmt>()};

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

		/* If the token type after the SQ_O_BRACKET is also SQ_O_BRACKET */
		if (peek(1).m_type == TokenType::SQ_O_BRACKET)
		{
			/* Safely assume that the user is creating a two dimensional list */
			var_stmt->m_is_2d_list = true;

			/* If the there are three tokens in a row with a type of SQ_O_BRACKET */
			if (peek(2).m_type == TokenType::SQ_O_BRACKET)
			{
				/* Indicates that the user is trying to create a three dimensional list */
				/* E.g. list <- [[[...]]] */
				/* Throw parse error */
				throw ParseError
				{
					"only one and two dimensional lists are supported, not > 2D",
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
		var_stmt->m_previous_expr = existing_var_lookup(var_stmt->m_name)->m_expr;

		/* Set reassignment boolean to true, which helps in the generation stage */
		var_stmt->m_is_reassignment = true;

		/* Parse and set the new variable expression */
		var_stmt->m_expr = parse_expr();
	}

	/* If the variable was not previously defined */
	else
	{
		/* Parse the variable expression */
		var_stmt->m_expr = parse_expr();

		/* Append the variable pointer to the existing variables array */
		m_existing_vars.push_back(var_stmt);
	}

	/* Return the variable statement */
	return *var_stmt;
}

[[nodiscard]] FieldAccessStmt Parser::parse_field_access_stmt()
{
	/* Create a new field access statement object */
	FieldAccessStmt field_access_stmt{};

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
	field_access_stmt.m_expr = parse_expr();

	/* Return the field access statement */
	return field_access_stmt;
}

[[nodiscard]] OutputStmt Parser::parse_output_stmt()
{
	/* Create a new output statement object */
	OutputStmt output_stmt{};

	/* No consume of the OUTPUT token as it is done in parse_cse() */
	/* Parse and store the output statement arguments */
	output_stmt.m_args.m_args = parse_cse<Arg>();

	/* Return the output statement */
	return output_stmt;
}

[[nodiscard]] FuncDefStmt Parser::parse_func_def_stmt()
{
	/* Create a new function definition statement pointer */
	std::shared_ptr<FuncDefStmt> func_def_stmt{std::make_shared<FuncDefStmt>()};

	/* Consume the SUBROUTINE token */
	consume();

	/* Try to consume and store a function name */
	func_def_stmt->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse the function definition parameters */
	func_def_stmt->m_params = parse_func_decl_params();

	/* Consume a C_PAREN token */
	consume();

	/* Save index position and skip over function body to parse it on the second pass */
	func_def_stmt->m_token_index_start = m_token_index;
	skip_over_function_body();

	/* Consume a ENDSUBROUTINE token */
	consume();

	/* Append the function to the existing functions list */
	m_existing_funcs.push_back(func_def_stmt);

	/* Return the function definition statement */
	return *func_def_stmt;
}

[[nodiscard]] ReturnStmt Parser::parse_return_stmt()
{
	/* Create a new return statement object */
	ReturnStmt return_stmt{};

	/* Consume the RETURN token */
	consume();

	/* Parse and store the return expression */
	return_stmt.m_return_expr = parse_expr();

	/* Set the return statement expression of the current function to the parsed return statement expression */
	m_existing_funcs.back()->m_return.m_return_expr = return_stmt.m_return_expr;

	/* Return the return statement */
	return return_stmt;
}

[[nodiscard]] FuncCallStmt Parser::parse_func_call_stmt()
{
	/* Create a new function call statement object */
	FuncCallStmt func_call_stmt{};

	/* Consume and store the function name */
	func_call_stmt.m_name = consume().m_value;

	/* Parse the function call arguments */
	func_call_stmt.m_args.m_args = parse_cse<Arg>();

	/* Try consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Check that the function has been declared */
	existing_func_lookup(func_call_stmt.m_name);

	/* Check that the function call argument count matches up with the function definition */
	check_arg_count_matches(func_call_stmt.m_name, func_call_stmt.m_args);

	/* Deduce the types of the function definition parameters from the types of the arguments used in this call */
	deduce_func_decl_param_types_from_args(func_call_stmt.m_name, func_call_stmt.m_args);

	/* Return the function call statement */
	return func_call_stmt;
}

[[nodiscard]] RepeatUntilStmt Parser::parse_repeat_until_stmt()
{
	/* Create a new repeat until statement object */
	RepeatUntilStmt repeat_until_stmt{};

	/* Consume a token of type REPEAT */
	consume();

	/* Parse the repeat until statement body */
	repeat_until_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::UNTIL}));

	/* No try_consume() used here as the function above exits upon a UNTIL token */
	/* Consume a token of type UNTIL */
	consume();

	/* Parse and store the condition expression */
	repeat_until_stmt.m_condition_expr = parse_expr();

	/* Return the repeat until statement */
	return repeat_until_stmt;
}

[[nodiscard]] WhileStmt Parser::parse_while_stmt()
{
	/* Create a new while statement object */
	WhileStmt while_stmt{};

	/* Consume a token of type WHILE */
	consume();

	/* Parse and store the condition expression */
	while_stmt.m_condition_expr = parse_expr();

	/* Parse and store the while statement body */
	while_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::END_WHILE}));

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

	/* Consume a token of type IF */
	consume();

	/* Parse and store the condition expression */
	if_stmt.m_condition_expr = parse_expr();

	/* Try to consume a token of type THEN */
	try_consume(TokenType::THEN);

	/* Parse and store the if statement body */
	/* An if statement can end in two ways, either with ENDIF or ELSE IF/ELSE */
	if_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::ELSE, TokenType::END_IF}));

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

	/* Consume the ELSE and IF tokens */
	consume(2);

	/* Parse and store the condition expression */
	else_if_stmt.m_condition_expr = parse_expr();

	/* Try to consume a token of type THEN */
	try_consume(TokenType::THEN);

	/* Parse and store the else if statement body */
	else_if_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::ELSE, TokenType::END_IF}));

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

	/* Consume an ELSE token */
	consume();

	/* Parse and store the else statement body */
	else_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::END_IF}));

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

	/* Consume a FOR token */
	consume();

	/* Parse and set a variable statement as part of the condition */
	for_to_stmt.m_var_stmt = parse_var_stmt();

	/* Try to consume a token of type TO */
	try_consume(TokenType::TO);

	/* Parse and store the loop boundary expression */
	for_to_stmt.m_boundary = parse_expr();

	/* If the current token is of type STEP */
	if (peek().m_type == TokenType::STEP)
	{
		/* Consume and advance past the current token */
		consume();

		/* Parse and store the step expression */
		for_to_stmt.m_step = parse_expr();
	}

	/* Parse and store the for to statement body */
	for_to_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::END_FOR}));

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

	/* Consume a FOR token */
	consume();

	/* Allocate heap memory for the loop declaration */
	for_in_stmt.m_declaration = std::make_shared<VarStmt>();

	/* If the reason for try_consume() here confuses you, see the implementation of parse_stmt() */
	for_in_stmt.m_declaration->m_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* No try_consume() for the same reason as before */
	/* Consume a IN token */
	consume();

	/* Store the range name for later use */
	const std::string_view range_name{peek().m_value};

	/* Parse and store the loop range expression */
	for_in_stmt.m_range = parse_expr();

	/* Allocate heap memory for the declaration's expression */
	for_in_stmt.m_declaration->m_expr = std::make_shared<Expr>();

	/* If the range type is unresolved */
	if (for_in_stmt.m_range->m_type == DataType::UNRESOLVED)
	{
		/* This means that the declaration is also unresolved, and must be resolved later on */
		/* The declaration is paired to the range name as they need to be of the same type */
		m_unresolved_exprs.emplace_back(for_in_stmt.m_declaration->m_expr, range_name);
	}

	/* If the range type is known */
	else
	{
		/* Set the declaration type to the same type as the range */
		for_in_stmt.m_declaration->m_expr->m_type = for_in_stmt.m_range->m_type;
	}

	/* Add the declaration variable to the existing variables list */
	m_existing_vars.push_back(for_in_stmt.m_declaration);

	/* Parse and store the for in statement body */
	for_in_stmt.m_body = std::make_shared<Body>(parse_body_until({TokenType::END_FOR}));

	/* No try_consume() used here as the function above exits upon a ENDFOR token */
	/* Consume a token of type END_FOR */
	consume();

	/* Now that the loop is over, remove the declared variable, so that it cannot be used outside of the loop */
	remove_var(for_in_stmt.m_declaration->m_name);

	/* Return the for in statement */
	return for_in_stmt;
}

[[nodiscard]] RecordStmt Parser::parse_record_stmt()
{
	/* Create a new record statement pointer */
	std::shared_ptr<RecordStmt> record_stmt{std::make_shared<RecordStmt>()};

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
	m_existing_records.push_back(record_stmt);

	/* Return the record statement */
	return *record_stmt;
}

[[nodiscard]] FieldStmt Parser::parse_field_stmt()
{
	/* Create a new field statement object */
	FieldStmt field_stmt{};

	/* Consume an IDENTIFIER token */
	field_stmt.m_name = consume().m_value;

	/* No try_consume() used for the same reason as before */
	/* Consume a COLON token */
	consume();

	/* Set the field to a valid data type */
	/* tt_to_dt() errors out if the token is unable to represent a valid data type */
	field_stmt.m_type = tt_to_dt(consume().m_type);

	/* Return the field statement */
	return field_stmt;
}

[[nodiscard]] ListAccessStmt Parser::parse_list_access_stmt()
{
	/* Create a new list access statement object */
	ListAccessStmt list_access_stmt{};

	/* Consume and store the name of the list */
	list_access_stmt.m_name = consume().m_value;

	/* No try_consume() use due to the same reason as before */
	/* Consume a token of type SQ_O_BRACKET */
	consume();

	/* Parse and store the access row expression */
	list_access_stmt.m_row = parse_expr();

	/* Try to consume a token of type SQ_C_BRACKET */
	try_consume(TokenType::SQ_C_BRACKET);

	/* If the list is a two dimensional list */
	if (existing_var_lookup(list_access_stmt.m_name)->m_is_2d_list)
	{
		/* Try to consume a token of type SQ_O_BRACKET */
		try_consume(TokenType::SQ_O_BRACKET);

		/* Parse and store the access col expression */
		list_access_stmt.m_col = parse_expr();

		/* Try to consume a token of type SQ_C_BRACKET */
		try_consume(TokenType::SQ_C_BRACKET);
	}

	/* Try to consume tokens indicating an left arrow "<-" symbol */
	try_consume(TokenType::LESS_THAN);
	try_consume(TokenType::SUBTRACTION);

	/* Parse the list index's new expression */
	list_access_stmt.m_expr = parse_expr();

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
				/* Throw parse error */
				throw ParseError
				{
					"expected parameter identifier, got '" + std::string{tt_to_string(peek().m_type)} + "'",
					m_tokens[m_token_index].m_row,
					m_tokens[m_token_index].m_col,
					m_source
				};
		}
	}

	/* Return the parameter list as type Params */
	return Params{params};
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
				std::shared_ptr<VarStmt> var_stmt{std::make_shared<VarStmt>()};

				/* Set the parameter name */
				var_stmt->m_name = param.m_name;

				/* Allocate heap memory for the variable expression */
				var_stmt->m_expr = std::make_shared<Expr>();

				/* Set the type of the expression to the parameter type */
				var_stmt->m_expr->m_type = param.m_type;

				/* Add the parameter to the existing variables */
				m_existing_vars.push_back(var_stmt);
			}

			/* Parse and store the function body */
			func->m_body = std::make_shared<Body>(parse_body_until({TokenType::END_SUB_ROUTINE}));

			/* If the function contains no return statement */
			if (!func->m_return.m_return_expr)
			{
				/* Allocate heap memory for the return expression */
				func->m_return.m_return_expr = std::make_shared<Expr>();

				/* Set the return expression type to void as nothing is returned */
				func->m_return.m_return_expr->m_type = DataType::VOID;
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
	for (auto& unresolved_expr : m_unresolved_exprs)
	{
		/* Resolve the type of the first element from the type of the second element */
		unresolved_expr.first->m_type = existing_func_lookup(unresolved_expr.second)->m_return.m_return_expr->m_type;
	}

	/* Loop through all of the unresolved declarations */
	for (auto& unresolved_decl : m_unresolved_decls)
	{
		/* Resolve the type of the first element from the type of the second element */
		unresolved_decl.first->m_type = existing_var_lookup(unresolved_decl.second)->m_expr->m_type;
	}
}

void Parser::check_arg_count_matches(const std::string_view name, const Args &args) const
{
	/* Create a pointer to the function definition statement with the matching name */
	std::shared_ptr<FuncDefStmt> func_def_stmt{existing_func_lookup(name)};

	/* If the function definition parameter count is not equal to the argument count */
	if (func_def_stmt->m_params.m_params.size() != args.m_args.size())
	{
		/* Throw parse error */
		throw ParseError
		{
			"expected '" +
			std::to_string(func_def_stmt->m_params.m_params.size()) +
			"' arguments" +
			", got '" +
			std::to_string(args.m_args.size()) +
			"' arguments",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}
}

[[nodiscard]] std::shared_ptr<Expr> Parser::parse_expr()
{
	/* Create the lhs expression object */
	std::shared_ptr<Expr> lhs{std::make_shared<Expr>()};

	/* If the type of the current token is SQ_O_BRACKET */
	if (peek().m_type == TokenType::SQ_O_BRACKET)
	{
		/* Create a list expression object */
		ListExpr list_expr{};

		/* No consume of the SQ_O_BRACKET token as it is done in parse_cse() */
		/* Parse the list expression as comma separated expressions */
		list_expr.m_list.m_list = parse_cse<Element>();

		/* Try to consume a token of type SQ_C_BRACKET */
		try_consume(TokenType::SQ_C_BRACKET);

		/* If the list expression is not empty */
		if (!list_expr.m_list.m_list.empty())
		{
			/* Set lhs type to the type of the first expression in the list expression */
			lhs->m_type = list_expr.m_list.m_list[0].m_expr->m_type;
		}

		/* If the list expression is empty */
		else
		{
			/* Set lhs type to UNRESOLVED */
			lhs->m_type = DataType::UNRESOLVED;
		}

		/* Set lhs to the list expression */
		*lhs = Expr{list_expr, lhs->m_type};
	}

	/* If the type of the current token is O_PAREN */
	else if (peek().m_type == TokenType::O_PAREN)
	{
		/* Create a parentheses expression object */
		ParenExpr paren_expr{};

		/* Consume the O_PAREN token */
		consume();

		/* Parse and store the parentheses expression */
		paren_expr.m_expr = parse_expr();

		/* Try to consume a token of type C_PAREN */
		try_consume(TokenType::C_PAREN);

		/* Set the lhs type to the type of the parentheses expression */
		lhs->m_type = paren_expr.m_expr->m_type;

		/* Set lhs to the parentheses expression */
		*lhs = Expr{paren_expr, lhs->m_type};
	}

	/* If the type of the current token indicates it is a unary operator */
	else if (is_unary(peek().m_type))
	{
		/* Create a unary operator expression object */
		UnaryOpExpr unary_op_expr{};

		/* Parse and store the unary operator expression */
		unary_op_expr = parse_unary_op_expr();

		/* Set the lhs type to the type of the unary operator expression */
		lhs->m_type = unary_op_expr.m_unary_expr->m_type;

		/* Set lhs to the unary operator expression */
		*lhs = Expr{unary_op_expr, lhs->m_type};
	}

	/* If the current and next token suggest a function call expression, a record, an access, or a standard library function */
	else if (peek().m_type == TokenType::IDENTIFIER && peek(1).m_type == TokenType::O_PAREN ||
	         peek(1).m_type == TokenType::DOT || is_stdlib(peek().m_value))
	{
		/* If the current token has a value that matches a record name */
		if (is_record(peek().m_value))
		{
			/* Set the lhs type to USER_DEFINED_TYPE */
			lhs->m_type = DataType::USER_DEFINED_TYPE;
		}

		/* If the current token has a value that matches a standard library function name */
		else if (is_stdlib(peek().m_value))
		{
			/* Set the lhs type to the return type of that standard library function */
			lhs->m_type = existing_func_lookup(peek().m_value)->m_return.m_return_expr->m_type;
		}

		/* If the token ahead is of type O_PAREN */
		else if (peek(1).m_type == TokenType::O_PAREN)
		{
			/* Set lhs type to UNRESOLVED */
			lhs->m_type = DataType::UNRESOLVED;

			/* Add lhs to the unresolved exprs list, as it is a function call */
			m_unresolved_exprs.emplace_back(lhs, peek().m_value);
		}

		/* If the token ahead is of type DOT */
		else if (peek(1).m_type == TokenType::DOT)
		{
			/* Deduce the field type from a field access */
			/* A field access expression is something such as - OUTPUT jake.age */
			lhs->m_type = deduce_field_type_from_access(peek(), peek(2));
		}

		/* Set the lhs to the atom expression */
		*lhs = Expr{parse_atom(), lhs->m_type};
	}

	/* If the expression type can be deduced easily */
	else
	{
		/* Deduce the expression type from the current token */
		lhs->m_type = deduce_expr_type(peek());

		/* Set the lhs to the atom expression */
		*lhs = Expr{parse_atom(), lhs->m_type};
	}

	/* Loop while the current token type indicates a binary operator expression */
	while (is_bin_op(peek().m_type))
	{
		/* Create a binary operator expression object */
		BinOpExpr bin_op_expr{};

		/* Set the binary operator expression lhs to the current lhs */
		bin_op_expr.m_lhs = std::make_shared<Expr>(*lhs);
	
		/* Parse and store the type of operator used */
		bin_op_expr.m_op = parse_op();

		/* Parse the rhs expression */
		bin_op_expr.m_rhs = parse_expr();

		/* Set the lhs to the binary operator expression */
		*lhs = Expr{bin_op_expr, lhs->m_type};
	}

	/* Return the lhs expression */
	return lhs;
}

[[nodiscard]] AtomExpr Parser::parse_atom()
{
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

	/* If the current token type is CHAR */
	else if (peek().m_type == TokenType::CHAR)
	{
		/* Parse the remainder as a character expression */
		return AtomExpr{parse_char_expr()};
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
	throw ParseError
	{
		"no matching atom expression found for '" + std::string{tt_to_string(peek().m_type)} + "'",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};

	/* Make the compiler stop giving warnings due to possible no return */
	return AtomExpr{};
}

[[nodiscard]] IntExpr Parser::parse_int_expr()
{
	/* Return an integer expression, constructed using the current token value casted to an integer */
	return IntExpr{std::stoi(consume().m_value)};
}

[[nodiscard]] RealExpr Parser::parse_real_expr()
{
	/* Return a real expression, constructed using the current token value casted to a double */
	return RealExpr{std::stod(consume().m_value)};
}

[[nodiscard]] StrExpr Parser::parse_str_expr()
{
	/* Return a string expression, constructed using the current token value */
	return StrExpr{consume().m_value};
}

[[nodiscard]] CharExpr Parser::parse_char_expr()
{

	/* Return a character expression, constructed using the current token's value at index zero */
	return CharExpr{consume().m_value[0]};
}

[[nodiscard]] VarExpr Parser::parse_var_expr()
{
	/* Return a variable expression, constructed using the current token value as the identifier */
	return VarExpr{consume().m_value};
}

[[nodiscard]] UnaryOpExpr Parser::parse_unary_op_expr()
{
	/* Create a new unary operator expression object */
	UnaryOpExpr unary_op_expr{};

	/* Parse and store the type of operator used */
	unary_op_expr.m_op = parse_op();

	/* Parse the unary expression that comes after the operator */
	unary_op_expr.m_unary_expr = parse_expr();

	/* Return the unary operator expression */
	return unary_op_expr;
}

[[nodiscard]] FieldAccessExpr Parser::parse_field_access_expr()
{
	/* Create a new field access expression object */
	FieldAccessExpr field_access_expr{};

	/* Consume and store the field access expression variable name */
	field_access_expr.m_name = consume().m_value;

	/* Consume a token of type DOT */
	consume();

	/* Consume and store the name of the field that the programmer is trying access */
	field_access_expr.m_field_name = try_consume(TokenType::IDENTIFIER).m_value;

	/* Return the field access expression */
	return field_access_expr;
}

[[nodiscard]] ListAccessExpr Parser::parse_list_access_expr()
{
	/* Create a new list access expression object */
	ListAccessExpr list_access_expr{};

	/* Consume and store the name of the list */
	list_access_expr.m_name = consume().m_value;

	/* Consume a token of type SQ_O_BRACKET */
	consume();

	/* Parse and store the row access expression */
	list_access_expr.m_row = parse_expr();

	/* Try to consume a token of type SQ_C_BRACKET */
	try_consume(TokenType::SQ_C_BRACKET);

	/* If the list being accessed is two dimensional and the current token is of type SQ_O_BRACKET*/
	if (existing_var_lookup(list_access_expr.m_name)->m_is_2d_list && peek().m_type == TokenType::SQ_O_BRACKET)
	{
		/* Consume the SQ_O_BRACKET token */
		consume();

		/* Parse and store the col access expression */
		list_access_expr.m_col = parse_expr();

		/* Try to consume a token of type SQ_C_BRACKET */
		try_consume(TokenType::SQ_C_BRACKET);
	}

	/* Return the list access expression */
	return list_access_expr;
}

[[nodiscard]] FuncCallExpr Parser::parse_func_call_expr()
{
	/* Create a new function call expression object */
	FuncCallExpr func_call_expr{};

	/* Consume and store the name of the called function */
	func_call_expr.m_name = consume().m_value;

	/* Parse and store the function call arguments */
	func_call_expr.m_args.m_args = parse_cse<Arg>();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Check that the function has been declared */
	existing_func_lookup(func_call_expr.m_name);

	/* Check that the function call argument count matches up with the function definition */
	check_arg_count_matches(func_call_expr.m_name, func_call_expr.m_args);

	/* Deduce the types of the function definition parameters from the types of the arguments used in this call */
	deduce_func_decl_param_types_from_args(func_call_expr.m_name, func_call_expr.m_args);

	/* Return the function call expression */
	return func_call_expr;
}

[[nodiscard]] UserInputExpr Parser::parse_user_input_expr()
{
	/* Consume a USERINPUT token */
	consume();

	/* Return a new user input expression object */
	return UserInputExpr{};
}

[[nodiscard]] ObjectCreationExpr Parser::parse_object_creation_expr()
{
	/* Create a new object creation expression object */
	ObjectCreationExpr object_creation_expr{};

	/* Consume and store the name of the record */
	object_creation_expr.m_record_name = consume().m_value;

	/* Parse and store the object creation expression arguments */
	object_creation_expr.m_args.m_args = parse_cse<Arg>();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the object creation expression */
	return object_creation_expr;
}

[[nodiscard]] LenCallExpr Parser::parse_len_call_expr()
{
	/* Create a new LEN() expression object */
	LenCallExpr len_call_expr{};

	/* Consume the LEN token */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the argument expression */
	len_call_expr.m_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the LEN() expression */
	return len_call_expr;
}

[[nodiscard]] PositionCallExpr Parser::parse_position_call_expr()
{
	/* Create a new POSITION() expression object */
	PositionCallExpr position_call_expr{};

	/* Consume the POSITION token */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	position_call_expr.m_str_expr = parse_expr();

	/* Try to consume a token of type COMMA */
	try_consume(TokenType::COMMA);

	/* Parse and store the second argument expression */
	position_call_expr.m_char_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the POSITION() expression */
	return position_call_expr;
}

[[nodiscard]] SubStrCallExpr Parser::parse_sub_str_call_expr()
{
	/* Create a new SUBSTRING() expression object */
	SubStrCallExpr sub_str_call_expr{};

	/* Consume a token of type SUBSTRING */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	sub_str_call_expr.m_num1_expr = parse_expr();

	/* Try to consume a token of type COMMA */
	try_consume(TokenType::COMMA);

	/* Parse and store the second argument expression */
	sub_str_call_expr.m_num2_expr = parse_expr();

	/* Try to consume a token of type COMMA */
	try_consume(TokenType::COMMA);

	/* Parse and store the third argument expression */
	sub_str_call_expr.m_str_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the SUBSTRING() expression */
	return sub_str_call_expr;
}

[[nodiscard]] StrToIntCallExpr Parser::parse_str_to_int_call_expr()
{
	/* Create a new STRING_TO_INT() expression object */
	StrToIntCallExpr str_to_int_call_expr{};

	/* Consume a token of type STRING_TO_INT */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	str_to_int_call_expr.m_str_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the STRING_TO_INT() expression */
	return str_to_int_call_expr;
}

[[nodiscard]] StrToRealCallExpr Parser::parse_str_to_real_call_expr()
{
	/* Create a new STRING_TO_REAL() expression object */
	StrToRealCallExpr str_to_real_call_expr{};

	/* Consume a token of type STRING_TO_REAL */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	str_to_real_call_expr.m_str_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the STRING_TO_REAL() expression */
	return str_to_real_call_expr;
}

[[nodiscard]] IntToStrCallExpr Parser::parse_int_to_str_call_expr()
{
	/* Create a new INT_TO_STRING() expression object */
	IntToStrCallExpr int_to_str_call_expr{};

	/* Consume a token of type INT_TO_STRING */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	int_to_str_call_expr.m_int_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the INT_TO_STRING() expression */
	return int_to_str_call_expr;
}

[[nodiscard]] RealToStrCallExpr Parser::parse_real_to_str_call_expr()
{
	/* Create a new REAL_TO_STRING() expression object */
	RealToStrCallExpr real_to_str_call_expr{};

	/* Consume a token of type REAL_TO_STRING */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	real_to_str_call_expr.m_real_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the REAL_TO_STRING() expression */
	return real_to_str_call_expr;
}

[[nodiscard]] CharToCodeCallExpr Parser::parse_char_to_code_call_expr()
{
	/* Create a new CHAR_TO_CODE() expression object */
	CharToCodeCallExpr char_to_code_call_expr{};

	/* Consume a token of type CHAR_TO_CODE */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	char_to_code_call_expr.m_char_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the CHAR_TO_CODE() expression */
	return char_to_code_call_expr;
}

[[nodiscard]] CodeToCharCallExpr Parser::parse_code_to_char_call_expr()
{
	/* Create a new CODE_TO_CHAR() expression object */
	CodeToCharCallExpr code_to_char_call_expr{};

	/* Consume a token of type CODE_TO_CHAR */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	code_to_char_call_expr.m_int_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the CODE_TO_CHAR() expression */
	return code_to_char_call_expr;
}

[[nodiscard]] RandomIntCallExpr Parser::parse_random_int_call_expr()
{
	/* Create a new RANDOM_INT() expression object */
	RandomIntCallExpr random_int_call_expr{};

	/* Consume a token of type RANDOM_INT */
	consume();

	/* Try to consume a token of type O_PAREN */
	try_consume(TokenType::O_PAREN);

	/* Parse and store the first argument expression */
	random_int_call_expr.m_int1_expr = parse_expr();

	/* Try to consume a token of type COMMA */
	try_consume(TokenType::COMMA);

	/* Parse and store the second argument expression */
	random_int_call_expr.m_int2_expr = parse_expr();

	/* Try to consume a token of type C_PAREN */
	try_consume(TokenType::C_PAREN);

	/* Return the RANDOM_INT() expression */
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
	/* Create a list of field statement objects */
	std::vector<FieldStmt> field_stmts{};

	/* Loop until the current token is not of any types listed in the initializer list */
	while (std::find(stop_tokens.begin(), stop_tokens.end(), peek().m_type) == stop_tokens.end())
	{
		/* Try to peek the current token, checking it's not end of file */
		try_peek();

		/* Parse field statement and add it to the field statement list */
		field_stmts.push_back(parse_field_stmt());
	}

	/* Return the field statement list as type Fields */
	return Fields{field_stmts};
}

[[nodiscard]] Operator Parser::parse_op()
{
	/* If the current token type is ADDITION */
	if (peek().m_type == TokenType::ADDITION)
	{
		/* Consume the ADDITION token */
		consume();

		/* Return the ADDITION operator */
		return Operator::ADDITION;
	}

	/* If the current token type is SUBTRACTION */
	else if (peek().m_type == TokenType::SUBTRACTION)
	{
		/* Consume the SUBTRACTION token */
		consume();

		/* Return the SUBTRACTION operator */
		return Operator::SUBTRACTION;
	}

	/* If the current token type is MULTIPLICATION */
	else if (peek().m_type == TokenType::MULTIPLICATION)
	{
		/* Consume the MULTIPLICATION token */
		consume();

		/* Return the MULTIPLICATION operator */
		return Operator::MULTIPLICATION;
	}

	/* If the current token type is DIVISION */
	else if (peek().m_type == TokenType::DIVISION)
	{
		/* Consume the DIVISION token */
		consume();

		/* Return the DIVISION operator */
		return Operator::DIVISION;
	}

	/* If the current token type is DIV */
	else if (peek().m_type == TokenType::DIV)
	{
		/* Consume the DIV token */
		consume();

		/* Return the DIV operator */
		return Operator::DIV;
	}

	/* If the current token type is MOD */
	else if (peek().m_type == TokenType::MOD)
	{
		/* Consume the MOD token */
		consume();

		/* Return the MOD operator */
		return Operator::MOD;
	}

	/* If the current token type is EQUALS */
	else if (peek().m_type == TokenType::EQUALS)
	{
		/* Consume the EQUALS token */
		consume();

		/* Return the EQUALS operator */
		return Operator::EQUALS;
	}

	/* If the current token type is AND */
	else if (peek().m_type == TokenType::AND)
	{
		/* Consume the AND token */
		consume();

		/* Return the AND operator */
		return Operator::AND;
	}

	/* If the current token type is OR */
	else if (peek().m_type == TokenType::OR)
	{
		/* Consume the OR token */
		consume();

		/* Return the OR operator */
		return Operator::OR;
	}

	/* If the current token type is NOT */
	else if (peek().m_type == TokenType::NOT)
	{
		/* Consume the NOT token */
		consume();

		/* Return the NOT operator */
		return Operator::NOT;
	}

	/* If the current token type is LESS_THAN */
	if (peek().m_type == TokenType::LESS_THAN)
	{
		/* If the next token type is EQUALS */
		if (peek(1).m_type == TokenType::EQUALS)
		{
			/* Consume the LESS_THAN and EQUALS tokens */
			consume(2);

			/* Return the LESS_THAN_OR_EQUAL_TO operator */
			return Operator::LESS_THAN_OET;
		}

		/* If the next token type is not EQUALS */
		else
		{
			/* Consume the LESS_THAN token */
			consume();

			/* Return the LESS_THAN operator */
			return Operator::LESS_THAN;
		}
	}

	/* If the current token type is GREATER_THAN */
	else if (peek().m_type == TokenType::GREATER_THAN)
	{
		/* If the next token type is EQUALS */
		if (peek(1).m_type == TokenType::EQUALS)
		{
			/* Consume the GREATER_THAN and EQUALS tokens */
			consume(2);

			/* Return the GREATER_THAN_OR_EQUAL_TO operator */
			return Operator::GREATER_THAN_OET;
		}
		/* If the next token type is not EQUALS */
		else
		{
			/* Consume the GREATER_THAN token */
			consume();

			/* Return the GREATER_THAN operator */
			return Operator::GREATER_THAN;
		}
	}

	/* If the current token type is EXCLAMATION and if the next token type is EQUALS */
	else if (peek().m_type == TokenType::EXCLAMATION && peek(1).m_type == TokenType::EQUALS)
	{
		/* Consume the EXCLAMATION and EQUALS tokens */
		consume(2);

		/* Return the NOT_EQUALS operator */
		return Operator::NOT_EQUALS;
	}

	/* If no operator was found */
	/* Throw parse error */
	throw ParseError
	{
		"no matching operator found for '" + std::string{tt_to_string(peek().m_type)} + "'",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};

	/* Make the compiler stop giving warnings due to possible no return */
	return Operator{};
}

[[nodiscard]] const Token& Parser::peek(const std::size_t distance) const
{
	/* If peek offset is out of range */
	if (m_token_index + distance >= m_tokens.size())
	{
		/* Throw parse error */
		throw ParseError
		{
			"peek offset out of range",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	/* Return the token at the peek offset */
	return m_tokens[m_token_index + distance];
}

const Token& Parser::try_peek(const std::size_t distance) const
{
	/* If the token at the peek offset is END_OF_FILE */
	if (peek(distance).m_type == TokenType::END_OF_FILE)
	{
		/* Throw parse error */
		throw ParseError
		{
			"unexpected end of file reached",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	/* If the peeked token is not END_OF_FILE */
	return peek(distance); /* Return the token at the peek offset */
}

const Token& Parser::consume(const std::size_t distance)
{
	/* If consume offset is out of range */
	if (m_token_index + distance >= m_tokens.size())
	{
		/* Throw parse error */
		throw ParseError
		{
			"consume offset out of range",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	/* Advance the token index by the given distance */
	m_token_index += distance;

	/* Return the consumed token */
	return m_tokens[m_token_index - distance];
}

const Token& Parser::try_consume(const TokenType type)
{
	/* If the current token does not match the expected type */
	if (peek().m_type != type)
	{
		/* Throw parse error */
		throw ParseError
		{
			"expected '" + std::string{tt_to_string(type)} + "', got '" + std::string{tt_to_string(peek().m_type)} + "'",
			m_tokens[m_token_index].m_row,
			m_tokens[m_token_index].m_col,
			m_source
		};
	}

	/* If the current token matches the expected type */
	return consume(); /* Return the consumed token */
}

void Parser::remove_var(const std::string_view name)
{
	/* Loop through all the existing variables from start to end */
	for (auto it{m_existing_vars.begin()}; it != m_existing_vars.end(); it++)
	{
		/* If the current variable name matches the name specified */
		if (it->get()->m_name == name)
		{
			/* Remove the variable from the existing variables list */
			m_existing_vars.erase(it);

			/* Break from the loop to avoid unnecessary cycles */
			break;
		}
	}
}

[[nodiscard]] bool Parser::is_var_defined(const std::string_view name) const
{
	/* Create the loop stop index */
	std::size_t stop_index{};

	/* If the scope stack is not empty */
	if (!m_var_scope_stack.empty())
	{
		/* Set the stop index to the top stack element */
		stop_index = m_var_scope_stack.top();
	}

	/* Loop through the existing variables backwards until the stop index */
	/* Looping backwards in order to make scope management effective */
	for (std::size_t i = m_existing_vars.size(); i > stop_index; i--)
	{
		/* If the current variable name matches the name specified */
		if (m_existing_vars[i - 1]->m_name == name)
		{
			/* Return true to signify the variable is defined */
			return true;
		}
	}

	/* Return false to signify the variable is not defined */
	return false;
}

[[nodiscard]] std::shared_ptr<VarStmt> Parser::existing_var_lookup(const std::string_view name) const
{
	/* Loop backwards through the existing variables */
	/* This is because a variable is likely to be used close to where it is declared */
	for (auto it{m_existing_vars.rbegin()}; it != m_existing_vars.rend(); it++)
	{
		/* If the current variable name matches the name specified */
		if (it->get()->m_name == name)
		{
			/* Return the variable pointer */
			return *it;
		}
	}

	/* If nothing was found */
	/* Throw parse error */
	throw ParseError
	{
		"variable named '" + std::string{name} + "' could not be found",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};
}

std::shared_ptr<FuncDefStmt> Parser::existing_func_lookup(const std::string_view name) const
{
	/* Loop through the existing functions */
	for (auto it{m_existing_funcs.begin()}; it != m_existing_funcs.end(); it++)
	{
		/* If the current function name matches the name specified */
		if (it->get()->m_name == name)
		{
			/* Return the function pointer */
			return *it;
		}
	}

	/* If nothing was found */
	/* Throw parse error */
	throw ParseError
	{
		"function named '" + std::string{name} + "' could not be found",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};
}

[[nodiscard]] bool Parser::is_record(const std::string_view name) const
{
	/* Loop through all the existing records */
	for (const auto& record : m_existing_records)
	{
		/* If the current record name matches the name specified */
		if (record->m_name == name)
		{
			/* Return true to signify that a record of the name specified exists */
			return true;
		}
	}

	/* Return false to signify that a record of the name specified does not exist */
	return false;
}

[[nodiscard]] bool Parser::is_stdlib(const std::string_view name) const
{
	/* Return whether the name specified matches with the name of any standard library functions */
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

[[nodiscard]] bool Parser::is_data_type(const TokenType token_type) const
{
	/* Return whether the token type could be represented as a data type */
	return token_type == TokenType::REAL ||
	       token_type == TokenType::INT ||
	       token_type == TokenType::STRING ||
	       token_type == TokenType::CHAR;
}

void Parser::deduce_func_decl_param_types_from_args(const std::string_view name, const Args& args)
{
	/* Store a pointer to the looked up function definition statement */
	std::shared_ptr<FuncDefStmt> func_decl{existing_func_lookup(name)};

	/* If it is the first time that this function is called, deduce types */
	if (!func_decl->m_is_called)
	{
		/* Loop through all of the function parameters */
		for (std::size_t i{}; i < func_decl->m_params.m_params.size(); i++)
		{
			/* Set each parameter type to the corresponding call argument type */
			func_decl->m_params.m_params[i].m_type = args.m_args[i].m_expr->m_type;
		}

		/* Set boolean to true to indicate that this function was called */
		func_decl->m_is_called = true;
	}
}

[[nodiscard]] DataType Parser::deduce_expr_type(const Token token) const
{
	/* If the current token type could be represented as a data type */
	if (is_data_type(token.m_type))
	{
		/* Return the data type conversion of the current token */
		return tt_to_dt(token.m_type);
	}

	/* If the current token is of type IDENTIFIER */
	else if (token.m_type == TokenType::IDENTIFIER)
	{
		/* Return the corresponding variable's type */
		return existing_var_lookup(token.m_value)->m_expr->m_type;
	}

	/* If the current token is of type USER_INPUT */
	else if (token.m_type == TokenType::USER_INPUT)
	{
		/* Return a data type of STRING */
		/* This is because user input is treated as a string literal by default */
		return DataType::STRING;
	}

	/* If no type matches occured */
	/* Throw parse error */
	throw ParseError
	{
		"type of token '" + std::string{token.m_value} + "' could not be deduced",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};
}

[[nodiscard]] DataType Parser::deduce_field_type_from_access(const Token name, const Token field) const
{
	/* Store the name of the record, which is deduced by an existing variable lookup */
	const std::string_view record_name{existing_var_lookup(name.m_value)->m_record_name};

	/* Store the name of the field attempting to be accessed */
	const std::string_view field_name{field.m_value};

	/* Loop through all of the existing records */
	for (const auto& record : m_existing_records)
	{
		/* If the current record name matches the specified name */
		if (record->m_name == record_name)
		{
			/* Loop through all of that record's fields */
			for (const auto& field : record->m_fields.m_fields)
			{
				/* If the field name matches the specified name */
				if (field.m_name == field_name)
				{
					/* Return the type of that field */
					return field.m_type;
				}
			}
		}
	}

	/* If no matches are made */
	/* Throw parse error */
	throw ParseError
	{
		"field '" + std::string{field_name} + "' not found in record '" + std::string{record_name} + "'",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};
}

[[nodiscard]] DataType Parser::tt_to_dt(const TokenType token_type) const
{
	/* Switch through all of the possible token types that can be converted directly into a data type */
	switch (token_type)
	{
		/* If the current token is of type INT_TYPE */
		case (TokenType::INT_TYPE): /* Fall through to next case */

		/* If the current token is of type INT */
		case (TokenType::INT):
			/* Return a data type of INT */
			return DataType::INT;

		/* If the current token is of type REAL_TYPE */
		case (TokenType::REAL_TYPE): /* Fall through to next case */

		/* If the current token is of type REAL */
		case (TokenType::REAL):
			/* Return a data type of REAL */
			return DataType::REAL;

		/* If the current token is of type STRING_TYPE */
		case (TokenType::STRING_TYPE): /* Fall through to next case */

		/* If the current token is of type STRING */
		case (TokenType::STRING):
			/* Return a data type of STRING */
			return DataType::STRING;

		/* If the current token is of type CHAR_TYPE */
		case (TokenType::CHAR_TYPE): /* Fall through to next case */

		/* If the current token is of type CHAR */
		case (TokenType::CHAR):
			/* Return a data type of CHAR */
			return DataType::CHAR;

		/* If no matches are made */
		default:
			break; /* Fall through to error */
	}

	/* Throw parse error */
	throw ParseError
	{
		"token type '" + std::string{tt_to_string(token_type)} + "' cannot be converted to a data type",
		m_tokens[m_token_index].m_row,
		m_tokens[m_token_index].m_col,
		m_source
	};
}

[[nodiscard]] bool Parser::is_bin_op(const TokenType token_type) const
{
	/* Return whether the current token type indicates a binary operator is present */
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

[[nodiscard]] bool Parser::is_unary(const TokenType token_type) const
{
	/* Return whether the current token type indicates a unary operator is present */
	return token_type == TokenType::NOT ||
	       token_type == TokenType::SUBTRACTION;
}

void Parser::populate_stdlib_funcs()
{
	/* Add all of the standard library functions to the existing functions list */
	add_stdlib_func("LEN", {DataType::STRING}, DataType::INT);
	add_stdlib_func("POSITION", {DataType::STRING, DataType::CHAR}, DataType::INT);
	add_stdlib_func("SUBSTRING", {DataType::INT, DataType::INT, DataType::STRING}, DataType::STRING);
	add_stdlib_func("STRING_TO_INT", {DataType::STRING}, DataType::INT);
	add_stdlib_func("STRING_TO_REAL", {DataType::STRING}, DataType::REAL);
	add_stdlib_func("INT_TO_STRING", {DataType::INT}, DataType::STRING);
	add_stdlib_func("REAL_TO_STRING", {DataType::REAL}, DataType::STRING);
	add_stdlib_func("CHAR_TO_CODE", {DataType::CHAR}, DataType::INT);
	add_stdlib_func("CODE_TO_CHAR", {DataType::INT}, DataType::STRING);
	add_stdlib_func("RANDOM_INT", {DataType::INT, DataType::INT}, DataType::INT);
}

void Parser::add_stdlib_func(const std::string_view name, const std::initializer_list<DataType>& param_types, const DataType return_type)
{
	/* Create a new function definition pointer and allocate memory for it */
	std::shared_ptr<FuncDefStmt> func{std::make_shared<FuncDefStmt>()};

	/* Initialize the function name */
	func->m_name = name;

	/* Loop through all of the parameter types given */
	for (const auto& param_type : param_types)
	{
		/* Add them to the function parameter list */
		func->m_params.m_params.push_back(Param{"", param_type});
	}

	/* Allocate heap memory for the return expression */
	func->m_return.m_return_expr = std::make_shared<Expr>();

	/* Set the return expression type to the type provided */
	func->m_return.m_return_expr->m_type = return_type;

	/* Add the function to the existing functions list */
	m_existing_funcs.push_back(func);
}