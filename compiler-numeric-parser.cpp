// compiler-numeric-parser.cpp: 递归下降法解析数学表达式

#include "compiler-syntax-parser.h"

using namespace compiler;

node_value* compiler::numeric_parser::parse_numeric_expression(const token_list& list, size_t& index, size_t depth)
{
	try
	{
		auto node = parse_priority9(list, index, depth);

		if (index >= list.size())
			return node;

		if (list.at(index).match(token_type::right_parentheses) && depth != 0)
			index++;

		return node;
	}
	catch (std::exception)
	{
		throw syntax_parsing_exception(-1, -1, "Unable to parse the expression.");
	}
}

node_value* compiler::numeric_parser::parse_priority0(const token_list& list, size_t& index, size_t depth)
{
	return parse_single_operand<operand_type::get_address>(list, index, parse_number, depth);
}

node_value* compiler::numeric_parser::parse_priority1(const token_list& list, size_t& index, size_t depth)
{
	return parse_single_operand<operand_type::bit_not>(list, index, parse_priority0, depth);
}

node_value* compiler::numeric_parser::parse_priority2(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::mul, operand_type::div, operand_type::mod>(list, index, parse_priority1, depth);
}

node_value* compiler::numeric_parser::parse_priority3(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::add, operand_type::sub>(list, index, parse_priority2, depth);
}

node_value* compiler::numeric_parser::parse_priority4(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::bit_lsh, operand_type::bit_rsh>(list, index, parse_priority3, depth);
}

node_value* compiler::numeric_parser::parse_priority5(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::smaller, operand_type::smaller_or_equal, operand_type::larger, operand_type::larger_or_equal>(list, index, parse_priority4, depth);
}

node_value* compiler::numeric_parser::parse_priority6(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::equal, operand_type::not_equal>(list, index, parse_priority5, depth);
}

node_value* compiler::numeric_parser::parse_priority7(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::bit_and>(list, index, parse_priority6, depth);
}

node_value* compiler::numeric_parser::parse_priority8(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::bit_xor, operand_type::not_equal>(list, index, parse_priority7, depth);
}

node_value* compiler::numeric_parser::parse_priority9(const token_list& list, size_t& index, size_t depth)
{
	return parse_double_operand<operand_type::bit_or, operand_type::not_equal>(list, index, parse_priority8, depth);
}

node_value* compiler::numeric_parser::parse_number(const token_list& list, size_t& index, size_t depth)
{
	auto& tgt = list.at(index);

	switch (tgt.type)
	{
	case token_type::left_parentheses: // 左括号
		return parse_numeric_expression(list, ++index, depth + 1);

	case token_type::double_operand:
		if (tgt.op_type == operand_type::sub) // 带负号
		{
			auto ret = new node_value();
			ret->node_type = node_value::node_constant;

			auto& val_token = list.at(++index);

			if (val_token.type == token_type::number) // 数字
			{
				ret->value = val_token.get_negative_val(); // 提取负数
				ret->number_type = val_token.number_type;
			}

			index++;
			return ret;
		}
		else // 其他符号，统一视为无效
			throw syntax_parsing_exception(tgt.line, tgt.character, "Invalid expression.");

	case token_type::number: // 数字
	{
		auto ret = new node_value();

		ret->node_type = node_value::node_constant;
		ret->value = tgt.number_data;
		ret->number_type = tgt.number_type;

		index++;
		return ret;
	}

	case token_type::identifier: // 标识符
		if (index < list.size() - 1)
			if (list.at(index + 1).type == token_type::left_parentheses)
			{
				// 解析函数
				auto ret = new node_value();
				ret->node_type = node_value::node_function;
				ret->identifier = tgt.literal;

				index += 2;

				if (list.at(index).type == token_type::right_parentheses)
				{
					index++;
					return ret;
				}

				while (1)
				{
					ret->parameter_list.push_back(parse_numeric_expression(list, index));

					if (index >= list.size())
						throw syntax_parsing_exception(-1, -1, "Invalid function call format.");

					if (list.at(index).type == token_type::separator)
					{
						index++;
						continue;
					}
					else if (list.at(index).type == token_type::right_parentheses)
					{
						index++;
						return ret;
					}
					else
						throw syntax_parsing_exception(-1, -1, "Invalid function call format.");
				}
			}
		{
			// 普通标识符
			auto ret = new node_value();
			ret->node_type = node_value::node_identifier;
			ret->identifier = tgt.literal;

			index++;
			return ret;
		}

	default:
		throw syntax_parsing_exception(tgt.line, tgt.character, "Invalid token here.");
	}
}