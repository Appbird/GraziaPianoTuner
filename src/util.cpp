# include "util.hpp"
# include <regex>
String find_last_abc_block(const String& GPT_answer)
{
    const std::regex regex{"```abc\\n([^`]+?)\\n```"};
    std::string str_utf8 = GPT_answer.toUTF8();
    std::smatch tmp_matches;
    std::smatch matches;
    std::string::const_iterator text_iter = str_utf8.cbegin();
    while (std::regex_search(text_iter, str_utf8.cend(), tmp_matches, regex)){
        matches = tmp_matches;
        text_iter = tmp_matches[0].second;
    }
    assert(matches.size() > 0);
    
    const std::string last_match_str = matches.str(matches.size() - 1);
    return Unicode::FromUTF8(last_match_str);
}