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
#include <iterator>

#if 1
size_t split_string(const std::string& input, std::vector<std::string>& output)
{
    std::istringstream iss(input);
    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(output));
    return output.size();
}
#else
size_t split_string(const std::string& input, std::vector<std::string>& output)
{
    size_t pos = 0;
    size_t i = 0;
    while (i < input.size())
    {
        char c = input[i];
        if (c == ' ')
        {
            output.push_back(input.substr(pos, i - pos));
            pos = -1;
        }
        else
        {
            if (pos == -1)
            {
                pos = i;
            }
        }
        ++i;
    }

    if (pos != -1)
    {
        output.push_back(input.substr(pos, i - pos));
    }

    return output.size();
}
#endif

//单词
struct Word
{
    std::string chinese;
    std::string english;

    Word() : chinese("空"), english("none") {}
    Word(const std::string& chinese, const std::string& english) : chinese(chinese), english(english) {}

    bool operator<(const Word& rhs) const
    {
        return english < rhs.english;
    }

    bool operator<(const std::string& rhs) const
    {
        return english < rhs;
    }

    bool operator==(const std::string& english) const
    {
        return this->english == english;
    }

    bool is_valid() const
    {
        return !chinese.empty() && !english.empty();
    }
};

//单词本
struct WordBook
{
    std::string name;
    std::vector<Word> list;

    WordBook() = default;

    WordBook(const std::string& name, const std::vector<Word>& list) : name(name), list(list) {}
};

//单词本管理器
struct WordBookManager
{
    static WordBookManager& instance()
    {
        static WordBookManager wbm;
        return wbm;
    }

    bool init(const std::string& filelist)
    {
        std::fstream f;
        f.open(filelist, std::ios::in);
        if (!f.is_open())
        {
            std::cout << "open file " << filelist << " failed" << std::endl;
            f.close();
            return false;
        }

        char word_book[1024] = {};
        while (f.getline(word_book, sizeof(word_book)))
        {
            load(word_book, false);
        }
        f.close();
        return true;
    }

    WordBook* get(const std::string& bookname)
    {
        auto x = books_map.find(bookname);
        if (x != books_map.end())
        {
            return &x->second;
        }
        return nullptr;
    }

    void add(const WordBook& wb)
    {
        auto x = books_map.find(wb.name);
        if (x != books_map.end())
        {
            std::cout << "update wordbook:" << wb.name << std::endl;
        }
        books_map[wb.name] = wb;
    }

    bool load(const std::string &wordbook, bool silent)
    {
        std::fstream f;
        f.open(wordbook, std::ios::in);
        if (!f.is_open())
        {
            std::cout << "open file " << wordbook << " failed" << std::endl;
            f.close();
            return false;
        }

        std::set<Word> set; //去重
        std::vector<Word> word_list;
        char line[1024] = {};
        while (f.getline(line, sizeof(line)))
        {
            std::istringstream iss(line);
            Word word;
            iss >> word.english >> word.chinese;
            if (!word.is_valid())
            {
                printf("invalid word\n");
                continue;
            }

            std::string skip;
            iss >> skip;
            if (skip != "x")
            {
                if (set.insert(word).second && !silent)
                {
                    word_list.push_back(word);
                    std::cout << "["  << set.size() << "] read word: " << word.chinese << " " << word.english << " " << skip << std::endl;
                }
            }
        }

        if (!silent) 
        {
            std::cout << "read " << wordbook << " completed, word count:" << set.size() << std::endl;
        }

        add(WordBook(wordbook, word_list));
        if (default_book.empty())
        {
            default_book = wordbook;
        }
        return true;
    }

    size_t word_count() const
    {
        size_t count = 0;
        for (auto x : books_map)
        {
            count += x.second.list.size();
        }
        return count;
    }

    std::map<std::string, WordBook> books_map;
    std::string default_book;
};

enum POLICY
{
    RAND, //随机
    ORDER //顺序
};

using range_t = std::pair<int, int>;

const static range_t range_all = {0, std::numeric_limits<int>::max()};

struct TestWordInfo
{
    std::set<Word> word_set;
    std::vector<Word> word_list;
    size_t word_list_cursor;

    void clear()
    {
        word_set.clear();
        word_list.clear();
        word_list_cursor = 0;
    }

    void add_word_book(const WordBook* wordbook, const range_t range)
    {
        for (size_t i = 0; i < wordbook->list.size(); ++i)
        {
            if (i >= range.first && i < range.second)
            {
                auto word = wordbook->list[i];
                if (word_set.insert(word).second)
                {
                    word_list.push_back(word);
                }
            }
        }
    }

