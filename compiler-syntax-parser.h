// compiler-syntax-parser.h: �﷨������

#pragma once

#include "compiler-head.h"
#include "compiler-base.h"

namespace compiler
{
	struct syntax_parsing_exception : public compiler_exception 
	{ 
		using compiler_exception::compiler_exception; // �̳й��캯��
	};

	// ��ֵ���ʽ����
	namespace numeric_parser
	{
		// ����������ʽ
		node_value* parse_numeric_expression(const token_list& list, size_t& index, size_t depth = 0);

		// ��Ԫ���������ģ��
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

		// һԪ���������ģ��
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

		// ����ȡ�ڴ�����������ȼ�0
		node_value* parse_priority0(const token_list& list, size_t& index, size_t depth);

		// ������λ������������ȼ�1
		node_value* parse_priority1(const token_list& list, size_t& index, size_t depth);

		// �����˳�����ȡ�࣬���ȼ�2
		node_value* parse_priority2(const token_list& list, size_t& index, size_t depth);

		// �����Ӽ��������ȼ�3
		node_value* parse_priority3(const token_list& list, size_t& index, size_t depth);

		// ����λ������������ȼ�4
		node_value* parse_priority4(const token_list& list, size_t& index, size_t depth);

		// ������С��ϵ����������ȼ�5
		node_value* parse_priority5(const token_list& list, size_t& index, size_t depth);

		// �������/����ȣ����ȼ�6
		node_value* parse_priority6(const token_list& list, size_t& index, size_t depth);

		// ����λ�룬���ȼ�7
		node_value* parse_priority7(const token_list& list, size_t& index, size_t depth);

		// ����λ������ȼ�8
		node_value* parse_priority8(const token_list& list, size_t& index, size_t depth);

		// ����λ�����ȼ�9
		node_value* parse_priority9(const token_list& list, size_t& index, size_t depth);

		// ��������/����/����
		node_value* parse_number(const token_list& list, size_t& index, size_t depth);
	}

	namespace statement_parser
	{
		// �滻������
		void node_value_identifier_replace(node_value* node, variable_scope& scope);
		
		// ��������
		void parse_statement_block(token_list& list, size_t& index, std::vector<statement_wrapper>& block, variable_scope* var_scope);

		// ������ֵ��ʽ
		std::optional<assignment*> parse_assignment(token_list& list, size_t& index);

		// �����������; 
		// - ��ʧ�ܣ����� std::nullopt; 
		// - ���ɹ��������޸�ֵ���ʱ���� nullptr, ���򷵻ؾ��帳ֵ�ڵ�
		std::optional<std::tuple<declarement*, assignment*>> parse_declarement(token_list& list, size_t& index, bool global = false);
		
		// ����if��֧���
		std::optional<if_branch*> parse_if_branch(token_list& list, size_t& index);
	
	}
}