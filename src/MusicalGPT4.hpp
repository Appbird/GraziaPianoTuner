# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "util.hpp"
# include "HistoryViewer.hpp"
# include "Composed.hpp"
# include "Interface/PanelComponents.hpp"

class EmotionalController;

class MusicalGPT4{
    public:
        String model;
    private:
        String GPT_API_KEY;
        String system_prompt;
        bool debug = false;
            bool debug_is_ready = false;
            int debug_count = 0;
        OpenAI::Chat::Request history;
        Array<int32> iteration_indices;
        AsyncHTTPTask task;
        bool is_ready();
        String get_answer();
        
    public:
        MusicalGPT4(StringView model_name, FilePathView prompt_filepath);
		void set_model(StringView model_name);
        
        /** LLMにuser_requestの内容に基づいて、楽曲を新たに記述するようリクエストする。
         * 出力の受け取りは非同期で行う必要がある。try_to_get_answer()を定期的に呼び出して、LLMからの応答が返ってきているかをチェックする必要がある。
         */
        void request();
        void request(const String& user_requset);

        /** LLMとの対話履歴にuser_requestの内容を挿入する。この関数を実行しても、request関数とは異なりLLMとの通信は行わない。 */
        void tell(const String& user_requset);
        /** @brief request関数でリクエストをLLMに送った後、返答があった場合は、対話履歴にその返答を加えながら、その回答を一度だけ返す。返答がなかった、返答をもうすでに一度返した場合はnoneを返す。 */
        Optional<String> try_to_get_answer();
        /** この関数を呼ぶ前に、ユーザ入力が履歴に一つ以上存在することを要求する。 */
        String last_user_input();
        void dump_answer() const;

        
        bool is_downloading()   { return task.isDownloading(); }
        HTTPProgress progress() { return task.getProgress(); }
        void update()           {  }

        void remember(const JSON& json);

        JSON snapshot() const;

};
