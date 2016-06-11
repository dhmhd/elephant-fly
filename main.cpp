#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <unordered_set>

bool oneLetterDifference(const std::wstring &w1, const std::wstring &w2)
{
    bool flag = false;
    for(auto i1 = w1.cbegin(), i2 = w2.cbegin(); i1 != w1.cend() && i2 != w2.cend(); i1++, i2++)
    {
        if(*i1 != *i2)
        {
            if(!flag)
            {
                flag = true;
            }
            else
            {
                flag = false;
                break;
            }
        }
    }
    return flag;
}

size_t differentLetters(const std::wstring &w1, const std::wstring &w2)
{
    size_t count = 0;
    for(auto i1 = w1.cbegin(), i2 = w2.cbegin(); i1 != w1.cend() && i2 != w2.cend(); i1++, i2++)
    {
        if(*i1 != *i2)
        {
            ++count;
        }
    }
    return count;
}

class Graph {
private:
    class Node {
    private:
        const size_t index;
        const std::wstring &value;
        std::vector<size_t> neighbors;
    public:
        Node(size_t index, const std::wstring &value)
                : index(index), value(value)
        {
        }

        size_t getIndex(void) const
        {
            return index;
        }

        const std::wstring &getValue() const
        {
            return value;
        }

        const std::vector<size_t> &getNeighbors() const
        {
            return neighbors;
        }

        bool eqStr(const std::wstring &str) const
        {
            return str == value;
        }

        void tryToAddNeighbor(Node &node)
        {
            // std::wcout << "[d]" << value << " ~ " << node.value << std::endl;
            if(oneLetterDifference(node.value, value))
            {
                neighbors.push_back(node.index);
                node.neighbors.push_back(index);
            }
        }

        void print()
        {
            std::wcout << "Node: " << index << "[" << value << "]" << std::endl;
            for(auto n : neighbors)
            {
                std::wcout << "\t" << n << std::endl;
            }
        }
    };

    const std::set<std::wstring> &words;
    std::vector<Node> nodes;
public:
    Graph(const std::set<std::wstring> &words)
            : words(words)
    {
    }

