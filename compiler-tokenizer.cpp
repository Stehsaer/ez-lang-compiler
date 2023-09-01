// compiler-tokenizer.cpp: 实现词法分析器

#include "compiler-tokenizer.h"
#include "compiler-tokenizer-regex.hpp"

using namespace compiler;

std::optional<std::tuple<number_union, size_t, number_type>> compiler::tokenizer::get_number(std::string_view input)
{
    // 确定长度；若std::string_view可于std::regex_match中使用，则去除
    size_t len = 0;
    while ((input[len] >= '0' && input[len] <= '9')
        || (input[len] == 'x' && len == 1) 
        || input[len] == 'u' 
        || input[len] == '.' 
        || input[len] == 'f' 
        || (input[len] == '-' && len == 0))
    {
        len++;
        if (len >= input.length()) break;
    }

    std::string number_string{input.substr(0, len)}; // 拷贝字符串，导致性能损失

    number_union ret;

    if (std::regex_match(number_string, hex_regex)) // 十六进制
    {
        std::string match = std::regex_replace(number_string, hex_regex, "$1");
        ret.uint_number = strtoul(match.c_str(), nullptr, 16);
        return std::tuple(ret, len, number_type::unsigned_integer);
    }

    if (std::regex_match(number_string, unsigned_dec_regex)) // 无符号整数，32位
    {
        std::string match = std::regex_replace(number_string, unsigned_dec_regex, "$1");
        ret.uint_number = strtoul(match.c_str(), nullptr, 10);
        return std::tuple(ret, len, number_type::unsigned_integer);
    }

    if (std::regex_match(number_string, float_regex)) // 单精度浮点数
    {
        std::string match = std::regex_replace(number_string, float_regex, "$1");
        ret.fp_number = strtof(match.c_str(), nullptr);
        return std::tuple(ret, len, number_type::float_point);
    }

    if (std::regex_match(number_string, signed_dec_regex)) // 有符号整数，32位
    {
        std::string match = std::regex_replace(number_string, signed_dec_regex, "$1");
        ret.int_number = atoi(match.c_str());
        return std::tuple(ret, len, number_type::integer);
    }

    return std::nullopt; // 无对应
}

std::optional<std::string_view> compiler::tokenizer::get_identifier(std::string_view input)
{
    // 空字符串检测
    if (input.length() == 0) return std::nullopt;

    // 首字符检测
    if (!((input[0] >= 'A' && input[0] <= 'Z')
        || (input[0] >= 'a' && input[0] <= 'z')
        || input[0] == '_'))
    {
        return std::nullopt;
    }

    // 确定长度
    size_t iter = 1;
    while (1)
    {
        if (iter >= input.length())
            break;

        if (!((input[iter] >= 'A' && input[iter] <= 'Z')
            || (input[iter] >= 'a' && input[iter] <= 'z')
            || input[iter] == '_'
            || (input[iter] >= '0' && input[iter] <= '9')))
        {
            break; // 不符合条件，跳出
        }

        iter++;

        if (iter >= input.length()) break;
    }

    return input.substr(0, iter); // 截取字符串
}

bool compiler::tokenizer::has_white_space(std::string_view view)
{
    if (view.length() > 0)
        if (view[0] == ' ' || view[0] == '\r' || view[0] == '\n' || view[0] == '\t')
            return true;
    return false;
}

const std::unordered_map<std::string, keyword_type> compiler::keyword_lut = 
{
    {"while", keyword_type::kw_while},
    {"if", keyword_type::kw_if},
    {"func", keyword_type::kw_func},
    {"return", keyword_type::kw_return},
    {"for", keyword_type::kw_for},
    {"break", keyword_type::kw_break},
    {"continue", keyword_type::kw_continue},
    {"else", keyword_type::kw_else}
};

const std::unordered_map<std::string, number_type> compiler::number_type_lut =
{
    {"int", number_type::integer},
    {"uint", number_type::unsigned_integer},
    {"float", number_type::float_point}
};