    const Word* get_next_word(POLICY policy)
    {
        if (policy == ORDER)
        {
            if (word_list_cursor < word_list.size())
            {
                return &word_list[word_list_cursor];
            }
        } 
        else if (policy == RAND)
        {
            if (!word_set.empty())
            {
                int random = rand() % word_set.size();
                auto x = word_set.begin();
                std::advance(x, random);
                return &(*x);
            }
        }
        
        return nullptr;
    }

    void on_reply(POLICY policy, const std::string& answer, bool right)
    {
        if (right)
        {
            if (policy == ORDER)
                ++word_list_cursor;
            else
            {
                Word w;
                w.english = answer;
                word_set.erase(w);
            }
        }
        else
        {
            if (policy == ORDER)
            {
                auto word = word_list[word_list_cursor];
                word_list.erase(word_list.begin() + word_list_cursor);
                word_list.push_back(word);
            }
        }
    }

    size_t word_count() const
    {
        return word_list.size();
    }
};

//测验
struct Test
{
    Test()
    {
        srand(time(nullptr));
    }

    static Test& instance()
    {
        static Test test;
        return test;
    }

    void start(const std::string filelist)
    {
        std::cout << "测试开始..." << std::endl;
        WordBookManager::instance().init(filelist);
        select_word_book(WordBookManager::instance().default_book, range_all);
        build_test_set();
        next();
        process_input();
        print_result();
    }

    void restart()
    {
        build_test_set();
        clear_stat();
        next();
    }

    int get_test_count() const
    {
        return right + wrong;
    }

    bool select_word_book(const std::string& bookname, range_t range)
    {
        if (!WordBookManager::instance().get(bookname))
        {
            return false;
        }
        word_book_selector.clear();
        word_book_selector[bookname] = range;
        return true;
    }

    void build_test_set()
    {
        test_word_info.clear();
        for (auto x : word_book_selector)
        {
            std::cout << "build test set from " << x.first << std::endl;
            if (auto book = WordBookManager::instance().get(x.first))
            {
                test_word_info.add_word_book(book, x.second);
            }
        }
        std::cout << "测试集构建完毕(" << test_word_info.word_count() << ")" << std::endl;
    }

    void clear_stat()
    {
        right = 0;
        wrong = 0;
        wrong_word = "";
        wrong_set.clear();
    }

    void change_policy(POLICY new_policy)
    {
        if (new_policy != policy)
        {
            policy = new_policy;
            if (new_policy == ORDER)
            {
                test_word_info.word_list_cursor = 0;
            }
        }
    }

    void next()
    {
        if (get_test_count() >= test_count)
        {
            std::cout << "到达最大测试数量" << std::endl;
            quit = true;
            return;
        }

        auto word = test_word_info.get_next_word(policy);
        if (word == nullptr)
        {
            quit = true;
        }
        else
        {
            testing_question = *word;
            std::cout << "[" << (get_test_count()+1) << "] " << testing_question.chinese << " ";
        }
    }

    bool check(std::string answer)
    {
        if (!wrong_word.empty())
        {
            if (answer == wrong_word)
            {
                std::cout << "=============================" << std::endl;
                wrong_word = "";
                test_word_info.on_reply(policy, answer, false);
                return true;
            }
            else
            {
                return false;
            }
        }

        if (answer == testing_question.english)
        {
            std::cout << "✅" << std::endl;
            if (wrong_set.find(Word("", answer)) == wrong_set.end())
            {
                ++right;
            }
            test_word_info.on_reply(policy, answer, true);
            std::cout << "=============================" << std::endl;
            return true;
        }
        else
        {
            wrong_word = testing_question.english;
            if (wrong_set.insert(testing_question).second)
            {
                ++wrong;
            }
            std::cout << "❎ " << testing_question.english << std::endl;
            return false;
        }
    }

