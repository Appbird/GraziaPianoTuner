# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "Composed.hpp"


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
        String GPT_API_AKY;
        String construct_prompt(String request){
            return request;
        }
    public:
        MusicalGPT4(String arg_GPT_API_AKY):
            GPT_API_AKY(arg_GPT_API_AKY)
        {}
        Composed request(String requset){
            // const String prompt = construct_prompt(request);
            // OpenAI::Chat::Complete(API_KEY, prompt);
            return Composed(U"生成しました。次のとおりです。\n```\nX:3\nT: false score \nM:4/4\nL:1/4\nK:D\nQ:1/4 = 225\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"Aade|\"Em7\"fz3|\"A7\"z4|\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"A2\"F#sus4\"c2|\"Dmaj\"d4|z4|\n```\nOUTPUT\n```\nX:3\nT: Browned Bread\nM:4/4\nL:1/4\nK:D\nQ:1/4 = 225\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"Aade|\"Em7\"fz3|\"A7\"z4|\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"A2\"F#sus4\"c2|\"Dmaj\"d4|z4|\n```");
        }
};

void Main()
{
	Scene::SetBackground(HSV{210, 0.4, 0.2});

	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	// 環境変数から API キーを取得する
	const String API_KEY = U"SECRET_KEY"; // EnvironmentVariable::Get(U"MY_OPENAI_API_KEY");
    MusicalGPT4 musicalGPT4(API_KEY);

	// テキストボックスの中身
	TextEditState textEditState;

	// 回答を格納する変数
	Composed answer;

	while (System::Update())
	{
        ClearPrint();
		// テキストボックスを表示する
		SimpleGUI::TextBox(textEditState, Vec2{ 40, 40 }, 600);

		if (SimpleGUI::Button(U"送信", Vec2{ 660, 40 }, 80, (not textEditState.text.isEmpty()))){ 
			const String input = textEditState.text;
			answer = musicalGPT4.request(U"create a shiny piano music.");
            answer.play();
        }
        answer.update();
        answer.render();
	}
}
