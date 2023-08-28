# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Composed.hpp"
# include "Main_test.hpp"
/**
 * 1) 来た人に打ち込んでもらう
 * 2) パワポで生成物を図る。
 * 3) オフセット
 */

/* 前提知識
    MIDIの時間の表現には三種類ある
    - beats (quater note)
    - ticks
    - seconds
    これらを用途に合わせてうまく使い分けてプログラミングしていかなければならない。
*/ 


/// ------------------------------
///  DEBUG
/// ------------------------------

std::ostream& operator<<(std::ostream& os, Rect r){
    return os << "(" << r.x << r.y << ") ~ (" << r.br().x << ", " << r.br().y << ")";
}

/// ------------------------------
///  Implementation
/// ------------------------------
class MusicalGPT4{
    private:
        String GPT_API_KEY;
        String system_prompt;
        bool debug = true;
        bool debug_is_ready = false;
        int debug_count = 0;
        String answer;
        String construct_prompt(String request){
            return request;
        }
        void request_tmp(const String& user_requset){
            return;
        }
        void update_tmp(Composed& composed){
            if (debug_is_ready) {
                TextReader ans{(debug_count == 0) ? U"../src/answer_a.txt" : U"../src/answer_b.txt"};
                debug_count ++;
                composed = Composed(ans.readAll());
                debug_is_ready = false;
            }
        }
        AsyncHTTPTask task;
    public:
        MusicalGPT4(String arg_GPT_API_KEY):
            GPT_API_KEY(arg_GPT_API_KEY)
        {
            TextReader sysprpt{U"../src/system_prompt1.txt"};
            system_prompt = sysprpt.readAll();
        }
        void request(const String& user_requset)
        {
            debug_is_ready = true;
            if (debug) { request_tmp(user_requset); return; }
            const String prompt = construct_prompt(user_requset);
            task = OpenAI::Chat::CompleteAsync(GPT_API_KEY, {
                { U"system", system_prompt  },
                { U"user",  user_requset    }
            });
            // https://siv3d.github.io/ja-jp/tutorial3/openai/
        }
        const String& get_answer(){
            return answer;
        }
        void update(Composed& composed){
            if (debug_is_ready and debug) { update_tmp(composed); return; }
            if (not task.isReady()) { return; }
            if (task.getResponse().isOK())
            {
                answer = OpenAI::Chat::GetContent(task.getAsJSON());
                snap(answer);
                composed = Composed{answer};
            }
            else
            {
                std::cerr << "エラーが発生しました。" << std::endl;
            }
        }
        bool is_proceeding(){
            return task.isDownloading();
        }
};


void Main()
{
    test_main();
	Window::Resize(1000, 1000);
    Scene::SetBackground(HSV{210, 0.4, 0.2});
    

	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	// 環境変数から API キーを取得する
    TextReader api_key_text{U"../src/OPEN_AI_KEY.txt"};
	const String API_KEY = api_key_text.readAll();
    assert(not API_KEY.empty());
    
    MusicalGPT4 musicalGPT4(API_KEY);

	// テキストボックスの中身
	TextEditState textEditState;
    
	// 回答を格納する変数
	Composed player;
    ComposedViewer composed_viewer{Rect{Point::Zero(), Scene::Size().x, int(Scene::Size().y * 0.8)}};

	while (System::Update())
	{
        ClearPrint();
		// テキストボックスを表示する
		SimpleGUI::TextBox(textEditState, Vec2{ 40, 525 }, 600);
		if (SimpleGUI::Button(U"送信", Vec2{ 660, 525 }, 80, (not textEditState.text.isEmpty())))
        { 
			const String input = textEditState.text;
			musicalGPT4.request(U"create a shiny piano music.");
        }
        musicalGPT4.update(player);
        
        player.update();
        composed_viewer.update(player);
        composed_viewer.render(player);
        if (musicalGPT4.is_proceeding()){
            Circle{ Scene::Center(), 50 }.drawArc((Scene::Time() * 120_deg), 300_deg, 4, 4);
        }
	}
}
