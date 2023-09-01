// compiler-base.h: 编译器主要数据结构

#pragma once

#include "compiler-head.h" // 通用头文件

namespace compiler
{
    struct compiler_exception;
    struct token;
    struct variable_identifier;
    struct node_value;
    struct assignment;

    struct compiler_exception
    {
    public:
        size_t line, character;
        std::string msg;
        bool pos_available;

        compiler_exception(size_t line, size_t character, std::string msg = "") : line(line), character(character), msg(msg), pos_available(true) {}
        compiler_exception(std::string msg) : line(0), character(0), msg(msg), pos_available(false) {}
        compiler_exception(const token& token, std::string msg = "");
    };

    // 操作符类型
    enum class operand_type
    {
        none = 0,
        add, // ‘+’
        sub, // ‘-’
        mul,
        div,
        mod,
        bit_not,
        bit_and,
        bit_or,
        bit_xor,
        bit_lsh,
        bit_rsh,
        equal,
        not_equal,
        larger,
        larger_or_equal,
        smaller,
        smaller_or_equal,
        get_address, // 取地址（@）
        set_val,
        explanation // 解释符：':'
    };

    // 关键词类型
    enum class keyword_type
    {
        none = 0,
        kw_while,
        kw_if,
        kw_func,
        kw_return,
        kw_for,
        kw_break,
        kw_continue,
        kw_else
    };

    // 关键词查找表
    extern const std::unordered_map<std::string, keyword_type> keyword_lut;

    // 数据类型
    enum class number_type
    {
        none = 0,
        integer, // 有符号32位整形
        unsigned_integer, // 无符号32位整型
        float_point // 32位浮点
    };

    // 数据类型查找表
    extern const std::unordered_map<std::string, number_type> number_type_lut;

    /* 词元分析器 */

    // 数据union
    union number_union
    {
        float fp_number;
        int int_number;
        unsigned int uint_number = 0;
    };

    // 词元类型
    enum class token_type
    {
        unknown = 0,
        identifier, // 标识符
        double_operand, // 双目运算
        single_operand, // 单目运算
        keyword, // 关键词
        number_type, // 数据类型
        number, // 常量
        semicolon, // 语句结束符：;
        left_bracket, // 语块开始符：{
        right_bracket, // 语块结束符：}
        left_parentheses, // 左圆括号
        right_parentheses, // 右圆括号
        separator, // 逗号
    };

    // 词元
    struct token
    {
        bool white_space = false;
        token_type type = token_type::unknown; // 大类别

        // 细分类别
        union
        {
            operand_type op_type = operand_type::none;
            keyword_type key_type;
            number_type number_type;
        };

        std::string literal;

        number_union number_data;

        size_t line = 0, character = 0;

    public:
        token() = default;

        // 提取负数
        number_union get_negative_val() const;

        // 匹配
        inline bool match(const operand_type target) const
        {
            if (type != token_type::double_operand && type != token_type::single_operand) return false;
            return op_type == target;
        }

        inline bool match(const keyword_type target) const
        {
            if (type != token_type::keyword) return false;
            return target == key_type;
        }

        inline bool match(const token_type target) const
        {
            return target == type;
        }
    };

    // 词元列表
    typedef std::vector<token> token_list; 

    /* 语法树 */

    // 变量
    struct variable_identifier
    {
        bool global = false; // 全局或临时变量
        std::string name;
        number_type val_type = number_type::none; // 变量类型
        size_t mem_alloc = SIZE_MAX; // 寄存器/内存位置

    public:
        variable_identifier() = default;
        variable_identifier(std::string name, number_type val_type, bool global = false) : name(name), val_type(val_type), global(global) {}
        ~variable_identifier() = default;
    };

    // 变量空间
    struct variable_scope
    {
        variable_scope* parent = nullptr; // 上级变量空间
        std::unordered_map<std::string, variable_identifier> map;

    public:
        variable_scope() = default;
        variable_scope(variable_scope* parent) : parent(parent) {}

        ~variable_scope() = default;

        // 搜索变量
        std::optional<variable_identifier*> search(const std::string& identifier);

        // 插入变量
        inline void add_identifier(const variable_identifier& id)
        {
            map.insert(std::make_pair(id.name, id));
        }
    };

    // 数值节点
    struct node_value
    {
        enum type
        {
            node_invalid,
            node_operator,
            node_identifier,
            node_constant,
            node_function
        };

        type node_type = node_invalid;

        std::string identifier;
        number_union value;

        union
        {
            operand_type op_type;
            number_type number_type = number_type::none;
            variable_identifier* val_identifier;
        };

        node_value* left = nullptr;
        node_value* right = nullptr;

        std::vector<node_value*> parameter_list; // 函数参数列表

    public:
        node_value() = default;

        ~node_value()
        {
            if (left) delete left;
            if (right) delete right;
            for (auto para : parameter_list) delete para;
        }
    };

    struct declarement
    {
        number_type type;
        std::string name;
    };
    
    // 赋值语句
    struct assignment
    {
        enum variable_type
        {
            ram_address, // 内存地址
            variable // 变量
        };

        node_value* right = nullptr; // 右值
        variable_type type = ram_address; // 左值类型
        
        union
        {
            node_value* left = nullptr;
            std::string var_name;
        };

    public:
        assignment() = default;

        ~assignment()
        {
            if (right) delete right;
            if (type == ram_address) if (left) delete left;
            else var_name.~basic_string();
        }
    };

    struct statement_wrapper;

    //== 语句包裹结构体 ==

    // if 分支
    struct if_branch
    {
        node_value* condition = nullptr;
        variable_scope var_scope; // 主体部分变量空间
        std::optional<variable_scope> else_var_scope = std::nullopt; // else部分变量空间
        std::vector<statement_wrapper> body; // 语句主体
        std::vector<statement_wrapper> else_body;

    public:
        if_branch() = default;
    };

    // while 循环
    struct while_loop
    {
        node_value* condition;
        variable_scope var_scope;
        std::vector<statement_wrapper> body;
    };

    struct statement_wrapper
    {
        enum type // 语句类型
        {
            none,
            decl, // 声明语句
            assign, // 赋值语句
            if_statement, // if 语句
            while_loop
        } 
        statement_type = none;

        union
        {
            void* ptr = nullptr;
            declarement* decl_ptr;
            assignment* assignment_ptr;
            if_branch* if_ptr;
        };

        statement_wrapper() = default;
        statement_wrapper(type t, void* ptr): statement_type(t), ptr(ptr){}
    };
}