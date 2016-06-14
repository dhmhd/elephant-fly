#include <iostream>
#include <set>
#include <fstream>
#include <map>
//#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <string>
#include <clocale>

#ifdef _WIN32
#define USE_CODECVT 1
#elif __linux__
#define USE_CODECVT 0
#elif __unix__
#define USE_CODECVT 0
#elif defined(_POSIX_VERSION)
#define USE_CODECVT 0
#else
#error "Unknown compiler"
#endif

#if USE_CODECVT
#include <codecvt>
#include <locale>
#endif 


bool oneLetterDifference(const std::wstring &w1, const std::wstring &w2)
{
    bool flag = false;
    for(size_t i = 0; i < w1.length(); ++i)
    {
        if(w1[i] != w2[i])
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
    for(size_t i = 0; i < w1.length(); ++i)
    {
        if(w1[i] != w2[i])
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
        mutable std::mutex mutex;
        std::vector<size_t> neighbors;

        void addNeighbor(size_t node_index)
        {
            std::lock_guard<std::mutex> lock(mutex);
            neighbors.push_back(node_index);
        }

    public:
        Node(size_t index, const std::wstring &value)
                : index(index), value(value)
        {
        }

        Node(const Node &node)
            : index(node.index), value(node.value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            neighbors = node.neighbors;
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
            std::lock_guard<std::mutex> lock(mutex);
            return neighbors;
        }

        bool eqStr(const std::wstring &str) const
        {
            return str.compare(value) == 0;
        }

        bool isOneLetterDiff(Node &node) const
        {
            // return index != node.index && oneLetterDifference(value, node.value);
            return oneLetterDifference(value, node.value);
        }

        void addNeighbor(Node &node)
        {
            mutex.lock();
            neighbors.push_back(node.index);
            mutex.unlock();
            node.addNeighbor(index);
        }

        void print() const
        {
            std::wcout << L"Node: " << index << L"[" << value << L"]" << std::endl;
            for(auto n : neighbors)
            {
                std::wcout << "\t" << n << std::endl;
            }
        }
    };

    std::vector<Node> nodes;
    bool isBuilded;
public:
    Graph(const std::set<std::wstring> &words)
        : isBuilded(false)
    {
        for(auto &word : words)
        {
            nodes.push_back(Node(nodes.size(), word));
        }
    }

    void build()
    {
        if(!isBuilded)
        {
//            auto startTime = std::chrono::high_resolution_clock::now();
            size_t thread_count = std::thread::hardware_concurrency();
            size_t words_count = nodes.size();
            size_t nodes_per_thread = words_count / thread_count + 1;
            size_t const NPS_MAX = 250;
            if(nodes_per_thread > NPS_MAX)
            {
                thread_count = words_count / (NPS_MAX - 1) + 1;
                nodes_per_thread = NPS_MAX;
            }
            std::vector<std::thread> threads;
            for(size_t i = 0; i < thread_count; ++i)
            {
                size_t start = i * nodes_per_thread;
                size_t end = start + nodes_per_thread;
                if(end > words_count)
                {
                    end = words_count;
                }
                auto fnk = [this](size_t start, size_t end) -> void
                {
                    for(size_t i = start; i < end; ++i)
                    {
                        auto &ni = nodes[i];
                        for(size_t j = 0; j < i; ++j)
                        {
                            auto &nj = nodes[j];
                            if(ni.isOneLetterDiff(nj))
                            {
                                ni.addNeighbor(nj);
                            }
                        }
                    }
                };
                threads.push_back(std::thread(fnk, start, end));
            }
            for(auto &thread : threads)
            {
                thread.join();
            }
            isBuilded = true;
            // auto endTime = std::chrono::high_resolution_clock::now() - startTime;
            // std::wcout << "Graph build: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
        }
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
        build();

        class Chain {
        private:
            // TODO: Постоянное копирование этого вектора!
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

        // auto startTime = std::chrono::high_resolution_clock::now();
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
                        // auto endTime = std::chrono::high_resolution_clock::now() - startTime;
                        // std::wcout << "Search[+]: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
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

        // auto endTime = std::chrono::high_resolution_clock::now() - startTime;
        // std::wcout << "Search[-]: " << std::chrono::duration_cast<std::chrono::microseconds>(endTime).count() << " microseconds" << std::endl;
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
        #if USE_CODECVT
            std::locale ru(std::locale(), new std::codecvt_utf8<wchar_t>);
        #else
            std::locale ru("en_US.UTF-8");
        #endif
        input.imbue(ru);
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
        else
        {
            std::wcout << L"Проблемы при открытии файла словаря" << std::endl;
        }
    }

    std::vector<std::wstring> search(const std::wstring &w1, const std::wstring &w2)
    {
        size_t length = w1.length();

        std::vector<std::wstring> result;

        if(length != w2.length())
        {
            std::wcout << L"Разная длинна слов" << std::endl;
            return std::move(result);
        }

        if(wordsets.find(length) == wordsets.end())
        {
            std::wcout << L"Нет слов данной длинны" << std::endl;
            return std::move(result);
        }

        if(graphs.find(length) == graphs.end())
        {
            std::wcout << L"Нет графа для слов данной длинны" << std::endl;
            return std::move(result);
        }

        if(wordsets[length].find(w1) == wordsets[length].end())
        {
            std::wcout << L"В словаре нет слова: " << w1 << std::endl;
            return std::move(result);
        }

        if(wordsets[length].find(w2) == wordsets[length].end())
        {
            std::wcout << L"В словаре нет слова: " << w2 << std::endl;
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
    #if USE_CODECVT
        std::setlocale(LC_ALL, "Russian");
        std::locale ru(std::locale(), new std::codecvt_utf8<wchar_t>());
    #else
        std::setlocale(LC_ALL, "en_US.UTF-8");
        std::locale ru("en_US.UTF-8");
    #endif

    if(argc < 3)
    {
        printUsage(argv[0]);
        return 0;
    }

    // std::string dictionary_file_name = "/usr/share/dict/words";
    // std::string dictionary_file_name = "/home/dhmhd/Projects/elephant-fly/words";
    std::string dictionary_file_name = argv[2];
    Dictionary dictionary(dictionary_file_name);

    std::wstring w[2];
    std::string input_file_name = argv[1];
    std::wifstream input(input_file_name);
    input.imbue(ru);
    if(input.is_open())
    {
        int i = 0;
        for (std::wstring word; std::getline(input, word) && i < 2; i++)
        {
            w[i] = word;
        }
        auto r = dictionary.search(w[0], w[1]);
        if(r.size() == 0)
        {
            std::wcout << L"Цепочки не существует" << std::endl;
        }
        else
        {
            printSearchResult(r);
        }
    }
    else
    {
        std::wcout << L"Проблемы при открытии файла" << std::endl;
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
