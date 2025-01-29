# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "util.hpp"
# include "HistoryViewer.hpp"
# include "Composed.hpp"
# include "Interface/PanelComponents.hpp"

class EmotionalController;

class MusicalGPT4{
    public:
        StringView model = U"gpt-4o";
    private:
        String GPT_API_KEY;
        String system_prompt;
        String system_prompt_tlanslate;
        bool debug = false;
            int debug_count = 0;
        Array<std::pair<String, String>> history;
        AsyncHTTPTask task_for_composing;
        //#DONE 多様性
        String construct_prompt(const String& user_request, const String& params_description);
        bool is_ready();
        String get_answer();
        
    public:
        MusicalGPT4(){}
        MusicalGPT4(String arg_GPT_API_KEY):
            GPT_API_KEY(arg_GPT_API_KEY)
        {
            TextReader sysprpt{U"../src/system_prompt1.txt"};
            system_prompt = sysprpt.readAll();
            system_prompt_tlanslate = sysprpt_revise.readAll();
            history.push_back({U"developer", system_prompt});
        }
        
        /** LLMにuser_request, emotional_controllerの内容に基づいて、楽曲を新たに記述するようリクエストする。
         * 出力の受け取りは非同期で行う必要がある。try_to_get_answer()を定期的に呼び出して、LLMからの応答が返ってきているかをチェックする必要がある。
         * TODO: これをstd::futureを使って書き換える。
         */
        void request(const String& user_requset, const String& params_description);
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
        /** この関数を呼ぶ前に、ユーザ入力が履歴に一つ以上存在することを要求する。 */
        String last_user_input() {
            for (int32_t i = history.size() - 1; i >= 0; i--) {
                snap(history[i].first);
                if (history[i].first == U"user") { return history[i].second; }
            }
            exit(1);
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
            history.push_back({ U"developer", system_prompt });
            for (const auto& snapshot:snapshots){
                //#DONE emotionalとguideのどちらの場合にも対応すること
                history.push_back({ U"user", snapshot.user_to_LLM });
                history.push_back({ U"assistant", snapshot.answer_from_LLM });
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
                        { U"developer", system_prompt_tlanslate },
                        { U"user", base_answer }
                    }
                , model);
            }
        }
        
        
};
