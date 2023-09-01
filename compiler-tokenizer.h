// compiler-tokenizer: 词法分析器

#pragma once

#include "compiler-base.h"

namespace compiler
{
	struct tokenizer_exception : public compiler_exception 
	{
		using compiler_exception::compiler_exception; // 继承构造函数
	};

	// 词元分析器包装类
	class tokenizer
	{
	private: // 辅助函数
		static std::optional<std::tuple<number_union, size_t, number_type>> get_number(std::string_view input); // 提取数字常量
		static std::optional<std::string_view> get_identifier(std::string_view input); // 提取单词（标识符/关键词）
		static bool has_white_space(std::string_view view); // 视图首是否为空格

	public:
		static std::vector<token> tokenize(std::string_view str);
	};
}