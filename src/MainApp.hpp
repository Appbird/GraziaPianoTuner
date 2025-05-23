#pragma once
# include <Siv3D.hpp>

# include "EditRoom.hpp"
# include "SideMenuButtons.hpp"

class MainApp {
    enum ApplicationMode{
        Edit,
        Credits,
        QR_code
    };


    // レイアウト
    Rect room_rect;
    Rect menu_rect;
    SideMenuButtons sidemenu_buttons;

    // サブモードの表示に関する長方形
    Rect message_area;
    Rect message_header_area;
    Rect message_contents_area;
    
    // メイン処理を担当するオブジェクト
    EditRoom edit_room;
    // ステート(状態の数が少ないのでswitch-caseで管理)
    ApplicationMode mode = Edit;
    // クレジット表記のためのデータ
    String credits;
    // QRコードを表示するための動的テクスチャ
    DynamicTexture qr_code_works;

    

public:
    MainApp() {
        Init();
    }

    void Update() {
        switch(mode){
            case Edit:      edit_room.update(); break;
            case Credits:   break;
            case QR_code:   break;
            default:        assert(0); break;
        }
        handle_button();
        // ボタンの描画
        for (auto& [str, button_ptr]:sidemenu_buttons.buttons){
            button_ptr.update();
        }
    }

    void Draw() {
        switch(mode){
            case Edit:      edit_room.render(); break;
            case Credits:   draw_credits(); break;
            case QR_code:   draw_qr(); break;
            default:        assert(0); break;
        }
        for (auto& [str, button_ptr]:sidemenu_buttons.buttons){
            button_ptr.render();
        }
    }
private:
    void Init() {
        Rect window_rect{Point::Zero(), Scene::Size()};
        RectSlicer layout{window_rect, RectSlicer::X_axis};
        {
            room_rect = layout.to(0.8);
            menu_rect = layout.from(0.82).to(1.0);
            sidemenu_buttons = {menu_rect};
        }

        // サブモードの表示に関する長方形
        message_area = room_rect.stretched(-30);
        message_header_area = clipped(message_area.stretched(-30), RectF{0.0, 0.0, 0.8, 0.1});
        message_contents_area = clipped(message_area.stretched(-10), RectF{0.1, 0.1, 0.8, 0.8});
        
        edit_room = {room_rect};
        mode = Edit;
        {
            TextReader credits_reader{U"../src/credits"};
            credits = credits_reader.readAll();
        }

        edit_room.starting_time_session = DateTime::Now();
    }

    void handle_button() {
        RichButton& button_credits  = sidemenu_buttons.ref(U"credits");
        RichButton& button_save     = sidemenu_buttons.ref(U"save");
        RichButton& button_qr       = sidemenu_buttons.ref(U"qr");
        RichButton& button_load     = sidemenu_buttons.ref(U"load");
        RichButton& button_reset    = sidemenu_buttons.ref(U"reset");
        RichButton& button_ex1      = sidemenu_buttons.ref(U"ex1");
        RichButton& button_ex2      = sidemenu_buttons.ref(U"ex2");
        RichButton& button_ex3      = sidemenu_buttons.ref(U"ex3");

        button_save.enabled = edit_room.history.size() > 0;
        button_qr.enabled = edit_room.history.size() > 0;

        if (button_credits.leftReleased()){
            button_credits.flip_selected();
            mode = button_credits.selected ? Credits : Edit;
            if (button_credits.selected){ edit_room.player.stop(); }
        }
        if (button_qr.leftReleased()){
            button_qr.flip_selected();
            mode = button_qr.selected ? QR_code : Edit;
            if (button_qr.selected){ prepare_QR(); }
        }
        if (button_save.leftReleased())     { save_score(); }
        if (button_load.leftReleased())     { load_score_from_dir(); }
        if (button_reset.leftReleased())    { reset_history(); }
        if (button_ex1.leftReleased())      { load_score(U"archive/brightsun.json"); }
        if (button_ex2.leftReleased())      { load_score(U"archive/cafe_serenity.json"); }
        if (button_ex3.leftReleased())      { load_score(U"archive/bright_daybreak.json"); }
    }

    void prepare_QR() {
        const String abc_score = edit_room.player.get_abc_score();
        if (const auto qr = QR::EncodeText(abc_score)){
            qr_code_works.fill(QR::MakeImage(qr).scaled(500, 500, InterpolationAlgorithm::Nearest));
        }
    }
    void load_score(const FilePath& fp) { edit_room.load_history(fp); }
    void load_score_from_dir() {
        Optional<FilePath> path = Dialog::OpenFile({ FileFilter::JSON() }, U"archive");
        if (path){ load_score(*path); }
    }
    void save_score() {
        edit_room.save_history();
        System::MessageBoxOK(U"作業履歴が保存されました。");
    }
    void reset_history() {
        const MessageBoxResult result = System::MessageBoxOKCancel(U"ここまでの作業履歴を全てリセットします。こうかいしませんね？");
        if (result == MessageBoxResult::OK) { edit_room.reset(); }
    }

    void draw_credits() const {
        RoundRect{message_area, 5}.draw(ColorF{0.9}).drawFrame(3, ColorF{0.5});
        FontAsset(U"default")(U"# クレジット").draw(48, Arg::topLeft = message_header_area.pos,  ColorF{0.2});
        FontAsset(U"default")(credits).draw(30, message_contents_area, ColorF{0.2});
    }
    
    void draw_qr() const {
        RoundRect{message_area, 5}.draw(ColorF{0.9}).drawFrame(3, ColorF{0.5});
        // QR コードを表示するための動的テクスチャ
        FontAsset(U"default")(U"# QRコード").draw(48, Arg::topLeft = message_header_area.pos,  ColorF{0.2});
        if (qr_code_works){
            FontAsset(U"default")
                (
                    U"ABC記譜法で記されたプレーンテキストの楽譜を保存できます。\n"
                    "ABCJS quick editor(https://editor.drawthedots.com)などで再生可能"
                ).draw(20, Arg::topLeft= message_contents_area.pos, ColorF{0.2});
            
            qr_code_works.scaled(
                message_contents_area.w / 500.0 / 2,
                message_contents_area.w / 500.0 / 2
            ).draw(Arg::center = message_contents_area.center());
        } else {
            FontAsset(U"default")(U"楽曲をQRコードに変換できませんでした。").draw(20, Arg::topCenter = message_contents_area.center(), ColorF{0.2});
        }
    }

};