#include "regex.h"
/**
 * 注：如果你愿意，你可以自由的using namespace。
 */

std::vector<Rule> Regex::single_build_rules(antlr4::RuleContext* rule_node, int curr, int next) //single类型的直接构造转移规则
{
    int rule_index;
    std::vector<Rule> new_rules;
    for (auto single_child : rule_node->children) // 其实应该只有一个child
    {
        auto single_child_Node = (antlr4::RuleContext *)single_child;
        if (single_child_Node->getTreeType() == antlr4::tree::ParseTreeType::TERMINAL)     //叶子节点，匹配any character
        {
            Rule new_rule(next, 2, ".");
            new_rules.push_back(new_rule);
            return new_rules;
        }
        rule_index = single_child_Node->getRuleIndex();
        switch (rule_index) // single分四种，上面实现了一种，还需要实现三种
        {
            case regexParser::RuleChar:
            {
                for (auto child : single_child->children)//其实应该只有一个child
                {
                    std::string cur_str = child->getText();
                    if (cur_str.length() >= 2)
                    {
                        new_rules.push_back(Rule(next, 0, cur_str.substr(1, 1))); //dst, type, by(防止转义)
                        return new_rules;
                    }
                    else
                    {
                        new_rules.push_back(Rule(next, 0, cur_str));    //dst, type, by
                        return new_rules;
                    }
                }
            }
            case regexParser::RuleCharacterClass:
            {
                for (auto child : rule_node->children)//其实应该只有一个child
                {
                    std::string cur_str = child->getText();
                    new_rules.push_back(Rule(next, 2, cur_str.substr(1, 1)));
                    return new_rules;
                }
            }
            case regexParser::RuleCharacterGroup:
            {
                return character_group_build_rules(single_child_Node);
            }
        }
   }
}

