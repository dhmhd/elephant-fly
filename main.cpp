#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <algorithm>

class Dictionary {
private:
    std::map<size_t, std::set<std::string>> wordsets;
public:
    Dictionary(const std::string &file_name)
    {
        std::ifstream input(file_name);
        if(input.is_open())
        {
            for (std::string word; std::getline(input, word);)
            {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                wordsets[word.length()].insert(word);
            }

            for(auto wordset : wordsets)
            {
                std::cout << wordset.first << std::endl;
                for(auto word : wordset.second)
                {
                    std::cout << "\t" << word << std::endl;                    
                }
            }
        }
    }
};

int main()
{
    //std::string dictionary_file_name = "/usr/share/dict/words";
    std::string dictionary_file_name = "/home/dhmhd/Projects/elephant-fly/words";
    Dictionary dictionary(dictionary_file_name);
    return 0;
}
