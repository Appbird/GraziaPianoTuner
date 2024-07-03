# pragma once
# include <Siv3D.hpp>
# include "Layout.hpp"
# include "MusicalGPT4.hpp"
# include "Layout.hpp"
# include "EmotionalController.hpp"
# include "HistoryViewer.hpp"

class EditRoom{
public:
    MusicalGPT4 musicalGPT4;
    Composed player;
    ComposedViewer composed_viewer;
    EmotionalController emotional_controller;
    HistoryViewer history;
private:
	// 回答を格納する変数
    bool editable = true;
    
    // テキストボックスの中身
	TextAreaEditState user_request_text_state;
    TextAreaEditState GPT_answer_text_state;

    // レイアウト
    FilePath font_smart_path;
    Rect all_area;
    Rect composed_area;
        Rect input_textbox_area;
        Rect input_button_area;
        Rect emotional_controll_area;
    Rect page_flipper_area;
    Rect GPT_answer_guide_area; Font GPT_answer_guide_font;
    Rect GPT_answer_area;
    Rect menu_area;
    

    void set_rect(const Rect& rect);

    void restore(const HistoryViewer::Snapshot& snapshot){
        player.set_answer(snapshot.answer);
        emotional_controller.memento(snapshot.params);
        user_request_text_state = TextAreaEditState{snapshot.request};
        GPT_answer_text_state   = TextAreaEditState{snapshot.answer};
        editable = (history.current_page == history.size() - 1);
        emotional_controller.set_state(editable);
    }
    void set_GPT_answer(const String& answer){
        player.set_answer(answer);
        composed_viewer.reset_scroll();
        GPT_answer_text_state = TextAreaEditState{answer};
        history.remember(HistoryViewer::Snapshot{
            user_request_text_state.text,
            GPT_answer_text_state.text,
            emotional_controller.snapshot()
        });
    }
    
public:
    EditRoom(){}
    EditRoom(const String& GPT_API_KEY, const Rect& area):
        musicalGPT4{GPT_API_KEY},
        font_smart_path(FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"03スマートフォントUI.otf")
    {
        set_rect(area);
        composed_viewer.set_renderer_area(composed_area);
        emotional_controller.set_render_area(emotional_controll_area);
        history.set_render_area(page_flipper_area);
    }
    void update();
    void render();

};