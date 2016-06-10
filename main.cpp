#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <algorithm>

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
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                wordsets[word.length()].insert(word);
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
