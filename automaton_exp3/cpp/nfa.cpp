#include "nfa.h"
#include <sstream>
#include <algorithm>


/**
 * 在自动机上执行指定的输入字符串。
 * @param text 输入字符串
 * @return 若拒绝，请 return Path::reject(); 。若接受，请手工构造一个Path的实例并返回。
 */

bool multiline = false;
bool single_line = false;

bool Rule::match(char c)
{
    switch (type)
    {
    case NORMAL:
        return (by[0] == c);
        break;
    case RANGE_HERE:
        for (char i = by[0]; i <= to[0]; i++)
        {
            if (c == i)
                return true;
        }
        return false;
        break;
    case SPECIAL:
        switch (by[0])
        {
        case 'd':
            for (char i = '0'; i <= '9'; i++)
            {
                if (c == i)
                    return true;
            }
            return false;
            break;
        case 's':
            return (c == '\f' || c == '\n' || c == '\r'
                || c == '\t' || c == '\v' || c == ' ');
            break;
        case 'w':
            for (char i = '0'; i <= '9'; i++)
            {
                if (c == i)
                    return true;
            }
            for (char i = 'a'; i <= 'z'; i++)
            {
                if (c == i)
                    return true;
            }
            for (char i = 'A'; i <= 'Z'; i++)
            {
                if (c == i)
                    return true;
            }
            if (c == '_')
                return true;
            return false;
            break;
        case 'D':
            for (char i = '0'; i <= '9'; i++)
            {
                if (c == i)
                    return false;
            }
            return true;
            break;
        case 'S':
            if (c == '\f' || c == '\n' || c == '\r'
                || c == '\t' || c == '\v' || c == ' ')
            {
                return false;
            }
            else
                return true;
            break;
        case 'W':
            for (char i = '0'; i <= '9'; i++)
            {
                if (c == i)
                    return false;
            }
            for (char i = 'a'; i <= 'z'; i++)
            {
                if (c == i)
                    return false;
            }
            for (char i = 'A'; i <= 'Z'; i++)
            {
                if (c == i)
                    return false;
            }
            if (c == '_')
                return false;
            return true;
            break;
        case '.':
            if (single_line)    //s flag
                return true;
            else
                return (c != '\r' && c != '\n');
            break;
        }
    case NOT_INCLUDE:
        for (int i = 0; i < not_include.size(); i++)
        {
            if (not_include.count(c))
                return false;
        }
        return true;
    default:
        return false;
        break;
    }
}

Path backtrace(std::vector<Description> path_here)
{
    Path a;
    for (int i = 0; i < path_here.size(); i++)
    {
        a.states.push_back(path_here[i].curstate);
        if (i < path_here.size() - 1)
        {
            if (path_here[i].remain != path_here[i + 1].remain)
                a.consumes.push_back({ path_here[i].remain[0] });
            else
                a.consumes.push_back("");
        }
    }
    return a;
}

bool belong_to_backslash_w(char c)
{
    for (char i = '0'; i <= '9'; i++)
    {
        if (c == i)
            return true;
    }
    for (char i = 'a'; i <= 'z'; i++)
    {
        if (c == i)
            return true;
    }
    for (char i = 'A'; i <= 'Z'; i++)
    {
        if (c == i)
            return true;
    }
    if (c == '_')
        return true;
    return false;
}

