// compiler-tokenizer-regex.hpp: 词法分析器所用正则表达式

#pragma once

#include "compiler-head.h"

namespace compiler
{
	const std::regex hex_regex{ "^0x([0-9A-Fa-f]+)$" }; // 十六进制
	const std::regex unsigned_dec_regex{"^([0-9]+)[Uu]$"}; // 无符号十进制
	const std::regex signed_dec_regex{"^([0-9]+)$"}; // 有符号十进制，不带符号
	const std::regex float_regex{"^([0-9]*\\.[0-9]+)f?$"}; // 浮点，不带符号
}