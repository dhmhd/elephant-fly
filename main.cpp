#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>

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

        //print();
        typedef std::vector<size_t> Chain;

        auto startTime = std::chrono::high_resolution_clock::now();
        auto start_node = std::find_if(nodes.begin(), nodes.end(), [&w1](const Node &node) -> bool {return node.eqStr(w1);});
        if(start_node != nodes.end())
        {
            std::set<std::wstring> visided;
            std::queue<Chain> queue;
            Chain chain;
            chain.push_back(start_node->getIndex());
            queue.push(chain);
            while (!queue.empty())
            {
                auto &q = queue.front();
                size_t node_index = q.back();
                for(auto n : nodes[node_index].getNeighbors())
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
                        new_chain.push_back(n);
                        queue.push(new_chain);
                    }
                }
                queue.pop();
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

int main()
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
    std::string dictionary_file_name = "/usr/share/dict/words";
    //std::string dictionary_file_name = "/home/dhmhd/Projects/elephant-fly/words";
    Dictionary dictionary(dictionary_file_name);

    printSearchResult(dictionary.search(L"top", L"hot"));
    printSearchResult(dictionary.search(L"stop", L"flag"));
    printSearchResult(dictionary.search(L"clock", L"store"));
    printSearchResult(dictionary.search(L"store", L"clock"));
    printSearchResult(dictionary.search(L"ledges", L"poking"));
    printSearchResult(dictionary.search(L"poking", L"ledges"));
    printSearchResult(dictionary.search(L"left", L"mime"));
    printSearchResult(dictionary.search(L"single", L"simple"));
    printSearchResult(dictionary.search(L"simple", L"single"));

    return 0;
}
