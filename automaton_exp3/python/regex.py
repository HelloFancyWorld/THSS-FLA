#!/usr/bin/env python

import json
import sys
from typing import List

import antlr4.Token
from antlr4 import CommonTokenStream, InputStream

from antlr_parser.regexLexer import regexLexer
from antlr_parser.regexParser import regexParser
from nfa import NFA

"""
本文件中已经定义好了一些类和函数，类内也已经定义好了一些成员变量和方法。不建议大家修改这些已经定义好的东西。
但是，为了实现功能，你可以自由地增加新的函数、类等，包括可以在已经定义好的类自由地添加新的成员变量和方法。

本文件可以直接作为python的入口点文件。
支持两种运行方式：
1、将输入文件的文件名作为唯一的命令行参数传入。
   例如: python regex.py ../cases/01.txt
2、若不传入任何参数，则程序将从stdin中读取输入。
"""
"""
在第二次实验中：
  - 保证正则表达式字符串和待操作的文本内容都仅包含ASCII字符，且不包含'\0'。
  - 仅保证pattern不含和换行符'\r' '\n'，但文本串可以含有换行符！
  - 要求支持的正则表达式文法为，随第三次实验下发的`regex.g4`中，所有未被注释的部分。
  - 更具体的本次实验相对上次实验新增特性的列表，请阅读实验文档。
  - 需要实现matchAll函数（返回串中所有匹配结果），replaceAll函数（将串中所有匹配结果进行替换）。
  - 同时你将会需要修改match函数和compile函数，以实现诸如返回捕获分组之类的功能。
  - 要求支持的特殊字符在`regex.g4`中均有列出。具体包括（如下一行列举的内容与`regex.g4`中的声明有冲突，请以`regex.g4`为准）：
    - \d \w \s \D \W \S . ^ $ \b \B
  - 特别注意，本次起需要支持m、s两种修饰符。
正则表达式中各种字符的具体定义可查看 https://www.runoob.com/regexp/regexp-metachar.html 
"""


class Regex:
    """
    表示一个正则表达式的类。
    """
    nfa: NFA  # 正则表达式所使用的NFA

    @staticmethod
    def parse(pattern: str) -> regexParser.RegexContext:
        """
        解析正则表达式的字符串，生成语法分析树。
        你应该在compile函数中调用一次本函数，以得到语法分析树。
        通常，你不需要改动此函数，也不需要理解此函数实现每一行的具体含义。
        但是，你应当对语法分析树的数据结构(RegexContext)有一定的理解，作业文档中有相关的教程可供参考。
        :param pattern 要解析的正则表达式的字符串
        :return regexParser.RegexContext类的对象
        """
        input_stream = InputStream(pattern)
        lexer = regexLexer(input_stream)
        stream = CommonTokenStream(lexer)
        parser = regexParser(stream)
        tree = parser.regex()
        errCount = parser.getNumberOfSyntaxErrors()
        if errCount > 0: raise ValueError("parser解析失败，表达式中有" + str(errCount) + "个语法错误！")
        if stream.LA(1) != antlr4.Token.EOF: raise ValueError(
            "parser解析失败，解析过程未能到达字符串结尾，可能是由于表达式中间有无法解析的内容！已解析的部分：" +
            stream.getText(0, stream.index - 1))
        return tree

    def compile(self, pattern: str, flags="") -> None:
        """
        编译给定的正则表达式。
        具体包括两个过程：解析正则表达式得到语法分析树（这步已经为你写好，即parse方法），
        和在语法分析树上进行分析（遍历），构造出NFA（需要你完成的部分）。
        在语法分析树上进行分析的方法，可以是直接自行访问该树，也可以是使用antlr的Visitor机制，详见作业文档。
        你编译产生的结果，NFA应保存在self.nfa中，其他内容也建议保存在当前对象下。
        :param pattern 正则表达式的字符串
        :param flags 正则表达式的修饰符
        """
        tree = Regex.parse(pattern)  # 这是语法分析树
        # TODO 请你将在上次实验的内容粘贴过来，在其基础上进行修改。

    def match(self, text: str) -> List[str]:
        """
        在给定的输入文本上，进行正则表达式匹配，返回匹配到的第一个结果。
        匹配不成功时，返回空数组[]；
        匹配成功时，返回一个由字符串组成的数组，其中下标为0的元素是匹配到的字符串，
        下标为i(i>=1)的元素是匹配结果中的第i个分组。
        :param text 输入的文本
        :return 如上所述
        """
        # TODO 请你将在上次实验的内容粘贴过来，在其基础上进行修改。
        return []

    def matchAll(self, text: str) -> List[List[str]]:
        """
        在给定的输入文本上，进行正则表达式匹配，返回匹配到的**所有**结果。
        匹配不成功时，返回空数组[]；
        匹配成功时，返回一个二维数组，其中每个元素是每一个带分组的匹配结果数组，其格式同match函数的返回值（详见上面）。
        :param text 输入的文本
        :return 如上所述
        """
        # TODO 请你完成这个函数
        return []

    def replaceAll(self, text: str, replacement: str) -> str:
        """
        在给定的输入文本上，进行基于正则表达式的替换，返回替换完成的结果。
        需要支持分组替换，如$1表示将此处填入第一个分组匹配到的内容。具体的规则和例子详见文档。
        :param text 输入的文本
        :param replacement 要将每一处正则表达式的匹配结果替换为什么内容
        :return 替换后的文本
        """
        # TODO 请你完成这个函数
        return text


if __name__ == '__main__':
    """
    程序入口点函数。已经帮你封装好了读取文本输入、调用compile方法和match等方法、输出结果等。
    一般来说，你不需要阅读和改动这里的代码，只需要完成上面Regex类中的标有TODO的函数即可。
    """
    if len(sys.argv) >= 2:
        with open(sys.argv[1], "r") as f:
            text = f.read()
    else:
        text = sys.stdin.read()

    type = ""
    pattern = None
    flags = ""
    input_str = None
    replacement = None
    lines = text.splitlines(keepends=True)
    lenBeforeInputLine = 0
    for line in lines:
        if line.startswith("type:"):
            type = line[5:].strip()
        elif line.startswith("pattern: "):
            pattern = line.splitlines()[0][9:]  # 去掉结尾的换行符
        elif line.startswith("flags:"):
            flags = line[6:].strip()
        elif line.startswith("replacement: "):
            replacement = line.splitlines()[0][13:]  # 去掉结尾的换行符
        elif line.startswith("input: "):
            input_str = text[lenBeforeInputLine + 7:]
        lenBeforeInputLine += len(line)
    if pattern is None or input_str is None:
        raise ValueError("pattern或input未找到！注意pattern: 和input: ，冒号后面必须有空格！")

    regex = Regex()
    regex.compile(pattern, flags)
    if type == "find" or type == "match":
        result = regex.match(input_str)
        print(json.dumps(result))
    elif type == "matchAll":
        result = regex.matchAll(input_str)
        print(json.dumps(result))
    elif type == "replaceAll":
        if replacement is None:
            raise ValueError("type=replaceAll的输入，但是未找到replacement字段！")
        result = regex.replaceAll(input_str, replacement)
        sys.stdout.buffer.write(result.encode("utf-8"))
    else:
        raise ValueError("不支持的输入文件类型！")
