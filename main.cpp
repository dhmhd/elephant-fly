#include <iostream>
#include <set>
#include <fstream>
#include <map>
#include <algorithm>

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

    std::wcout << (oneLetterDifference(L"foo", L"bar") == false) << std::endl;
    std::wcout << (oneLetterDifference(L"foo", L"boo") == true) << std::endl;
    std::wcout << (oneLetterDifference(L"fof", L"bof") == true) << std::endl;
    std::wcout << (oneLetterDifference(L"fof", L"boo") == false) << std::endl;
    std::wcout << (oneLetterDifference(L"fof0", L"bof0") == true) << std::endl;
    std::wcout << (oneLetterDifference(L"fof0", L"boo0") == false) << std::endl;
    std::wcout << (oneLetterDifference(L"sof0", L"bof0") == true) << std::endl;
    std::wcout << (oneLetterDifference(L"f1f0", L"boo0") == false) << std::endl;
    std::wcout << (oneLetterDifference(L"asfoff", L"asboff") == true) << std::endl;
    std::wcout << (oneLetterDifference(L"asoff", L"asoxf") == true) << std::endl;

    return 0;
}
