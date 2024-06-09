# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include "EditRoom.hpp"
# include "Layout.hpp"
# include "Main_test.hpp"
# include "RichButton.hpp"

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

enum ApplicationMode{
    Edit,
    Credits,
    QR_code
};

void Main()
{
    test_main();
    Window::SetTitle(U"Grazie Piano Tuner4");
	Window::Resize(1500, 1000);
    Scene::SetResizeMode(ResizeMode::Keep);
    Window::SetStyle(WindowStyle::Sizable);
    Rect window_rect{Point::Zero(), Scene::Size()};
    Scene::SetBackground(HSV{210, 0.1, 0.15});
    
    // フォント
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };    
    
	// 環境変数から API キーを取得する
    TextReader api_key_text{U"../src/OPEN_AI_KEY.txt"};
	const String API_KEY = api_key_text.readAll();
    assert(not API_KEY.empty());
    
    // レイアウト
    Rect room_rect;
    Rect menu_rect;
    Array<Rect> button_rects;
    RectSlicer layout{window_rect, RectSlicer::X_axis};
    {
        room_rect = layout.to(0.8);
        menu_rect = layout.from(0.82).to(1.0);
        RectSlicer layout_for_buttons{menu_rect, RectSlicer::Y_axis};
        {
            for (int i = 1; i < 10; i++){
                button_rects.push_back(layout_for_buttons.to(i / 10.0).stretched(-5));
            }
        }
    }
    // サブモードの表示に関する長方形
    const Rect message_area = room_rect.stretched(-30);
    const Rect message_header_area = clipped(message_area.stretched(-30), RectF{0.0, 0.0, 0.8, 0.1});
    const Rect message_contents_area = clipped(message_area.stretched(-10), RectF{0.1, 0.1, 0.8, 0.8});
                


    //ボタン
    RichButton button_credits   {U"\U000F0189", U"クレジット", button_rects[0], true, HSV{30, 0.5, 0.8}};
    RichButton button_save      {U"\U000F0193", U"保存", button_rects[1],      false, HSV{30, 0.5, 0.8}};
    RichButton button_load      {U"\U000F024B", U"ロード", button_rects[2],     true, HSV{30, 0.5, 0.8}};
    RichButton button_qr        {U"\U000F0432", U"QRコード", button_rects[3],   false,HSV{60, 0.4, 0.7}};
    RichButton button_ex1       {U"\U000F0387", U"Bright Sun", button_rects[4], true, HSV{90, 0.4, 0.7}};
    RichButton button_ex2       {U"\U000F0387", U"Cafe Serenity", button_rects[5], true, HSV{90, 0.4, 0.7}};
    RichButton button_ex3       {U"\U000F0387", U"Bright Daybreak", button_rects[6], true, HSV{90, 0.4, 0.7}};
    Array<RichButton*> buttons{
        &button_credits, &button_save, &button_qr, &button_load, &button_ex1, &button_ex2, &button_ex3
    };

    // メイン処理を担当するオブジェクト
    EditRoom edit_room{API_KEY, room_rect};
    // ステート(状態の数が少ないのでswitch-caseで管理)
    ApplicationMode mode = Edit;
    // クレジット表記のためのデータ
    String credits;
    {
        TextReader credits_reader{U"../src/credits"};
        credits = credits_reader.readAll();
        
    }
    // QRコードを表示するための動的テクスチャ
    DynamicTexture qr_code_works;
    // 紙面上でabc.js quick editorへ掲示する必要あり // https://editor.drawthedots.com

	while (System::Update())
	{
        ClearPrint();
        switch(mode){
            case Edit:
                edit_room.update();
                edit_room.render();
                break;
            case Credits:
                RoundRect{message_area, 5}.draw(ColorF{0.9}).drawFrame(3, ColorF{0.5});
                font(U"# クレジット").draw(48, Arg::topLeft = message_header_area.pos,  ColorF{0.2});
                font(credits).draw(30, message_contents_area, ColorF{0.2});
                break;
            case QR_code:
                RoundRect{message_area, 5}.draw(ColorF{0.9}).drawFrame(3, ColorF{0.5});
                // QR コードを表示するための動的テクスチャ
                font(U"# QRコード").draw(48, Arg::topLeft = message_header_area.pos,  ColorF{0.2});
                if (qr_code_works){
                    font(U"ABC記譜法で記されたプレーンテキストの楽譜を保存できます。\nABCJS quick editor(https://editor.drawthedots.com)などで再生可能").draw(20, Arg::topLeft= message_contents_area.pos, ColorF{0.2});
                    qr_code_works.scaled(message_contents_area.w /500.0 / 2, message_contents_area.w /500.0 / 2).draw(Arg::center = message_contents_area.center());
                } else {
                    font(U"楽曲をQRコードに変換できませんでした。").draw(20, Arg::topCenter = message_contents_area.center(), ColorF{0.2});
                }
                break;
        }

        // ボタンに対するハンドラ
        if (button_credits.leftClicked()){
            button_credits.selected = not button_credits.selected;
            mode = button_credits.selected ? Credits : Edit;
            if (button_credits.selected){
                edit_room.player.stop();
            }
        }
        button_save.enabled = edit_room.history.size() > 0;
        if (button_save.leftClicked()){
            const String title = edit_room.player.get_title();
            edit_room.history.save(title);
            System::MessageBoxOK(U"{}が保存されました。"_fmt(title));
        }
        button_qr.enabled = edit_room.history.size() > 0;
        if (button_qr.leftClicked()){
            button_qr.selected = not button_qr.selected;
            mode = button_qr.selected ? QR_code : Edit;
            if (button_qr.selected){
                const String abc_score = edit_room.player.get_abc_score();
                snap(abc_score);
                if (const auto qr = QR::EncodeText(abc_score)){
                    qr_code_works.fill(QR::MakeImage(qr).scaled(500, 500, InterpolationAlgorithm::Nearest));
                }
            }
        }
        if (button_load.leftClicked()){
            Optional<FilePath> path = Dialog::OpenFile({ FileFilter::JSON() }, U"/Users/AppleBird/Documents/programming/VSCodeProject/siv3d_v0.6.11_macOS/examples/GraziePianoTuner4/archive");
            if (path){
                edit_room.history.load_json(*path);
                edit_room.musicalGPT4.remember_from_snapshots(edit_room.history.see_snapshots());
            }
        }
        if (button_ex1.leftClicked()){
            edit_room.history.load_json(U"/Users/AppleBird/Documents/programming/VSCodeProject/siv3d_v0.6.11_macOS/examples/GraziePianoTuner4/archive/brightsun.json");
            edit_room.musicalGPT4.remember_from_snapshots(edit_room.history.see_snapshots());
        }
        if (button_ex2.leftClicked()){
            edit_room.history.load_json(U"/Users/AppleBird/Documents/programming/VSCodeProject/siv3d_v0.6.11_macOS/examples/GraziePianoTuner4/archive/cafe_serenity.json");
            edit_room.musicalGPT4.remember_from_snapshots(edit_room.history.see_snapshots());
        }
        if (button_ex3.leftClicked()){
            edit_room.history.load_json(U"/Users/AppleBird/Documents/programming/VSCodeProject/siv3d_v0.6.11_macOS/examples/GraziePianoTuner4/archive/bright_daybreak.json");
            edit_room.musicalGPT4.remember_from_snapshots(edit_room.history.see_snapshots());
        }

        // ボタンの描画
        for (RichButton* button_ptr:buttons){
            button_ptr->update();
        }
        for (RichButton* button_ptr:buttons){
            button_ptr->render();
        }
	}
}
