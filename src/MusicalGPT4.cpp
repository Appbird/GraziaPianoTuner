# include "MusicalGPT4.hpp"
# include "util.hpp"
# include "EmotionalController.hpp"

String MusicalGPT4::construct_prompt(const String& user_request, const String& params_description){
    return 
        U"# INPUT\n{}\n{}"_fmt(user_request, params_description);
}

void MusicalGPT4::request(const String& user_requset, const String& params_description) {
    Console << U"[INFO] Start Request.";
    history.push_back({U"user", construct_prompt(user_requset, params_description)});
    snap(construct_prompt(user_requset, params_description));

    if (not debug) {
        task_for_composing = OpenAI::Chat::CompleteAsync(GPT_API_KEY, history, model);
        task_for_translating = AsyncHTTPTask{};
    } else {
        debug_is_ready = true;
    }
    // https://siv3d.github.io/ja-jp/tutorial3/openai/
}

String MusicalGPT4::get_answer(){
    if (not debug){
        if (task_for_composing.getResponse().isOK() and task_for_translating.getResponse().isOK())
        {
            Console << U"[INFO] received answer";
            return 
                OpenAI::Chat::GetContent(task_for_translating.getAsJSON());
        }
        else
        {
            std::cerr << "エラーが発生しました。" << std::endl;
            return String{};
        }
    } else {
        //#TODO リファクタリング
        TextReader ans{(debug_count == 0) ? U"../src/answer_a.txt" : U"../src/answer_b.txt"};
        debug_count++;
        return ans.readAll();
    }
}

bool MusicalGPT4::is_ready(){
    if (not debug){
        return task_for_translating.isReady();
    } else {
        if (debug_is_ready){
            debug_is_ready = false;
            return true;
        } else {
            return false;
        }
    }
}