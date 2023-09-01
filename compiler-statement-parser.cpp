// compiler-statement-parser: 语句解析

#include "compiler-syntax-parser.h"

using namespace compiler;
using namespace compiler::statement_parser;

void compiler::statement_parser::node_value_identifier_replace(node_value* node, variable_scope& scope)
{
	if (node->node_type == node_value::node_identifier)
	{
		auto search = scope.search(node->identifier);

		if (search)
			node->val_identifier = search.value();
		else
			throw syntax_parsing_exception(std::format("No identifier found in current scope: \"{0}\".", node->identifier));
	}
	else
	{
		if (node->left) node_value_identifier_replace(node->left, scope);
		if (node->right) node_value_identifier_replace(node->right, scope);
	}
}

void compiler::statement_parser::parse_statement_block(token_list& list, size_t& index, std::vector<statement_wrapper>& block, variable_scope* var_scope)
{
	try
	{
		// 前导花括号
		if (!list.at(index++).match(token_type::left_bracket))
		{
			throw syntax_parsing_exception(list.at(index), "Statements should be enclosed by brackets");
		}

		// 解析括号内内容
		while (!list.at(index).match(token_type::right_bracket))
		{
			if (auto parse = parse_assignment(list, index); parse)
			{
				block.push_back(statement_wrapper(statement_wrapper::assign, parse.value()));
				continue;
			}

			if (auto parse = parse_declarement(list, index); parse)
			{
				auto decl = std::get<0>(parse.value()); // 变量声明
				auto assign = std::get<1>(parse.value()); // 变量赋值

				block.push_back(statement_wrapper(statement_wrapper::decl, decl));
				if (assign) block.push_back(statement_wrapper(statement_wrapper::assign, assign));

				continue;
			}

			if (auto parse = parse_if_branch(list, index); parse)
			{
				parse.value()->var_scope.parent = var_scope; // 母变量范围
				block.push_back(statement_wrapper(statement_wrapper::if_statement, parse.value()));
				continue;
			}

			throw syntax_parsing_exception(list.at(index), "Unrecognized statement.");
		}

		index++;
	}
	catch (std::exception)
	{
		throw syntax_parsing_exception("Parsing error, there's likely an incomplete statement in your code that caused subscription out of range.");
	}
}

std::optional<assignment*> compiler::statement_parser::parse_assignment(token_list& list, size_t& index)
{
	auto node = new assignment();

	if (list.at(index).match(token_type::identifier)) // 标识符
	{
		node->var_name = list.at(index++).literal;
	}
	else if (list.at(index).match(operand_type::get_address))
	{
		node->left = numeric_parser::parse_numeric_expression(list, ++index);
	}
	else
	{
		delete node; // 开头找不到有效字符
		return std::nullopt;
	}

	// 检测等号

	if (index >= list.size()) throw syntax_parsing_exception("Invalid assignment statement.");

	if (!list.at(index).match(operand_type::set_val))
	{
		delete node; // 开头找不到有效字符
		throw syntax_parsing_exception(list.at(index), "Invalid assignment statement.");
	}

	node->right = numeric_parser::parse_numeric_expression(list, ++index, 0);

	// 检查结尾
	if (index < list.size())
		if (list.at(index).match(token_type::semicolon))
		{
			// 有效
			index++;
			return node;
		}

	// 无效，报错
	delete node;
	throw syntax_parsing_exception(list.at(index).character, list.at(index).line, 
		"Invalid assignment statement. End the statement with \";\".");
}

std::optional<std::tuple<declarement*, assignment*>> compiler::statement_parser::parse_declarement(token_list& list, size_t& index, bool global)
{
	// 数据类型
	if (!list.at(index).match(token_type::number_type))
		return std::nullopt;

	declarement* decl = new declarement();
	decl->type = list.at(index).number_type;
	decl->name = list.at(index).literal;

	// 变量名称
	if (++index >= list.size()) throw syntax_parsing_exception("Invalid variable declarement."); // 越界检查

	if (!list.at(index).match(token_type::identifier))
		throw syntax_parsing_exception(list.at(index), "Invalid variable declarement.");

	auto& tok = list.at(index); // 变量名
	
	assignment* ret = nullptr;

	// 是否自带赋值
	if (++index >= list.size()) throw syntax_parsing_exception("Invalid variable declarement."); // 越界检查

	if (list.at(index).match(operand_type::set_val))
	{
		ret = new assignment();

		// 解析算式
		ret->right = numeric_parser::parse_numeric_expression(list, ++index);

		// 全局变量不允许复杂初始化
		if (ret->right->node_type != node_value::node_constant && global)
			throw syntax_parsing_exception(list.at(index), "Invalid global variable initialization.");
	}

	if (index >= list.size()) throw syntax_parsing_exception("Invalid variable declarement."); // 越界检查

	// 检查尾：分号
	if (!list.at(index).match(token_type::semicolon))
	{
		if (ret) delete ret;
		throw syntax_parsing_exception(list.at(index), "Invalid variable declarement.");
	}

	index++;

	return std::tuple(decl, ret);
}

std::optional<if_branch*> compiler::statement_parser::parse_if_branch(token_list& list, size_t& index)
{
	if (!list.at(index).match(keyword_type::kw_if))
	{
		return std::nullopt;
	}

	if (!list.at(++index).match(token_type::left_parentheses))
	{
		throw syntax_parsing_exception(list.at(index), "Invalid if-branch grammar.");
	}

	auto condition = numeric_parser::parse_numeric_expression(list, ++index);

	if (!list.at(index).match(token_type::right_parentheses))
	{
		throw syntax_parsing_exception(list.at(index), "Invalid if-branch grammar.");
	}

	auto branch_instance = new if_branch();

	branch_instance->condition = condition; // 条件
	parse_statement_block(list, ++index, branch_instance->body, &branch_instance->var_scope); // 解析语句主体

	// else部分解析
	if(index < list.size()) if (list.at(index).match(keyword_type::kw_else))
	{
		branch_instance->else_var_scope = variable_scope(); // 启用else变量空间

		// 解析else主体
		parse_statement_block(list, ++index, branch_instance->else_body, &branch_instance->else_var_scope.value());
	}

	return branch_instance;
}
