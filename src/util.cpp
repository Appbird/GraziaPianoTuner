# include "util.hpp"
# include <regex>

static String find_last_block(const String& GPT_answer, const std::string& kind) {
    const std::regex regex{"```"+kind+"\\n([^`]+?)\\n```"};
    std::string str_utf8 = GPT_answer.toUTF8();
    std::smatch tmp_matches;
    std::smatch matches;
    std::string::const_iterator text_iter = str_utf8.cbegin();
    while (std::regex_search(text_iter, str_utf8.cend(), tmp_matches, regex)){
        matches = tmp_matches;
        text_iter = tmp_matches[0].second;
    }
    if (matches.size() == 0) { return U""; }
    
    const std::string last_match_str = matches.str(matches.size() - 1);
    return Unicode::FromUTF8(last_match_str);
}

String find_last_abc_block(const String& GPT_answer)
{
    return find_last_block(GPT_answer, "abc");
}

String find_last_axis_block(const String& GPT_answer)
{
    return find_last_block(GPT_answer, "axis");
}