void NFA::DFS(int q, std::string const_text, std::string text, int step, int length)
{
    path_record.push_back(Description(q, text));
    std::string str = text;
    //if (is_final[q] && (str.size() == 0)) //第1次实验
    if (num_states - 1 == q) //第2次实验
    {
        //std::cout << "match found" << std::endl;
        result = backtrace(path_record);
        flag = 1;
        return;
    }
    std::vector<Description> temp = path_record;
    for (int i = 0; i < rules[q].size(); i++)
    {
        if (rules[q][i].type == EPSILON_HERE)
        {
            if (rules[q][i].by == "^")
            {
                if (multiline)
                {
                    if (length > 0)
                    {
                        if((const_text[length - 1] != '\r') && (const_text[length - 1] != '\n'))
                            continue;
                    }
                }
                else if (!(length == 0))
                        continue;
            }
            else if (rules[q][i].by == "$")
            {
                if (multiline)
                {
                    if (const_text.size() != length)
                    {
                        if ((const_text[length] != '\r') && (const_text[length] != '\n'))
                            continue;
                    }
                }
                else if (!(const_text.size() == length))
                    continue;
            }
            else if (rules[q][i].by == "\\b")
            {
                bool curr_flag = 0;
                if ((length == 0) || (const_text.size() == length))
                {
                    curr_flag = 1;
                }
                else
                {
                    bool judge1 = belong_to_backslash_w(const_text[length]);
                    bool judge2 = belong_to_backslash_w(const_text[length - 1]);
                    if (judge1 == judge2)
                    {
                        curr_flag = 1;
                    }
                }
                if (curr_flag)
                    continue;
            }
            else if (rules[q][i].by == "\\B")
            {
                bool curr_flag = 0;
                bool judge1 = belong_to_backslash_w(const_text[length]);
                bool judge2 = belong_to_backslash_w(const_text[length - 1]);
                if (judge1 != judge2)
                {
                    curr_flag = 1;
                }
                if (curr_flag)
                    continue;
            }
            int len = str.size();
            int tempnum = -1;
            for (int j = 0; j < record_epsilon.size(); j++)
            {
                if (record_epsilon[j].remain_length == len)
                {
                    tempnum = j;
                    break;
                }
            }
            if (tempnum == -1)
            {
                std::vector<int> tempvector;
                tempvector.push_back(rules[q][i].dst);
                Record temprecord = Record(len, tempvector);
                record_epsilon.push_back(temprecord);
                DFS(rules[q][i].dst, const_text, str, step + 1, length);
                if (flag == 1)
                    return;
                path_record = temp;
                str = text;
            }
            else if (std::find(record_epsilon[tempnum].epsilon_circle.begin(), record_epsilon[tempnum].epsilon_circle.end(),
                rules[q][i].dst) == record_epsilon[tempnum].epsilon_circle.end())
            {
                record_epsilon[tempnum].epsilon_circle.push_back(rules[q][i].dst);
                DFS(rules[q][i].dst, const_text, str, step + 1, length);
                if (flag == 1)
                    return;
                path_record = temp;
                str = text;
            }
            else
                continue;
        }

        else if (str.size() > 0)
        {
            if (rules[q][i].match(str[0]))
            {
                str.erase(str.begin());
                DFS(rules[q][i].dst, const_text, str, step + 1, length + 1);
                if (flag == 1)
                    return;
                path_record = temp;
                str = text;
            }
        }
    }
}

Path NFA::exec(std::string text) {
    // TODO 请你完成这个函数
    DFS(0, text, text, 0, 0);
    if (result.states.size() > text.size())
        return result;
    else
        return Path::reject();
}

std::vector<std::string> NFA::exec_2(std::string const_text, std::string text, int length) {
    // TODO 请你完成这个函数
    DFS(0, const_text, text, 0, length);
    std::vector<std::string> result_string;
    int result_string_0_size = text.length() - path_record[path_record.size() - 1].remain.size();
    result_string.push_back(text.substr(0, result_string_0_size));  //去掉remain的就是用掉的
    //std::cout << "result_string " << result_string << std::endl;
    for (int i = 0; i < groups; i++)
    {
        int group_start, group_end;
        bool group_empty = true;
        for (int j = path_record.size() - 1; j >= 0; j--)
        {
            if (path_record[j].curstate == marks_start[i])
            {
                group_start = j;
                for (int k = j + 1; k < path_record.size(); k++)
                {
                    if (path_record[k].curstate == marks_end[i])
                    {
                        group_end = k;
                        group_empty = false;
                        break;
                    }
                }
                if (group_empty == true)
                    continue;
            }
            if (group_empty == false)
                break;
        }
        if (group_empty == true)
        {
            result_string.push_back("");
        }
        else
        {
            int result_string_i_size = path_record[group_start].remain.size() - path_record[group_end].remain.size();
            result_string.push_back(path_record[group_start].remain.substr(0, result_string_i_size));
        }
    }
    return result_string;
}
/**
 * 将Path转为（序列化为）文本的表达格式（以便于通过stdout输出）
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */


