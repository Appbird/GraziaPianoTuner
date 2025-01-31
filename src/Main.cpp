# include <Siv3D.hpp> // OpenSiv3D v0.6.13
# include "EditRoom.hpp"
# include "Layout.hpp"
# include "Main_test.hpp"
# include "RichButton.hpp"
# include "MainApp.hpp"

# include "utility/notificator.hpp"

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

    Addon::Register<NotificationAddon>(U"NotificationAddon");
    NotificationAddon::SetLifeTime(3.0);
    
    // フォント
    FontAsset::Register(U"default", FontMethod::MSDF, 48, Typeface::Bold);
    FontAsset::Register(U"icon",    FontMethod::MSDF, 40, Typeface::Icon_MaterialDesign);
    FontAsset::Register(U"button",  40, FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"ロゴたいぷゴシック.otf");
    MainApp main_app{};
	while (System::Update())
	{
        ClearPrint();
        main_app.Update();
        main_app.Draw();
	}
}
