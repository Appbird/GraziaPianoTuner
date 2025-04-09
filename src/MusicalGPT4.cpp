# include "MusicalGPT4.hpp"
# include "util.hpp"
# include "EmotionalController.hpp"

static OpenAI::Chat::Role str2role(const String& role){
    using namespace OpenAI::Chat;
    if (role == U"developer")   { return Role::System; }
    else if (role == U"system")      { return Role::System; }
    else if (role == U"user")        { return Role::User; }
    else if (role == U"assistant")   { return Role::Assistant; }
    return Role::System;
}
static String role2str(const OpenAI::Chat::Role role){
    using namespace OpenAI::Chat;
    switch (role)
    {
        case Role::System: return U"developer";
        case Role::User: return U"user";
        case Role::Assistant: return U"assitant";
    }
}

void MusicalGPT4::request(const String& user_requset) {
    tell(user_requset);
    request();
}
void MusicalGPT4::request() {
    if (not debug) {
        history.model = model;
        task = OpenAI::Chat::CompleteAsync(GPT_API_KEY, history);
    } else {
        debug_is_ready = true;
    }
    // https://siv3d.github.io/ja-jp/tutorial3/openai/
}

void MusicalGPT4::tell(const String& user_requset) {
    history.messages.push_back({OpenAI::Chat::Role::User, user_requset});
}

String MusicalGPT4::get_answer(){
    if (not debug){
        if (task.getResponse().isOK()) {
            Console << U"[INFO] received answer";
            return OpenAI::Chat::GetContent(task.getAsJSON());
        } else {
            snap(task.getAsJSON().format());
            throw Error(U"[INFO | MusicalGPT4::get_answer()] LLMとの通信においてエラーが発生しました。");
        }
    } else {
        TextReader ans{U"../src/answer_a.txt"};
        debug_count++;
        return ans.readAll();
    }
}

/** LLMから返答を受け取った後、一度だけtrueを返す */
bool MusicalGPT4::is_ready(){
    if (not debug){
        return task.isReady();
    } else {
        if (debug_is_ready){
            debug_is_ready = false;
            return true;
        } else {
            return false;
        }
    }
}

// 返答を受け取った後、一度だけLLMの回答を返す。
Optional<String> MusicalGPT4::try_to_get_answer(){
    if (is_ready()) {
        const String answer = get_answer();
        history.messages.push_back({OpenAI::Chat::Role::Assistant, answer});
        snap(answer);
        return answer;
    } else {
        return none;
    }
}

String MusicalGPT4::last_user_input() {
    for (int32_t i = history.messages.size() - 1; i >= 0; i--) {
        if (history.messages[i].first == OpenAI::Chat::Role::User) { return history.messages[i].second; }
    }
    exit(1);
}


void MusicalGPT4::dump_answer() const {
    INFO("\n===================\n\tDUMP GPT4 CONVERSATION HISTORY\t\n===================\n");
    for (const auto& snapshot:history.messages){
        INFO("\"" << role2str(snapshot.first) << "\" :\n" << snapshot.second << "\n\n");
    }
}


MusicalGPT4::MusicalGPT4(FilePathView prompt_filepath)
{
    TextReader api_key_reader{U"credential/OPEN_AI_KEY.txt"};
    TextReader sysprpt{prompt_filepath};
    GPT_API_KEY = api_key_reader.readAll();
    system_prompt = sysprpt.readAll();
    history.messages.push_back({OpenAI::Chat::Role::System, system_prompt});
}

void MusicalGPT4::remember(const JSON& json) {
    assert(json[U"dialog"].isArray());
    history.messages.clear();
    iteration_indices.clear();
    for (const auto& speaking : json[U"dialog"].arrayView()) {
        assert(speaking[U"role"].isString());
        assert(speaking[U"prompt"].isString());
        
        history.messages.push_back({
            str2role(speaking[U"role"].getString()),
            speaking[U"prompt"].getString()
        });
    }

    assert(json[U"iteration_indices"].isArray());
    for (const auto& index : json[U"iteration_indices"].arrayView()) {
        assert(index.isInteger());
        iteration_indices.push_back(index.get<int32>());
    }
}

JSON MusicalGPT4::snapshot() const {
    JSON result;
    for (const auto& [role, prompt] : history.messages) {
        JSON speaking_in_json;
        Field2JSON(speaking_in_json, role2str(role));
        Field2JSON(speaking_in_json, prompt);
        result[U"dialog"].push_back(speaking_in_json);
    }
    for (const auto& index : iteration_indices) {
        result[U"iteration_indices"].push_back(index);
    }
    return result;
}