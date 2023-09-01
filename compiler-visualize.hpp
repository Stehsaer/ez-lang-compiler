// compiler-visualize.hpp: 可视化编译器内部结果

#include "compiler.h"

namespace compiler::visualization
{
	const char* operator_literals[] = { "NONE", "+", "-", "*", "/", "%", "!", "&", "|", "^", "<<", ">>", "==", "!=", ">", ">=", "<", "<=", "@", "=", ":" };

	void print_node_value(node_value* node, size_t depth = 0)
	{
		for (size_t i = 0; i < depth; i++)
			printf("  ");

		switch (node->node_type)
		{
		case node_value::node_constant:
		{
			switch (node->number_type)
			{
			case number_type::float_point:
				printf("float(%f);\n", node->value.fp_number);
				break;

			case number_type::integer:
				printf("int(%d);\n", node->value.int_number);
				break;

			case number_type::unsigned_integer:
				printf("uint(%u);\n", node->value.uint_number);
				break;
			}
			break;
		}

		case node_value::node_identifier:
			printf("identifier(\"%s\")\n", node->identifier.c_str());
			break;

		case node_value::node_operator:
			printf("operator(%s):\n", operator_literals[int(node->op_type)]);
			if(node->left) print_node_value(node->left, depth + 1);
			if(node->right) print_node_value(node->right, depth + 1);
			break;

		case node_value::node_function:
			printf("func(\"%s\"):", node->identifier.c_str());
			if (node->parameter_list.size())
			{
				printf("\n");
				for (auto x : node->parameter_list)
					print_node_value(x, depth + 1);
			}
			else
				printf(" ();\n");
			break;
		}
	}

}