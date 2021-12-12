#include <vector>
#include <string>
#include <map>
#include <time.h>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>

struct Word
{
    std::string chinese;
    std::string english;

    bool operator<(const Word& rhs) const
    {
        return english < rhs.english;
    }
};

typedef std::set<struct Word> word_book;

std::map<std::string, word_book> word_book_map;
word_book word_book_test;

std::set<struct Word> word_list_wrong;
std::string wrong_word;

int right = 0;
int wrong = 0;

bool quit = false;
std::set<struct Word>::iterator it;

bool load_word_book(const std::string &filename, bool silent)
{
    std::fstream f;
    f.open(filename, std::ios::in);
    if (!f.is_open())
    {
        std::cout << "open file failed\n";
        f.close();
        return false;
    }

    word_book book;

    char line[1024] = {};
    int count = 0;
    while (f.getline(line, sizeof(line)))
    {
        std::istringstream iss(line);
        Word word;
        iss >> word.english >> word.chinese;

        std::string skip;
        iss >> skip;

        if (skip != "x")
        {
            book.insert(word);
        }

        if (!silent) 
        {
            std::cout << "["  << ++count << "] read word: " << word.chinese << " " << word.english << " " << skip << std::endl;
        }
    }

    if (!silent) 
    {
        std::cout << "read " << filename << " completed, word count:" << book.size() << std::endl;
    }

    word_book_map[filename] = book;
    if (word_book_test.empty())
    {
        word_book_test = book;
    }
    return true;
}

void clear_stat()
{
    right = 0;
    wrong = 0;
    wrong_word = "";
    word_list_wrong.clear();
}

void next()
{
    if (word_book_test.empty())
    {
        std::cout << "没有下一题了" << std::endl;
        quit = true;
        return;
    }

    int random = rand() % word_book_test.size();
    it = word_book_test.begin();
    std::advance(it, random);
    auto &word = *it;
    std::cout << "[" << (right+wrong+1) << "] " << word.chinese << " ";
}

bool check(std::string answer)
{
    if (!wrong_word.empty())
    {
        if (answer == wrong_word)
        {
            std::cout << "=============================" << std::endl;
            wrong_word = "";
            return true;
        }
        else
        {
            return false;
        }
    }

    auto &word = *it;
    if (answer == word.english)
    {
        ++right;
        std::cout << "✅" << std::endl;
        word_book_test.erase(it);
        std::cout << "=============================" << std::endl;
        return true;
    }
    else
    {
        ++wrong;
        wrong_word = word.english;
        word_list_wrong.insert(word);
        std::cout << "❎ " << word.english << std::endl;
        return false;
    }
}

void console()
{
    while (!quit)
    {
        std::string input;
        std::getline(std::cin, input);
        std::istringstream iss(input);

        std::string cmd;
        iss >> cmd;

        if (cmd == "quit" || cmd == "q")
        {
            break;
        }
        else if (cmd == "select")
        {
            std::string bookname;
            iss >> bookname;
            auto x = word_book_map.find(bookname);
            if (x != word_book_map.end())
            {
                word_book_test = x->second;
            }
            std::cout << "选择单词表：" << bookname << "(" << word_book_test.size() << ") 重新开始测试..." << std::endl;
            clear_stat();
            next();
        }
        else if (cmd == "load")
        {
            std::string bookname;
            iss >> bookname;
            load_word_book(bookname, true);
            std::cout << "加载单词表：" << bookname << "完成" << std::endl;
        }
        else if (cmd == "print")
        {
            std::string bookname;
            iss >> bookname;
            std::cout << "单词本[" << bookname << "]" << std::endl;

            auto x = word_book_map.find(bookname);
            if (x != word_book_map.end())
            {
                int i = 0;
                for (auto &word : x->second)
                {
                    std::cout << "[" << ++i << "] " << word.chinese << " " << word.english << std::endl;
                }
            }
        }
        else if (cmd == "help")
        {
            std::cout << "加载单词本：load book-name" << std::endl;
            std::cout << "选择单词本：select book-name" << std::endl;
            std::cout << "打印单词本：print book-name" << std::endl;
            std::cout << "退出：quit or q" << std::endl;
        }
        else
        {
            if (check(cmd))
            {
                next();
            }
            else
            {
                std::cout << "请重新输入...";
            }
        }
    }
}

void print_result()
{
    std::cout << "你一共测试了" << (right+wrong) << "个单词" << std::endl;
    std::cout << "right：" << right << std::endl;
    std::cout << "wrong：" << wrong << std::endl;
    float ratio = (float)right / (right+wrong);
    std::cout << "正确率：" << (ratio * 100) << "%" << std::endl;
    std::cout << "学似逆水行舟，不进则退，勤加练习，才能每日进步!" << std::endl;

    std::ofstream f("wrong.txt", (std::ios_base::out|std::ios_base::trunc));
    if (f.is_open())
    {
        for (auto x : word_list_wrong)
        {
            f << x.chinese << " " << x.english << std::endl;
        }
    }
    else
    {
        std::cout << "打开wrong.txt失败" << std::endl;
    }
    f.close();
    std::cout << "答错单词已存入wrong.txt" << std::endl;
}

int main(int argc, char* argv[])
{
    std::string filename = "word.txt";
    bool silent = true;

    if (argc >= 2)
    {
        filename = argv[1];
    }

    if (argc >= 3)
    {
        silent = atoi(argv[2]);
    }

    srand(time(nullptr));

    load_word_book(filename, silent);

    std::cout << "测试开始..." << std::endl;

    next();
    console();

    print_result();

    return 0;    
}