    void build()
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        for(auto &word : words)
        {
            Node cur(nodes.size(), word);
            for(auto &node : nodes)
            {
                cur.tryToAddNeighbor(node);
            }
            nodes.push_back(cur);
        }
        auto endTime = std::chrono::high_resolution_clock::now() - startTime;
        std::wcout << "Graph build: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
    }

    void print()
    {
        for(auto &node : nodes)
        {
            node.print();
        }
    }
    
    std::vector<std::wstring> search(const std::wstring &w1, const std::wstring &w2)
    {
        if(nodes.empty())
        {
            build();
        }

        class Chain {
        private:
            std::vector<size_t> chain;
            size_t priority;
        public:
            size_t back() const
            {
                return chain.back();
            }

            void push(size_t index, size_t diff)
            {
                chain.push_back(index);
                priority = chain.size() + diff;
            }

            size_t getPriority() const
            {
                return priority;
            }

            std::vector<size_t>::iterator begin()
            {
                return chain.begin();
            }

            std::vector<size_t>::iterator end()
            {
                return chain.end();
            }
        };

        auto comparator = [](const Chain &left, const Chain &right) -> bool
        {
            return left.getPriority() > right.getPriority();
        };

        auto startTime = std::chrono::high_resolution_clock::now();
        auto start_node = std::find_if(nodes.begin(), nodes.end(), [&w1](const Node &node) -> bool {return node.eqStr(w1);});
        if(start_node != nodes.end())
        {
            std::priority_queue<Chain, std::vector<Chain>, decltype(comparator)> queue(comparator);
            std::unordered_set<std::wstring> visided;
            Chain chain;
            chain.push(start_node->getIndex(), differentLetters(w2, w1));
            queue.push(chain);
            while (!queue.empty())
            {
                auto q = queue.top();
                queue.pop();
                auto index = q.back();
                for (auto n : nodes[index].getNeighbors())
                {
                    if(nodes[n].eqStr(w2))
                    {
                        std::vector<std::wstring> result;
                        for(auto i : q)
                        {
                            result.push_back(nodes[i].getValue());
                        }
                        result.push_back(w2);
                        auto endTime = std::chrono::high_resolution_clock::now() - startTime;
                        std::wcout << "Search: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
                        return result;
                    }
                    if(visided.find(nodes[n].getValue()) == visided.end())
                    {
                        visided.insert(nodes[n].getValue());
                        Chain new_chain(q);
                        new_chain.push(n, differentLetters(w2, nodes[n].getValue()));
                        queue.push(new_chain);
                    }
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now() - startTime;
        std::wcout << "Search: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
        return std::vector<std::wstring>();
    }
};

class Dictionary {
private:
    std::map<size_t, std::set<std::wstring>> wordsets;
    std::map<size_t, std::unique_ptr<Graph>> graphs;
public:
    Dictionary(const std::string &file_name)
    {
        std::wifstream input(file_name);
        input.imbue(std::locale("en_US.UTF-8"));
        if(input.is_open())
        {
            for (std::wstring word; std::getline(input, word);)
            {
                // std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                // Алфавит не указан в условиях задачи, поэтому включаю все возможные символы:
                // заглавные, строчные, знаки препинания, пробелы...
                wordsets[word.length()].insert(word);
            }

            for (auto &wordset : wordsets)
            {
                std::unique_ptr<Graph> graph(new Graph(wordset.second));
                graphs[wordset.first] = std::move(graph);                
            }
        }
    }

    std::vector<std::wstring> search(const std::wstring &w1, const std::wstring &w2)
    {
        size_t length = w1.length();

        std::vector<std::wstring> result;

        if(length != w2.length())
        {
            std::wcout << "Разная длинна слов" << std::endl;
            return std::move(result);
        }

        if(wordsets.find(length) == wordsets.end())
        {
            std::wcout << "Нет слов данной длинны" << std::endl;
            return std::move(result);
        }

        if(graphs.find(length) == graphs.end())
        {
            std::wcout << "Нет графа для слов данной длинны" << std::endl;
            return std::move(result);
        }

        if(wordsets[length].find(w1) == wordsets[length].end())
        {
            std::wcout << "В словаре нет слова: " << w1 << std::endl;
            return std::move(result);
        }

        if(wordsets[length].find(w2) == wordsets[length].end())
        {
            std::wcout << "В словаре нет слова: " << w2 << std::endl;
            return std::move(result);
        }

        auto &g = graphs[length];
        return g->search(w1, w2);
    }
};

void printSearchResult(const std::vector<std::wstring> &result)
{
    for(auto &str : result)
    {
        std::wcout << str << std::endl;
    }
}

void printUsage(char *str)
{
    std::wcout << "Usage: " << str << " input_file dictionary_file" << std::endl;
    std::wcout << std::endl;

}

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "en_US.UTF-8");

    if(argc < 3)
    {
        printUsage(argv[0]);
    }

    // std::string dictionary_file_name = "/usr/share/dict/words";
    // std::string dictionary_file_name = "/home/dhmhd/Projects/elephant-fly/words";
    std::string dictionary_file_name = argv[2];
    Dictionary dictionary(dictionary_file_name);

    std::wstring w[2];
    std::string input_file_name = argv[1];
    std::wifstream input(input_file_name);
    input.imbue(std::locale("en_US.UTF-8"));
    if(input.is_open())
    {
        int i = 0;
        for (std::wstring word; std::getline(input, word) || i < 2; i++)
        {
            w[i] = word;
        }
        auto r = dictionary.search(w[0], w[1]);
        if(r.size() == 0)
        {
            std::wcout << "Цепочки не существует" << std::endl;
        }
        else
        {
            printSearchResult(r);
        }
    }
    else
    {
        std::wcout << "Проблемы при открытии файла" << std::endl;
    }

    // printSearchResult(dictionary.search(L"top", L"hot"));
    // printSearchResult(dictionary.search(L"stop", L"flag"));
    // printSearchResult(dictionary.search(L"clock", L"store"));
    // printSearchResult(dictionary.search(L"store", L"clock"));
    // printSearchResult(dictionary.search(L"ledges", L"poking"));
    // printSearchResult(dictionary.search(L"poking", L"ledges"));
    // printSearchResult(dictionary.search(L"left", L"mime"));
    // printSearchResult(dictionary.search(L"single", L"simple"));
    // printSearchResult(dictionary.search(L"simple", L"single"));

    return 0;
}
