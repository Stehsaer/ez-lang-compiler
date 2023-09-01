// compiler-syntax-parser.h: 语法树生成

#pragma once

#include "compiler-head.h"
#include "compiler-base.h"

namespace compiler
{
	struct syntax_parsing_exception : public compiler_exception 
	{ 
		using compiler_exception::compiler_exception; // 继承构造函数
	};

	// 数值表达式解析
	namespace numeric_parser
	{
		// 解析整个算式
		node_value* parse_numeric_expression(const token_list& list, size_t& index, size_t depth = 0);

		// 二元运算符解析模板
		template<operand_type... type>
		node_value* parse_double_operand(const token_list& list, size_t& index, std::function<node_value*(const token_list&, size_t&, size_t)> func, size_t depth)
		{
			auto node = func(list, index, depth);

			while (1)
			{
				if (index >= list.size())
					return node;

				auto& operand = list.at(index);

				if (operand.type == token_type::right_parentheses || operand.type == token_type::semicolon)
					return node;

				if (operand.type != token_type::double_operand)
					return node;

				if (((operand.op_type == type)||...))
				{
					auto right = func(list, ++index, depth);

					auto new_node = new node_value();
					new_node->left = node;
					new_node->right = right;
					new_node->node_type = node_value::node_operator;
					new_node->op_type = operand.op_type;
					node = new_node;

					continue;
				}

				return node;
			}
		}

		// 一元运算符解析模板
		template<operand_type... type>
		node_value* parse_single_operand(const token_list& list, size_t& index, std::function<node_value* (const token_list&, size_t&, size_t)> func, size_t depth)
		{
			auto& operand = list.at(index);

			if (operand.type == token_type::single_operand)
				if (((operand.op_type == type) || ...))
				{
					auto val_node = func(list, ++index, depth);
					auto node = new node_value();
					node->node_type = node_value::node_operator;
					node->op_type = operand.op_type;
					node->left = val_node;

					return node;
				}

			return func(list, index, depth);
		}

		// 解析取内存运算符，优先级0
		node_value* parse_priority0(const token_list& list, size_t& index, size_t depth);

		// 解析按位非运算符，优先级1
		node_value* parse_priority1(const token_list& list, size_t& index, size_t depth);

		// 解析乘除法和取余，优先级2
		node_value* parse_priority2(const token_list& list, size_t& index, size_t depth);

		// 解析加减法，优先级3
		node_value* parse_priority3(const token_list& list, size_t& index, size_t depth);

		// 解析位移运算符，优先级4
		node_value* parse_priority4(const token_list& list, size_t& index, size_t depth);

		// 解析大小关系运算符，优先级5
		node_value* parse_priority5(const token_list& list, size_t& index, size_t depth);

		// 解析相等/不相等，优先级6
		node_value* parse_priority6(const token_list& list, size_t& index, size_t depth);

		// 解析位与，优先级7
		node_value* parse_priority7(const token_list& list, size_t& index, size_t depth);

		// 解析位异或，优先级8
		node_value* parse_priority8(const token_list& list, size_t& index, size_t depth);

		// 解析位或，优先级9
		node_value* parse_priority9(const token_list& list, size_t& index, size_t depth);

		// 解析数字/括号/函数
		node_value* parse_number(const token_list& list, size_t& index, size_t depth);
	}

	namespace statement_parser
	{
		// 替换变量符
		void node_value_identifier_replace(node_value* node, variable_scope& scope);
		
		// 解析语句块
		void parse_statement_block(token_list& list, size_t& index, std::vector<statement_wrapper>& block, variable_scope* var_scope);

		// 解析赋值算式
		std::optional<assignment*> parse_assignment(token_list& list, size_t& index);

		// 解析声明语句; 
		// - 若失败，返回 std::nullopt; 
		// - 若成功，后置无赋值语句时返回 nullptr, 否则返回具体赋值节点
		std::optional<std::tuple<declarement*, assignment*>> parse_declarement(token_list& list, size_t& index, bool global = false);
		
		// 解析if分支语句
		std::optional<if_branch*> parse_if_branch(token_list& list, size_t& index);
	
	}
}