std::vector<Rule> Regex::character_group_build_rules(antlr4::RuleContext* rule_node)
{
    std::vector<Rule> new_rules;
    bool flag = 0;  //表示是否有取反符号
    std::set<char> _not_include;
    for (int i = 1; i < rule_node->children.size() - 1; i++)
    {
        auto group_item = (antlr4::RuleContext*)rule_node->children[i];
        
        if (group_item->getRuleIndex() == regexParser::RuleCharacterGroupNegativeModifier)
        {
            flag = 1;
            continue;
        }
        auto group_item_child = (antlr4::RuleContext*)group_item->children[0];
        switch (group_item_child->getRuleIndex())
        {
            case regexParser::RuleCharInGroup:
            {
                std::string cur_str = group_item_child->children[0]->getText();
                if (cur_str.length() >= 2)
                    cur_str = cur_str.substr(1, 1); //(防止转义)
                if(flag == 0)
                    new_rules.push_back(Rule(1, 0, cur_str)); //dst, type, by
                else
                {
                    _not_include.insert(cur_str[0]);
                }
                break;
            }
            case regexParser::RuleCharacterRange:
            {
                std::string cur_str_by = group_item_child->children[0]->getText();
                std::string cur_str_to = group_item_child->children[2]->getText();
                if (cur_str_by.length() >= 2)
                    cur_str_by = cur_str_by.substr(1, 1);
                if (cur_str_to.length() >= 2)
                    cur_str_to = cur_str_to.substr(1, 1);
                if (flag == 0)
                    new_rules.push_back(Rule(1, 1, cur_str_by, cur_str_to));
                else
                {
                    for (char i = cur_str_by[0]; i <= cur_str_to[0]; i++)
                    {
                        _not_include.insert(i);
                    }
                }
                break;
            }
            case regexParser::RuleCharacterClass:
            {
                std::string cur_str = group_item_child->children[0]->getText();
                if (flag == 0)
                    new_rules.push_back(Rule(1, 2, cur_str.substr(1, 1)));
                else    {
                    switch (cur_str[1]) {
                        case 'd':   {
                            for(char i = '0'; i <= '9'; i++)
                                _not_include.insert(i);
                            break;
                        }
                        case 'D':   {
                            for (char i = 32; i <= 126; i++)
                                if (!(i >= '0' && i <= '9'))
                                    _not_include.insert(i);
                            break;
                        }
                        case 'w':   {
                            for (char i = '0'; i <= '9'; i++)
                                _not_include.insert(i);
                            for (char i = 'a'; i <= 'z'; i++)
                                _not_include.insert(i);
                            for (char i = 'A'; i <= 'Z'; i++)
                                _not_include.insert(i);
                            break;
                        }
                        case 'W':  {
                            for (char i = 32; i <= 126; i++)
                            {
                                if(!(i >= '0' && i <= '9') && !(i >= 'a' && i <= 'z') && !(i >= 'A' && i <= 'Z'))
                                    _not_include.insert(i);
                            }
                            break;
                        }
                        case 's': {
                            _not_include.insert('\f');
                            _not_include.insert('\n');
                            _not_include.insert('\r');
                            _not_include.insert('\t');
                            _not_include.insert('\v');
                            _not_include.insert(' ');
                            break;
                        }
                        case 'S': {
                            for (char i = 32; i <= 126; i++) {
                                if (i == '\f' || i == '\n' || i == '\r'
                                    || i == '\t' || i == '\v' || i == ' ')
                                    continue;
                                _not_include.insert(i);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    if (flag == 1)
    {
        new_rules.push_back(Rule(1, 4, _not_include));
    }
    return new_rules;
}

void Rules_transition(std::vector<std::vector<Rule>>& rules, int distance) //组合NFA时将其中一个的rules的dst整体加一个常数
{
    for (int i = 0; i < rules.size(); i++)
    {
        for (int j = 0; j < rules[i].size(); j++)
        {
            rules[i][j].dst += distance;
        }
    }
}

NFA Regex::expression_to_nfa(antlr4::RuleContext* rule_node)
{
    std::vector<NFA> nfas;
    for (auto child : rule_node->children)//开始连接这些item
    {
        auto item_rule_node = (antlr4::RuleContext*)child;
        nfas.push_back(expression_item_to_nfa(item_rule_node));
        if(nfas.size()==2)
        {
            std::vector<std::vector<Rule>> curr_rules = nfas[0].rules;
            int temp = nfas[0].num_states;  //终态就是temp-1
            Rules_transition(nfas[1].rules, temp - 1); // 前者终态与后者初态合并
            for (int i = 0; i < nfas[1].rules.size(); i++)
            {
                nfas[0].rules.push_back(nfas[1].rules[i]);
            }
            nfas[0].num_states += (nfas[1].num_states - 1);
            nfas.pop_back();
        }
    }
    return nfas[0];
}

NFA Regex::expression_item_to_nfa(antlr4::RuleContext* rule_node)
{
    if (rule_node->children.size() == 1)
    {
        for (auto child : rule_node->children)  //其实只有1个
        {
            auto normal_rule_node = (antlr4::RuleContext*)child;
            return normal_item_to_nfa(normal_rule_node);
        }
    }
    else
    {
        auto normal_rule_node = (antlr4::RuleContext*)rule_node->children[0];
        NFA nfa_here = normal_item_to_nfa(normal_rule_node);
        auto quantifier_here = (antlr4::RuleContext*)rule_node->children[1];   //quantifier
        auto quantifier_type = (antlr4::RuleContext*)quantifier_here->children[0];   //quantifier_type
        std::string quantifier_str = quantifier_type->getText();
        NFA nfa_new;
        nfa_new.num_states = nfa_here.num_states + 2;   //新增一个初态、一个终态
        Rules_transition(nfa_here.rules, 1);
        nfa_new.rules.emplace_back();
        if (quantifier_here->children.size() == 1)   //无非贪婪？修饰
        {
            nfa_new.rules[0].push_back(Rule(1, 3, ""));    //新初态到原初态的epsilon转移(2)
            for (int i = 0; i < nfa_here.num_states - 1; i++)
            {
                nfa_new.rules.push_back(nfa_here.rules[i]);
            }
            nfa_new.rules.emplace_back();
            if (quantifier_str != "+")
            {
                nfa_new.rules[0].push_back(Rule(nfa_new.num_states - 1, 3, ""));  //新初态到新终态的epsilon转移（+不需要）(1）
            }
            if (quantifier_str != "?")
            {
                nfa_new.rules[nfa_new.num_states - 2].push_back(Rule(1, 3, ""));   //原终态到原初态的epsilon转移(?不需要)（4）
            }
            nfa_new.rules[nfa_new.num_states - 2].push_back(Rule(nfa_new.num_states - 1, 3, "")); //原终态到新终态的epsilon转移（3）
            //上面顺序不能反，关系到贪婪匹配模式非贪婪匹配模式
        }
        else
        {
            if (quantifier_str != "+")
            {
                nfa_new.rules[0].push_back(Rule(nfa_new.num_states - 1, 3, ""));  //新初态到新终态的epsilon转移（+不需要）(1）
            }
            nfa_new.rules[0].push_back(Rule(1, 3, ""));    //新初态到原初态的epsilon转移(2)
            for (int i = 0; i < nfa_here.num_states - 1; i++)
            {
                nfa_new.rules.push_back(nfa_here.rules[i]);
            }
            nfa_new.rules.emplace_back();
            nfa_new.rules[nfa_new.num_states - 2].push_back(Rule(nfa_new.num_states - 1, 3, "")); //原终态到新终态的epsilon转移（3）
            if (quantifier_str != "?")
            {
                nfa_new.rules[nfa_new.num_states - 2].push_back(Rule(1, 3, ""));   //原终态到原初态的epsilon转移(?不需要)（4）
            }
        }
        return nfa_new;
    }
}

NFA Regex::normal_item_to_nfa(antlr4::RuleContext* rule_node)
{
    for (auto child : rule_node->children)  //其实只有1个
    {
        auto normal_rule_node = (antlr4::RuleContext*)child;
        switch(normal_rule_node->getRuleIndex())
        {
            case regexParser::RuleSingle:
                return single_to_nfa(normal_rule_node);
            case regexParser::RuleGroup:
            {
                auto regex_rule_node = (antlr4::RuleContext*)normal_rule_node->children[1];
                return regex_to_nfa(regex_rule_node);
            }
        }
    }
}

NFA Regex::single_to_nfa(antlr4::RuleContext* rule_node)
{
    std::vector<std::vector<Rule>> curr_rules;
    curr_rules.push_back(single_build_rules(rule_node, 0, 1));
    NFA curr_nfa(2, curr_rules);
    return curr_nfa;
}

NFA Regex::regex_to_nfa(antlr4::RuleContext* rule_node)
{
    std::vector<NFA> nfas;
    for (auto child : rule_node->children)//开始并联这些expression      
    {
        auto expression_rule_node = (antlr4::RuleContext*)child;
        if (expression_rule_node->getText() == "|")
            continue;
        nfas.push_back(expression_to_nfa(expression_rule_node));
        if (nfas.size() == 2) // 并联需要的操作 （1）初态合并 （2） 后者状态整体加一常数（3）新增终态，两个终态epsilon转移通向新终态
        {
            std::vector<std::vector<Rule>> curr_rules = nfas[0].rules;
            int temp = nfas[0].num_states;  //前者终态就是temp-1

            Rules_transition(nfas[1].rules, temp - 1); // 后者状态整体加一常数
            for (int i = 0; i < nfas[1].rules[0].size(); i++)   //合并初态
            {
                nfas[0].rules[0].push_back(nfas[1].rules[0][i]);
            }
            for (int i = 0; i < nfas[0].num_states - nfas[0].rules.size(); i++)   //rules数组容量搞好
            {
                nfas[0].rules.emplace_back();
            }
            nfas[0].num_states += (nfas[1].num_states);  //新状态数（少一个初态多一个终态）

            for (int i = 1; i < nfas[1].num_states - 1; i++)    //后者加入前者
            {
                nfas[0].rules.push_back(nfas[1].rules[i]);
            }
            for (int i = 0; i < nfas[0].num_states - nfas[0].rules.size() - 1; i++)   //rules数组容量搞好
            {
                nfas[0].rules.emplace_back();
            }

            nfas[0].rules[temp - 1].push_back(Rule(nfas[0].num_states - 1, 3, ""));//两个终态epsilon转移通向新终态
            nfas[0].rules[nfas[0].num_states - 2].push_back(Rule(nfas[0].num_states - 1, 3, ""));

            nfas.pop_back(); //合并好了之后删去后者
        }
    }
    return nfas[0];
}

/**
 * 编译给定的正则表达式。
 * 具体包括两个过程：解析正则表达式得到语法分析树（这步已经为你写好，即parse方法），
 * 和在语法分析树上进行分析（遍历），构造出NFA（需要你完成的部分）。
 * 在语法分析树上进行分析的方法，可以是直接自行访问该树，也可以是使用antlr的Visitor机制，详见作业文档。
 * 你编译产生的结果，NFA应保存在当前对象的nfa成员变量中，其他内容也建议保存在当前对象下（你可以自由地在本类中声明新的成员）。
 * @param pattern 正则表达式的字符串
 * @param flags 正则表达式的修饰符（第二次实验不要求支持，保证传入的永远是空串）
 */
void Regex::compile(const std::string &pattern, const std::string &flags) {
    regexParser::RegexContext *tree = Regex::parse(pattern); // 这是语法分析树
    // TODO 请你完成这个函数
    // 你应该根据tree中的内容，恰当地构造NFA
    // 构造好的NFA，和其他数据变量（如有），均建议保存在当前对象中。
    // 例如 (this->)nfa.num_states = 100;
    
    nfa = regex_to_nfa(tree);
    nfa_const = nfa;
    return;
}

/**
 * 在给定的输入文本上，进行正则表达式匹配，返回匹配到的第一个结果。
 * 匹配不成功时，返回空vector( return std::vector<std::string>(); ，或使用返回初始化列表的语法 return {}; )；
 * 匹配成功时，返回一个std::vector<std::string>，其中下标为0的元素是匹配到的字符串，
 * 下标为i(i>=1)的元素是匹配结果中的第i个分组。
 * （第二次实验中不要求支持分组功能，返回的vector中只含一个元素即可）
 * @param text 输入的文本
 * @return 如上所述
 */
std::vector<std::string> Regex::match(std::string text) {
    // TODO 请你完成这个函数
    //std::cout << "matching" << std::endl;
    std::vector<std::string> result_here;
    while (1)
    {
        std::string result_string = nfa.exec_2(text);
        if (!result_string.empty())
        {
            result_here.push_back(result_string);
            //std::cout << "matched" << std::endl;
            //std::cout << result_here[0] << std::endl;
            return result_here;
        }
        if(!text.empty())
        text.erase(0, 1);
        nfa = nfa_const;
        if (text.empty())
        {
            return {};
        }
    }
}
/**
 * 解析正则表达式的字符串，生成语法分析树。
 * 你应该在compile函数中调用一次本函数，以得到语法分析树。
 * 通常，你不需要改动此函数，也不需要理解此函数实现每一行的具体含义。
 * 但是，你应当对语法分析树的数据结构(RegexContext)有一定的理解，作业文档中有相关的教程可供参考。
 * @param pattern 要解析的正则表达式的字符串
 * @return RegexContext类的对象的指针。保证不为空指针。
 */
regexParser::RegexContext *Regex::parse(const std::string &pattern) {
    if (antlrInputStream) throw std::runtime_error("此Regex对象已被调用过一次parse函数，不可以再次调用！");
    antlrInputStream = new antlr4::ANTLRInputStream(pattern);
    antlrLexer = new regexLexer(antlrInputStream);
    antlrTokenStream = new antlr4::CommonTokenStream(antlrLexer);
    antlrParser = new regexParser(antlrTokenStream);
    regexParser::RegexContext *tree = antlrParser->regex();
    if (!tree) throw std::runtime_error("parser解析失败(函数返回了nullptr)");
    auto errCount = antlrParser->getNumberOfSyntaxErrors();
    if (errCount > 0) throw std::runtime_error("parser解析失败，表达式中有" + std::to_string(errCount) + "个语法错误！");
    if (antlrTokenStream->LA(1) != antlr4::Token::EOF)
        throw std::runtime_error("parser解析失败，解析过程未能到达字符串结尾，可能是由于表达式中间有无法解析的内容！已解析的部分："
                                 + antlrTokenStream->getText(antlrTokenStream->get(0),
                                                             antlrTokenStream->get(antlrTokenStream->index() - 1)));
    return tree;
}

// 此析构函数是为了管理ANTLR语法分析树所使用的内存的。你不需要阅读和理解它。
Regex::~Regex() {
    delete antlrInputStream;
    delete antlrLexer;
    delete antlrTokenStream;
    delete antlrParser;
}
