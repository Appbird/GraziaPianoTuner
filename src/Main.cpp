# include <Siv3D.hpp> // OpenSiv3D v0.6.13
# include "EditRoom.hpp"
# include "Layout.hpp"
# include "Main_test.hpp"
# include "RichButton.hpp"
# include "MainApp.hpp"

/**
 * 1) 来た人に打ち込んでもらう
 * 2) パワポで生成物を図る。
 * 3) オフセット
 */

// ロゴたいぷゴシックなどの著作権表示

/* 前提知識
    MIDIの時間の表現には三種類ある
    - beats (quater note)
    - ticks
    - seconds
    これらを用途に合わせてうまく使い分けてプログラミングしていかなければならない。
*/ 


void Main()
{
    test_main();
    Window::SetTitle(U"Grazie Piano Tuner4");
	Window::Resize(1500, 1000);
    Scene::SetResizeMode(ResizeMode::Keep);
    Window::SetStyle(WindowStyle::Sizable);
    Scene::SetBackground(HSV{210, 0.1, 0.15});
    
    // フォント
    FontAsset::Register(U"default", 40, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"Avenir.ttc");
    FontAsset::Register(U"icon",    FontMethod::MSDF, 40, Typeface::Icon_MaterialDesign);
    FontAsset::Register(U"button",  40, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"Avenir.ttc");
    
	// 環境変数から API キーを取得する
    TextReader api_key_text{U"../src/credential/OPEN_AI_KEY.txt"};
	const String API_KEY = api_key_text.readAll();
    assert(not API_KEY.empty());
    MainApp main_app{API_KEY};

	while (System::Update())
	{
        ClearPrint();
        main_app.Update();
        main_app.Draw();
	}
}
