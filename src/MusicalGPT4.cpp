# include "MusicalGPT4.hpp"
# include "util.hpp"
# include "EmotionalController.hpp"

void MusicalGPT4::request(const String& user_requset) {
    tell(user_requset);

    if (not debug) {
        snap(model);
        task = OpenAI::Chat::CompleteAsync(GPT_API_KEY, history, model);
    } else {
        debug_is_ready = true;
    }
    // https://siv3d.github.io/ja-jp/tutorial3/openai/
}
void MusicalGPT4::request() {
    if (not debug) {
        task = OpenAI::Chat::CompleteAsync(GPT_API_KEY, history, model);
    } else {
        debug_is_ready = true;
    }
    // https://siv3d.github.io/ja-jp/tutorial3/openai/
}

void MusicalGPT4::tell(const String& user_requset) {
    history.push_back({U"user", user_requset});
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
        history.push_back({U"assistant", answer});
        snap(answer);
        return answer;
    } else {
        return none;
    }
}

String MusicalGPT4::last_user_input() {
    for (int32_t i = history.size() - 1; i >= 0; i--) {
        snap(history[i].first);
        if (history[i].first == U"user") { return history[i].second; }
    }
    exit(1);
}


void MusicalGPT4::dump_answer() const {
    INFO("\n===================\n\tDUMP GPT4 CONVERSATION HISTORY\t\n===================\n");
    for (const auto& snapshot:history){
        INFO("\"" << snapshot.first << "\" :\n" << snapshot.second << "\n\n");
    }
}


MusicalGPT4::MusicalGPT4(FilePathView prompt_filepath)
{
    TextReader api_key_reader{U"credential/OPEN_AI_KEY.txt"};
    TextReader sysprpt{prompt_filepath};
    GPT_API_KEY = api_key_reader.readAll();
    system_prompt = sysprpt.readAll();
    history.push_back({U"developer", system_prompt});
}