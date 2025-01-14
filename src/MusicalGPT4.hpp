# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "util.hpp"
# include "HistoryViewer.hpp"
# include "Composed.hpp"

class EmotionalController;

class MusicalGPT4{
    public:
        StringView model = U"gpt-4o";
    private:
        String GPT_API_KEY;
        String system_prompt;
        String system_prompt_tlanslate;
        bool debug = false;
            bool debug_is_ready = false;
            int debug_count = 0;
        Array<std::pair<String, String>> history;
        AsyncHTTPTask task_for_composing;
        AsyncHTTPTask task_for_translating;
        String construct_prompt(const String& user_request, const EmotionalController& emotional_controller);
        String construct_prompt(const String& user_request, const EmotionalController::EmotionalParameters& emotional_controller);
        bool is_ready();
        String get_answer();
        
    public:
        MusicalGPT4(){}
        MusicalGPT4(String arg_GPT_API_KEY):
            GPT_API_KEY(arg_GPT_API_KEY)
        {
            TextReader sysprpt{U"../src/system_prompt1.txt"};
            TextReader sysprpt_revise{U"../src/system_prompt_for_translate.txt"};
            system_prompt = sysprpt.readAll();
            system_prompt_tlanslate = sysprpt_revise.readAll();
            history.push_back({U"user", system_prompt});
        }
        
        /** LLMにuser_request, emotional_controllerの内容に基づいて、楽曲を新たに記述するようリクエストする。
         * 出力の受け取りは非同期で行う必要がある。try_to_get_answer()を定期的に呼び出して、LLMからの応答が返ってきているかをチェックする必要がある。
         * TODO: これをstd::futureを使って書き換える。
         */
        void request(const String& user_requset, const EmotionalController& emotional_controller);
        /** @brief request関数でリクエストをLLMに送った後、返答があった場合は、対話履歴にその返答を加えながら返す。返答がなかった場合はnoneを返す。 */
        Optional<String> try_to_get_answer(){
            if (is_ready()) {
                const String answer = get_answer();
                history.push_back({U"assistant", answer});
                snap(answer);
                return answer;
            } else {
                return none;
            }
        }
        Optional<String> try_to_get_japanese_answer(){
            if (is_ready()) {
                const String answer = get_answer();
                history.push_back({U"assistant", answer});
                snap(answer);
                return answer;
            } else {
                return none;
            }
        }
        bool is_downloading(){
            return task_for_composing.isDownloading() or task_for_translating.isDownloading();
        }
        HTTPProgress progress(){
            return task_for_composing.getProgress();
        }
        void dump_answer() const{
            INFO("\n===================\n\tDUMP GPT4 CONVERSATION HISTORY\t\n===================\n");
            for (const auto& snapshot:history){
                INFO("\"" << snapshot.first << "\" :\n" << snapshot.second << "\n\n");
            }
        }
        void remember_from_snapshots(const Array<HistoryViewer::Snapshot>& snapshots){
            history.clear();
            history.push_back({ U"system", system_prompt });
            for (const auto& snapshot:snapshots){
                history.push_back({ U"user", construct_prompt(snapshot.request, snapshot.params) });
                history.push_back({ U"assistant", snapshot.answer });
            }
            
        }
        void update(){
            // #TODO リファクタリング
            // 基礎の答えが得られたら、修正タスクをOpenAI APIに送信する。
            if (task_for_composing.isReady() and task_for_composing.getResponse().isOK()){
                const String base_answer = OpenAI::Chat::GetContent(task_for_composing.getAsJSON());
                snap(base_answer);
                task_for_translating = OpenAI::Chat::CompleteAsync(GPT_API_KEY, 
                    {
                        { U"system", system_prompt_tlanslate },
                        { U"user", base_answer }
                    }
                , model);
            }
        }
        
        
};