std::ostream& operator<<(std::ostream& os, Path& path_here) {
    if (!path_here.states.empty()) {
        if (path_here.consumes.size() != path_here.states.size() - 1)
            throw std::runtime_error("Path的len(consumes)不等于len(states)-1！");
        for (int i = 0; i < path_here.consumes.size(); ++i) {
            os << path_here.states[i] << " " << path_here.consumes[i] << " ";
        }
        os << path_here.states[path_here.states.size() - 1];
    }
    else os << std::string("Reject");
    return os;
}

/**
 * 从自动机的文本表示构造自动机
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */
NFA NFA::from_text(const std::string& text) {
    NFA nfa = NFA();
    bool reading_rules = false;
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.find("states:") == 0) {
            nfa.num_states = std::stoi(line.substr(7));
            for (int i = 0; i < nfa.num_states; ++i) {
                nfa.rules.emplace_back();
                nfa.is_final.push_back(false);
            }
            continue;
        }
        else if (line.find("final:") == 0) {
            if (nfa.num_states == 0) throw std::runtime_error("states必须出现在final和rules之前!");
            std::istringstream ss2(line.substr(6));
            int t;
            while (true) {
                ss2 >> t;
                if (!ss2.fail()) nfa.is_final[t] = true;
                else break;
            }
            reading_rules = false;
            if (ss2.eof()) continue;
        }
        else if (line.find("rules:") == 0) {
            if (nfa.num_states == 0) throw std::runtime_error("states必须出现在final和rules之前!");
            reading_rules = true;
            continue;
        }
        else if (line.find("input:") == 0) {
            reading_rules = false;
            continue;
        }
        else if (reading_rules) {
            auto arrow_pos = line.find("->"), space_pos = line.find(' ');
            if (arrow_pos != std::string::npos && space_pos != std::string::npos && arrow_pos < space_pos) {
                int src = std::stoi(line.substr(0, arrow_pos));
                int dst = std::stoi(line.substr(arrow_pos + 2, space_pos - (arrow_pos + 2)));
                auto content = line.substr(space_pos + 1);
                bool success = true;
                while (success && !content.empty()) {
                    auto p = content.find(' ');
                    if (p == std::string::npos) p = content.size();
                    else if (p == 0) p = 1; // 当第一个字母是空格时，说明转移的字符就是空格。于是假定第二个字母也是空格（如果不是，会在后面直接报错）
                    Rule rule{ dst };
                    if (p == 3 && content[1] == '-') {
                        rule.type = RANGE_HERE;
                        rule.by = content[0];
                        rule.to = content[2];
                    }
                    else if (p == 2 && content[0] == '\\') {
                        if (content[1] == 'e') rule.type = EPSILON_HERE;
                        else {
                            rule.type = SPECIAL;
                            rule.by = content[1];
                        }
                    }
                    else if (p == 1 && (p >= content.length() || content[p] == ' ')) {
                        rule.type = NORMAL;
                        rule.by = content[0];
                    }
                    else success = false;
                    nfa.rules[src].push_back(rule);
                    content = content.substr(std::min(p + 1, content.size()));
                }
                if (success) continue;
            }
        }
        throw std::runtime_error("无法parse输入文件！失败的行： " + line);
    }
    if (!ss.eof()) throw std::runtime_error("无法parse输入文件！(stringstream在getline的过程中发生错误)");;
    return nfa;
}
