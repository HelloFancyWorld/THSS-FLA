#ifndef CPP_REGEX_H
#define CPP_REGEX_H

#include "nfa.h"
#include "parser/regexLexer.h"
#include "parser/regexParser.h"

/**
 * 本文件（包括对应的cpp文件）中已经定义好了一些类和函数，类内也已经定义好了一些成员变量和方法。不建议大家修改这些已经定义好的东西。
 * 但是，为了实现功能，你可以自由地增加新的函数、类等，包括可以在已经定义好的类自由地添加新的成员变量和方法。
 */
/**
 * 在第三次实验中：
 *   - 保证正则表达式字符串和待操作的文本内容都仅包含ASCII字符，且不包含'\0'。
 *   - 仅保证pattern不含和换行符'\r' '\n'，但文本串可以含有换行符！
 *   - 要求支持的正则表达式文法为，随第三次实验下发的`regex.g4`中，所有未被注释的部分。
 *     - 更具体的本次实验相对上次实验新增特性的列表，请阅读实验文档。
 *   - 需要实现matchAll函数（返回串中所有匹配结果），replaceAll函数（将串中所有匹配结果进行替换）。
 *   - 同时你将会需要修改match函数和compile函数，以实现诸如返回捕获分组之类的功能。
 *   - 要求支持的特殊字符在`regex.g4`中均有列出。具体包括（如下一行列举的内容与`regex.g4`中的声明有冲突，请以`regex.g4`为准）：
 *     - \d \w \s \D \W \S . ^ $ \b \B
 *   - 特别注意，本次起需要支持m、s两种修饰符。
 * 正则表达式中各种字符的具体定义可查看 https://www.runoob.com/regexp/regexp-metachar.html
 */

/**
 * 表示一个正则表达式的类。
 */
class Regex {
public:
    NFA nfa; // 正则表达式所使用的NFA
    NFA nfa_const;
    int nfa_states = 1; // 全局变量，不重复，从1开始是因为省略初态

    /**
     * 解析正则表达式的字符串，生成语法分析树。
     * 你应该在compile函数中调用一次本函数，以得到语法分析树。
     * 通常，你不需要改动此函数，也不需要理解此函数实现每一行的具体含义。
     * 但是，你应当对语法分析树的数据结构(RegexContext)有一定的理解，作业文档中有相关的教程可供参考。
     * @param pattern 要解析的正则表达式的字符串
     * @return RegexContext类的对象的指针。保证不为空指针。
     */
    regexParser::RegexContext *parse(const std::string &pattern);

    /**
     * 编译给定的正则表达式。
     * 具体包括两个过程：解析正则表达式得到语法分析树（这步已经为你写好，即parse方法），
     * 和在语法分析树上进行分析（遍历），构造出NFA（需要你完成的部分）。
     * 在语法分析树上进行分析的方法，可以是直接自行访问该树，也可以是使用antlr的Visitor机制，详见作业文档。
     * 你编译产生的结果，NFA应保存在当前对象的nfa成员变量中，其他内容也建议保存在当前对象下（你可以自由地在本类中声明新的成员）。
     * @param pattern 正则表达式的字符串
     * @param flags 正则表达式的修饰符
     */
    void compile(const std::string &pattern, const std::string &flags = "");

    //自己添加的
    std::vector<Rule> single_build_rules(antlr4::RuleContext* rule_node, int curr, int next);
    std::vector<Rule> character_group_build_rules(antlr4::RuleContext* rule_node);
    NFA expression_to_nfa(antlr4::RuleContext* rule_node);
    NFA expression_item_to_nfa(antlr4::RuleContext* rule_node);
    NFA normal_item_to_nfa(antlr4::RuleContext* rule_node);
    NFA single_to_nfa(antlr4::RuleContext* rule_node);
    NFA regex_to_nfa(antlr4::RuleContext* rule_node);

    /**
     * 在给定的输入文本上，进行正则表达式匹配，返回匹配到的第一个结果。
     * 匹配不成功时，返回空vector( return std::vector<std::string>(); ，或使用返回初始化列表的语法 return {}; )；
     * 匹配成功时，返回一个std::vector<std::string>，其中下标为0的元素是匹配到的字符串，
     * 下标为i(i>=1)的元素是匹配结果中的第i个分组。
     * @param text 输入的文本
     * @return 如上所述
     */
    std::vector<std::string> match(std::string text);

    /**
     * 在给定的输入文本上，进行正则表达式匹配，返回匹配到的**所有**结果。
     * 匹配不成功时，返回空vector( return std::vector<std::string>(); ，或使用返回初始化列表的语法 return {}; )；
     * 匹配成功时，返回一个std::vector<std::vector<std::string>>，其中每个元素是每一个带分组的匹配结果，其格式同match函数的返回值（详见上面）。
     * @param text 输入的文本
     * @return 如上所述
     */
    std::vector<std::vector<std::string>> matchAll(std::string text);

    /**
     * 在给定的输入文本上，进行基于正则表达式的替换，返回替换完成的结果。
     * 需要支持分组替换，如$1表示将此处填入第一个分组匹配到的内容。具体的规则和例子详见文档。
     * @param text 输入的文本
     * @param replacement 要将每一处正则表达式的匹配结果替换为什么内容
     * @return 替换后的文本
     */
    std::string replaceAll(std::string text, std::string replacement);

    // 目前仅支持一个默认无参构造函数，且不允许拷贝构造（因为类内有指针）。
    Regex() = default;

    Regex(const Regex &) = delete;

    // 析构函数，和以下那些private变量，是为了管理ANTLR语法分析树所使用的内存的。你不需要阅读和理解它们。
    ~Regex();

private:
    antlr4::ANTLRInputStream *antlrInputStream = nullptr;
    regexLexer *antlrLexer = nullptr;
    antlr4::CommonTokenStream *antlrTokenStream = nullptr;
    regexParser *antlrParser = nullptr;
};

#endif //CPP_REGEX_H
