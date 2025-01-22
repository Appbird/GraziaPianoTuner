# pragma once
# include <Siv3D.hpp>
# include "Layout.hpp"
# include "MusicalGPT4.hpp"
# include "util.hpp"
# include "HistoryViewer.hpp"
# include "Interface/PanelComponents.hpp"


class EditRoom{
public:
    MusicalGPT4 musicalGPT4;
    Composed player;
    ComposedViewer composed_viewer;
    //#DONE 多様性
    std::shared_ptr<ParameterControllerPanel> controller_panel;
    HistoryViewer history;
    // このセッション（編集作業）を開始した時刻。アプリケーションを開始した時、リセットボタンを押した時に、その時刻にセットされる。
    DateTime starting_time_session;
private:
    String GPT_API_KEY;
	// 回答を格納する変数
    bool editable = true;
    ParameterControllMode mode = ParameterControllMode::HarmonicGuide;
    
    // テキストボックスの中身
	TextAreaEditState user_request_text_state;
    TextAreaEditState GPT_answer_text_state;

    // レイアウト
    FilePath font_smart_path;
    Rect all_area;
    Rect composed_area;
        Rect input_textbox_area;
        Rect input_button_area;
        Rect quantitative_controll_area;
    Rect page_flipper_area;
    Rect GPT_answer_guide_area;
    Font GPT_answer_guide_font;
    Rect GPT_answer_area;
    Rect menu_area;

    void set_rect(const Rect& rect);
    // 指定したモードに切り替える
    void switch_panel(ParameterControllMode next_mode);
    // 与えられたSnapshotに基づいて状態を復元する。
    void restore(const HistoryViewer::Snapshot& snapshot);
    // GPTからの返答を解釈する
    void set_GPT_answer(const String& user_to_LLM, const String& answer);
    
public:
    EditRoom(){}
    EditRoom(const String& _GPT_API_KEY, const Rect& area):
        musicalGPT4{_GPT_API_KEY},
        GPT_API_KEY(_GPT_API_KEY),
        font_smart_path(FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"03スマートフォントUI.otf")
    {
        set_rect(area);
        reset();
    }
    void update();
    void render();
    void reset();
    // JSONファイルから履歴を復元する。
    void load_history(const FilePath& fp);
    // JSONファイルに今までの履歴を復元する。
    void save_history();
};