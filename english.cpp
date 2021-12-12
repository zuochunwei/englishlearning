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

enum POLICY
{
    RAND, //随机
    ORDER //顺序
} policy = RAND;

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
word_book test_set;

std::set<std::string> word_book_selected;

std::set<struct Word> word_list_wrong;

std::string wrong_word;

int right = 0;
int wrong = 0;
int test_count_max = 10000;

int test_count()
{
    return right + wrong;
}

bool quit = false;
std::set<struct Word>::iterator it;

void add_word_book(const std::string& bookname)
{
    auto it = word_book_map.find(bookname);
    if (it != word_book_map.end())
    {
        test_set.insert(it->second.begin(), it->second.end());
    }
}

void build_test_set()
{
    test_set.clear();
    for (auto &x : word_book_selected)
    {
        std::cout << "build_test_set:" << x << std::endl;
        add_word_book(x);
    }
    std::cout << "测试集构建完毕(" << test_set.size() << ")" << std::endl;
}

void clear_stat()
{
    right = 0;
    wrong = 0;
    wrong_word = "";
    word_list_wrong.clear();
}

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
    return true;
}

void change_policy(POLICY new_policy)
{
    if (new_policy != policy)
    {
        if (new_policy == ORDER)
        {
            it = test_set.begin();
        }
        else
        {
        }
    }
}

void next()
{
    if (test_set.empty())
    {
        std::cout << "没有下一题了" << std::endl;
        quit = true;
        return;
    }

    if (test_count() >= test_count_max)
    {
        std::cout << "到达最大测试数量" << std::endl;
        quit = true;
        return;
    }

    Word word;
    if (policy == RAND)
    {
        int random = rand() % test_set.size();
        it = test_set.begin();
        std::advance(it, random);
        word = *it;
    }
    else if (policy == ORDER)
    {
        if (it != test_set.end())
        {
            word = *it;
            ++it;
        }
    }
    else 
    {
        std::cout << "policy error" << std::endl;
    }

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
        std::cout << "✅" << std::endl;
        ++right;
        test_set.erase(it);
        std::cout << "=============================" << std::endl;
        return true;
    }
    else
    {
        wrong_word = word.english;
        if (word_list_wrong.insert(word).second)
        {
            ++wrong;
        }
        std::cout << "❎ " << word.english << std::endl;
        return false;
    }
}

void restart()
{
    build_test_set();
    clear_stat();
    next();
}

void merge()
{
    std::fstream f;
    f.open("file.list", std::ios::in);
    if (!f.is_open())
    {
        std::cout << "open file.list failed\n";
        f.close();
        return;
    }

    word_book_selected.clear();
    char word_book[1024] = {};
    while (f.getline(word_book, sizeof(word_book)))
    {
        load_word_book(word_book, false);
        word_book_selected.insert(word_book);
        std::cout << word_book << std::endl;
    }

    restart();
}

void save(const std::string& filename)
{
    std::ofstream f(filename, (std::ios_base::out|std::ios_base::trunc));
    if (!f.is_open())
    {
        std::cout << "打开wrong.txt失败" << std::endl;
    }
    else
    {
        std::set<struct Word> wordset;
        for (auto x : word_book_map)
        {
            for (auto y : x.second)
            {
                wordset.insert(y);
            }
        }

        for (auto x : wordset)
        {
            f << x.english << " " << x.chinese << std::endl;
        }
    }
    f.close();
    std::cout << "save OK" << std::endl;
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
                test_set = x->second;
            }
            std::cout << "选择单词表：" << bookname << "(" << test_set.size() << ") 重新开始测试..." << std::endl;
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
        else if (cmd == "add")
        {
            std::string bookname;
            iss >> bookname;
            auto x = word_book_map.find(bookname);
            if (x != word_book_map.end())
            {
                add_word_book(bookname);
                word_book_selected.insert(bookname);
                std::cout << "添加单词表：" << bookname << "成功" << std::endl;
            }
            std::cout << "添加单词表：" << bookname << "成功" << std::endl;
        }
        else if (cmd == "merge")
        {
            merge();
        }
        else if (cmd == "save")
        {
            std::string bookname;
            iss >> bookname;
            if (bookname.empty()) bookname = "save.txt";
            save(bookname);
        }
        else if (cmd == "restart")
        {
            restart();
        }
        else if (cmd == "wordcount")
        {
            std::cout << "wordcount:" << test_set.size() << std::endl;
        }
        else if (cmd == "order")
        {
            change_policy(ORDER);
            std::cout << "策略改为顺序出题" << std::endl;
            next();
        }
        else if (cmd == "rand")
        {
            change_policy(RAND);
            std::cout << "策略改为随机出题" << std::endl;
            next();
        }
        else if (cmd == "maxcount")
        {
            std::string max_count;
            iss >> max_count;
            int mc = atoi(max_count.c_str());
            if (mc > 0)
            {
                test_count_max = mc;
            }
            next();
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
            std::cout << "添加单词本：add book-name" << std::endl;
            std::cout << "打印单词本：print book-name" << std::endl;
            std::cout << "打印单词数：wordcount" << std::endl;
            std::cout << "设置最大测试单词数：maxcount num" << std::endl;
            std::cout << "合并所有单词本：merge" << std::endl;
            std::cout << "重新开始：restart" << std::endl;
            std::cout << "随机测试：rand" << std::endl;
            std::cout << "顺序测试：order" << std::endl;
            std::cout << "保存：save [filename]" << std::endl;
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
    std::string filename = "word4.txt";
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

    if (test_set.empty())
    {
        word_book_selected.insert(filename);
        build_test_set();
    }

    std::cout << "测试开始..." << std::endl;

    next();
    console();

    print_result();

    return 0;    
}