    void process_input()
    {
        while (!quit)
        {
            std::string input;
            std::getline(std::cin, input);
            
            std::vector<std::string> string_list;
            if (split_string(input, string_list) == 0)
            {
                string_list.push_back("");
            }

            std::string& cmd = string_list[0];
            if (cmd == "Quit" || cmd == "q")
            {
                break;
            }
            else if (cmd == "Select")
            {
                std::string& bookname = string_list[1];
                int from = 0, to = std::numeric_limits<int>::max();
                if (string_list.size() == 4)
                {
                    from = atoi(string_list[2].c_str());
                    to = atoi(string_list[3].c_str());
                }
                Test::instance().select_word_book(bookname, {from, to});
                std::cout << "选择单词表：" << bookname << "(" << test_word_info.word_count() << ") 重新开始测试..." << std::endl;
                restart();
            }
            else if (cmd == "Save")
            {
                std::string& bookname = string_list[1];
                if (bookname.empty()) bookname = "save.txt";
                save(bookname);
            }
            else if (cmd == "Restart")
            {
                restart();
            }
            else if (cmd == "Wordcount")
            {
                std::cout << "wordcount:" << WordBookManager::instance().word_count() << std::endl;
            }
            else if (cmd == "Order")
            {
                Test::instance().change_policy(ORDER);
                std::cout << "策略改为顺序出题" << std::endl;
                next();
            }
            else if (cmd == "Rand")
            {
                Test::instance().change_policy(RAND);
                std::cout << "策略改为随机出题" << std::endl;
                next();
            }
            else if (cmd == "Load")
            {
                std::string& bookname = string_list[1];
                WordBookManager::instance().load(bookname, true);
                std::cout << "加载单词表：" << bookname << "完成" << std::endl;
            }
            else if (cmd == "Add")
            {
                std::string& bookname = string_list[1];
                auto book = WordBookManager::instance().get(bookname);
                if (book != nullptr)
                {
                    Test::instance().select_word_book(bookname, range_all);
                    Test::instance().build_test_set();
                    std::cout << "添加单词表：" << bookname << "成功" << std::endl;
                }
            }
            else if (cmd == "Merge")
            {
                if (Test::instance().merge())
                {
                    Test::instance().restart();
                }
            }
            else if (cmd == "Testcount")
            {
                int max_count = atoi(string_list[1].c_str());
                if (max_count > 0)
                {
                    Test::instance().test_count = max_count;
                }
                next();
            }
            else if (cmd == "Print")
            {
                std::string& bookname = string_list[1];
                if (bookname.empty())
                {
                    bookname = Test::instance().word_book_selector.begin()->first;
                }

                std::cout << "打印单词本:" << bookname << std::endl;
                if (auto book = WordBookManager::instance().get(bookname))
                {
                    int i = 0;
                    for (auto &word : book->list)
                    {
                        //std::cout << "[" << ++i << "] " << word.english << " " << word.chinese << std::endl;
                        printf("[%4d] %-15s %-15s\n", ++i, word.english.c_str(), word.chinese.c_str());
                    }
                }
            }
            else if (cmd == "Help")
            {
                std::cout << "加载单词本：Load book-name" << std::endl;
                std::cout << "选择单词本：Select book-name [range_from, range_to)" << std::endl;
                std::cout << "添加单词本：Add book-name" << std::endl;
                std::cout << "打印单词本：Print book-name" << std::endl;
                std::cout << "打印单词数：Wordcount" << std::endl;
                std::cout << "设置测试单词数：Testcount wordcount" << std::endl;
                std::cout << "合并所有单词本：Merge" << std::endl;
                std::cout << "重新开始：Restart" << std::endl;
                std::cout << "随机测试：Rand" << std::endl;
                std::cout << "顺序测试：Order" << std::endl;
                std::cout << "保存：Save [filename]" << std::endl;
                std::cout << "退出：Quit or q" << std::endl;
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
            char buf[1024] = {};
            for (auto x : wrong_set)
            {
                sprintf(buf, "%-15s %-15s", x.english.c_str(), x.chinese.c_str());
                f << buf << std::endl;
            }
        }
        else
        {
            std::cout << "打开wrong.txt失败" << std::endl;
        }
        f.close();
        std::cout << "答错单词已存入wrong.txt" << std::endl;
    }

    bool save(const std::string& filename)
    {
        std::ofstream f(filename, (std::ios_base::out|std::ios_base::trunc));
        if (!f.is_open())
        {
            std::cout << "打开" << filename << "失败" << std::endl;
            f.close();
            return false;
        }

        std::set<struct Word> wordset;
        for (auto x : WordBookManager::instance().books_map)
        {
            for (auto y : x.second.list)
            {
                wordset.insert(y);
            }
        }

        char buf[1024] = {};
        for (auto x : wordset)
        {
            sprintf(buf, "%-15s %-15s", x.english.c_str(), x.chinese.c_str());
            f << buf << std::endl;
        }
        std::cout << "save OK, total word count:" << wordset.size() << std::endl;
        f.close();
        return true;
    }

    bool merge()
    {
        std::fstream f;
        f.open("file.list", std::ios::in);
        if (!f.is_open())
        {
            std::cout << "open file.list failed\n";
            f.close();
            return false;
        }

        word_book_selector.clear();
        char word_book[1024] = {};
        while (f.getline(word_book, sizeof(word_book)))
        {
            if (WordBookManager::instance().load(word_book, false))
            {
                word_book_selector[word_book] = range_all;
                std::cout << "merged:" << word_book << std::endl;
            }
        }
        f.close();
        return true;
    }

    POLICY policy = ORDER;

    std::map<std::string, range_t> word_book_selector;

    TestWordInfo test_word_info;

    std::string wrong_word;

    std::set<Word> wrong_set;

    Word testing_question; //正在测试的问题

    int right = 0;
    int wrong = 0;
    int test_count = 30;

    bool quit = false;
};

int main(int argc, char* argv[])
{
    Test::instance().start(argc >= 2 ? argv[1] : "file.list");
    return 0;    
}

