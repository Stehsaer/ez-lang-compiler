#include "compiler-base.h"

using namespace compiler;

compiler_exception::compiler_exception(const token& token, std::string msg) : line(token.line), character(token.character), msg(msg), pos_available(true) {}

std::optional<variable_identifier*> compiler::variable_scope::search(const std::string& identifier)
{
	auto search = this;

	while (1)
	{
		auto find = search->map.find(identifier);
		if (find == search->map.end())
		{
			if (search->parent)
			{
				search = search->parent; // 返回上一级，继续查找
				continue;
			}
			else
				return std::nullopt; // 无结果
		}
		else
			return &find->second;
	}
}

number_union compiler::token::get_negative_val() const
{
    if (type == token_type::number)
    {
        number_union temp;

        switch (number_type)
        {
        case number_type::float_point:
            temp.fp_number = -number_data.fp_number;
            return temp;

        case number_type::integer:
            temp.int_number = -number_data.int_number;
            return temp;

        case number_type::unsigned_integer:
            throw compiler_exception(line, character,
                "Can't apply negative operand to an unsigned integer.");

        default:
            throw compiler_exception(line, character,
                "No negative operand available for this token. This is an internal error/bug.");
        }
    }
    else
        throw compiler_exception(line, character,
            "Can't apply negative operand to this token.");
}
