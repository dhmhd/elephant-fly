#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <chrono>
#include <vector>

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
        std::wcout << "Graph build: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime).count() << " milliseconds" << std::endl;
    }

    void print()
    {
        for(auto &node : nodes)
        {
            node.print();
        }
    }
};

class Dictionary {
private:
    std::map<size_t, std::set<std::wstring>> wordsets;
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
                Graph *graph = new Graph(wordset.second);
                graph->build();
                graph->print();
                delete graph;
            }
        }
    }
};

int main()
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
    //std::string dictionary_file_name = "/usr/share/dict/words";
    std::string dictionary_file_name = "/home/dhmhd/Projects/elephant-fly/words";
    Dictionary dictionary(dictionary_file_name);

    return 0;
}