std::vector<token> compiler::tokenizer::tokenize(std::string_view str)
{
    std::vector<token> list;
    std::string_view view{str};
    size_t line = 0, character = 0;

    while (view.length() > 0)
    {
        // 空字符处理
        switch (view[0])
        {
        case ' ':
        case '\r':
        case '\t':
            view = view.substr(1);
            character++;
            continue;
        case '\n':
            view = view.substr(1);
            line++; character = 0;
            continue;
        }

        token t;
        t.line = line; t.character = character;

        // 标识符解析
        auto identifier_str = get_identifier(view);
        if (identifier_str)
        {
            auto result = identifier_str.value();
            view = view.substr(result.length());
            character += result.length();
            t.literal = result;

            // 解析关键词
            auto kw_search = keyword_lut.find(std::string(result));
            auto nt_search = number_type_lut.find(std::string(result));

            if (kw_search != keyword_lut.end())
            {
                t.type = token_type::keyword;
                t.key_type = kw_search->second;
            }
            else if (nt_search != number_type_lut.end())
            {
                t.type = token_type::number_type;
                t.number_type = nt_search->second;
            }
            else // 普通标识符
            {
                t.type = token_type::identifier;
            }

            t.white_space = has_white_space(view); // 空格

            list.push_back(t);
            continue;
        }

        // 解析数字
        auto number_parse_obj = get_number(view);
        if (number_parse_obj)
        {
            t.type = token_type::number;
            t.number_data = std::get<0>(number_parse_obj.value()); // 常量本体
            t.number_type = std::get<2>(number_parse_obj.value()); // 数据类型

            size_t len = std::get<1>(number_parse_obj.value());
            view = view.substr(len); // 长度
            t.white_space = has_white_space(view);
            character += len;

            list.push_back(t);
            continue;
        }

        size_t view_seek = 1;

        // 处理算术符和语块符
        switch (view[0])
        {
        case ';':
            t.type = token_type::semicolon;
            break;
        case ',':
            t.type = token_type::separator;
            break;
        case '{':
            t.type = token_type::left_bracket;
            break;
        case '}':
            t.type = token_type::right_bracket;
            break;
        case '(':
            t.type = token_type::left_parentheses;
            break;
        case ')':
            t.type = token_type::right_parentheses;
            break;
        case '+':
            t.type = token_type::double_operand;
            t.op_type = operand_type::add;
            break;
        case '-':
            t.type = token_type::double_operand;
            t.op_type = operand_type::sub;
            break;
        case '*':
            t.type = token_type::double_operand;
            t.op_type = operand_type::mul;
            break;
        case '/':
            t.type = token_type::double_operand;
            t.op_type = operand_type::div;
            break;
        case '%':
            t.type = token_type::double_operand;
            t.op_type = operand_type::mod;
            break;
        case '!':
            if (view[1] == '=')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::not_equal;

                view_seek = 2;
            }
            else
            {
                t.type = token_type::single_operand;
                t.op_type = operand_type::bit_not;
            }
            break;
        case '|':
            t.type = token_type::double_operand;
            t.op_type = operand_type::bit_or;
            break;
        case '&':
            t.type = token_type::double_operand;
            t.op_type = operand_type::bit_and;
            break;
        case '^':
            t.type = token_type::double_operand;
            t.op_type = operand_type::bit_xor;
            break;
        case '@':
            t.type = token_type::single_operand;
            t.op_type = operand_type::get_address;
            break;
        case ':':
            t.type = token_type::single_operand;
            t.op_type = operand_type::explanation;
            break;
        case '=':
            if (view[1] == '=')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::equal;

                view_seek = 2;
            }
            else
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::set_val;
            }
            break;
        case '<':
            if (view[1] == '=')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::smaller_or_equal;
                view_seek = 2;
            }
            else if (view[1] == '<')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::bit_lsh;
                view_seek = 2;
            }
            else
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::smaller;
            }
            break;
        case '>':
            if (view[1] == '=')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::larger_or_equal;
                view_seek = 2;
            }
            else if (view[1] == '>')
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::bit_rsh;
                view_seek = 2;
            }
            else
            {
                t.type = token_type::double_operand;
                t.op_type = operand_type::larger;
            }
            break;
        }

        // 运算符处理
        if (t.type != token_type::unknown)
        {
            view = view.substr(view_seek);
            t.white_space = has_white_space(view);
            character += view_seek;
            list.push_back(t);
            continue;
        }
        else // 无匹配运算符
        {
            throw tokenizer_exception(line, character, "Unable to parse this token.");
        }
    }

    return list;
}
