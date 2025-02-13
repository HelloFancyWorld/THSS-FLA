#include "nfa.h"
#include <sstream>
#include <algorithm>


/**
 * 在自动机上执行指定的输入字符串。
 * @param text 输入字符串
 * @return 若拒绝，请 return Path::reject(); 。若接受，请手工构造一个Path的实例并返回。
 */


bool Rule::match(char c)
{
    switch(type)
    {
        case NORMAL:
            return (by[0] == c);
            break;
        case RANGE:
            for (char i = by[0]; i <= to[0]; i++)
            {
                if(c == i)
                    return true;
            }
            return false;
            break;
        case SPECIAL:
            switch(by[0])
            {
                case 'd':
                    for (char i = '0'; i <= '9'; i++)
                    {
                        if(c == i)
                            return true;
                    }
                    return false;
                    break;
                case 's':
                    return (c == '\f' || c =='\n' || c == '\r'
                    || c == '\t' || c == '\v' || c == ' ');
                    break;
                case 'w':
                    for (char i = '0'; i <= '9'; i++)
                    {
                        if(c == i)
                            return true;
                    }
                    for (char i = 'a'; i <= 'z'; i++)
                    {
                        if(c == i)
                            return true;
                    }
                    for (char i = 'A'; i <= 'Z'; i++)
                    {
                        if(c == i)
                            return true;
                    }
                    if(c == '_')
                        return true;
                    return false;
                    break;
                case 'D':
                    for (char i = '0'; i <= '9'; i++)
                    {
                        if(c == i)
                            return false;
                    }
                    return true;
                    break;
                case 'S':
                    if(c == '\f' || c =='\n' || c == '\r'
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
                        if(c == i)
                            return false;
                    }
                    for (char i = 'a'; i <= 'z'; i++)
                    {
                        if(c == i)
                            return false;
                    }
                    for (char i = 'A'; i <= 'Z'; i++)
                    {
                        if(c == i)
                            return false;
                    }
                    if(c == '_')
                        return false;
                    return true;
                    break;
                case '.':
                    return (c != '\r' && c != '\n');
                    break;
            }
        default:
            return false;
            break;
    }
}

Path backtrace(std::vector<Description> path)
{
    Path a;
    for (int i = 0; i < path.size(); i++)
    {
        a.states.push_back(path[i].curstate);
        if(i < path.size()  - 1)
        {
            if(path[i].remain != path[i + 1].remain)
                a.consumes.push_back({path[i].remain[0]});
            else
                a.consumes.push_back("");
        }
    }
    return a;
}



void NFA::DFS(int q, std::string text, int step)
{
    path.push_back(Description(q, text));
    std::string str = text;
    if (is_final[q] && (str.size() == 0))
    {
        result = backtrace(path);
        flag = 1;
        return;
    }
    std::vector<Description> temp = path;
    for (int i = 0; i < rules[q].size(); i++)
    {
        if(rules[q][i].type == EPSILON)
        {
            int len = str.size();
            int tempnum = -1;
            for (int j = 0; j < record_epsilon.size(); j++)
            {
                if(record_epsilon[j].remain_length == str.size())
                {
                    tempnum = j;
                    break;
                }
            }
            if(tempnum == -1)
            {
                std::vector<int> tempvector;
                tempvector.push_back(rules[q][i].dst);
                Record temprecord = Record(str.size(), tempvector);
                record_epsilon.push_back(temprecord);
                DFS(rules[q][i].dst, str, step + 1);
                path = temp;
                str = text;
            }
            else if(std::find(record_epsilon[tempnum].epsilon_circle.begin(), record_epsilon[tempnum].epsilon_circle.end(), 
                rules[q][i].dst) == record_epsilon[tempnum].epsilon_circle.end())
            {
                record_epsilon[tempnum].epsilon_circle.push_back(rules[q][i].dst);
                DFS(rules[q][i].dst, str, step + 1);
                path = temp;
                str = text;
            }
            else
                continue;
        }

        else if(str.size() > 0)
        {
            if(rules[q][i].match(str[0]))
            {
                str.erase(str.begin());
                DFS(rules[q][i].dst, str, step + 1);
                path = temp;
                str = text;
            }
        }
        if(flag ==  1)
            return;
    }
}

Path NFA::exec(std::string text) {
    // TODO 请你完成这个函数
    DFS(0, text, 0);
    if(result.states.size() > text.size())
        return result;
    else
        return Path::reject();
}

/**
 * 将Path转为（序列化为）文本的表达格式（以便于通过stdout输出）
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */
std::ostream &operator<<(std::ostream &os, Path &path) {
    if (!path.states.empty()) {
        if (path.consumes.size() != path.states.size() - 1)
            throw std::runtime_error("Path的len(consumes)不等于len(states)-1！");
        for (int i = 0; i < path.consumes.size(); ++i) {
            os << path.states[i] << " " << path.consumes[i] << " ";
        }
        os << path.states[path.states.size() - 1];
    } else os << std::string("Reject");
    return os;
}

/**
 * 从自动机的文本表示构造自动机
 * 你不需要理解此函数的含义、阅读此函数的实现和调用此函数。
 */
NFA NFA::from_text(const std::string &text) {
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
        } else if (line.find("final:") == 0) {
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
        } else if (line.find("rules:") == 0) {
            if (nfa.num_states == 0) throw std::runtime_error("states必须出现在final和rules之前!");
            reading_rules = true;
            continue;
        } else if (line.find("input:") == 0) {
            reading_rules = false;
            continue;
        } else if (reading_rules) {
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
                    Rule rule{dst};
                    if (p == 3 && content[1] == '-') {
                        rule.type = RANGE;
                        rule.by = content[0];
                        rule.to = content[2];
                    } else if (p == 2 && content[0] == '\\') {
                        if (content[1] == 'e') rule.type = EPSILON;
                        else {
                            rule.type = SPECIAL;
                            rule.by = content[1];
                        }
                    } else if (p == 1 && (p >= content.length() || content[p] == ' ')) {
                        rule.type = NORMAL;
                        rule.by = content[0];
                    } else success = false;
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
