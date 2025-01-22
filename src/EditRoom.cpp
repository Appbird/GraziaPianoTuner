# include "EditRoom.hpp"
# include "EmotionalController.hpp"
# include "HarmonicGuide.hpp"

void EditRoom::set_rect(const Rect& rect){
    all_area = rect;
    RectSlicer window_layout{all_area, RectSlicer::X_axis};
    {
        auto window_layouts = window_layout.devide_at(0.7, RectSlicer::Y_axis);
        {
            RectSlicer& window_music_layout = window_layouts.first;
            composed_area           = window_music_layout.to(0.7).stretched(-10);
            {
                RectSlicer input_layout = window_music_layout.slice(0.76, RectSlicer::X_axis);
                input_textbox_area = input_layout.to(0.7).stretched(-10);
                input_button_area  = input_layout.to(1.0).stretched(-10);
            }
            quantitative_controll_area = window_music_layout.to(0.95).stretched(-10);
            page_flipper_area       = window_music_layout.to(1.0).stretched(-10);
        }
        {
            RectSlicer& window_GPT_layout = window_layouts.second;
            menu_area       = window_GPT_layout.to(0.08).stretched(-10);
            GPT_answer_guide_area = window_GPT_layout.to(0.13).stretched(-10);
            GPT_answer_guide_font = Font{int(GPT_answer_guide_area.h * 0.6), font_smart_path};
            GPT_answer_area = window_GPT_layout.to(0.95).stretched(-10);
        }
    }
}

void EditRoom::update(){
    // 入力と状態アップデート
    player.update();
    musicalGPT4.update();
    composed_viewer.update(player, not user_request_text_state.active);
    //#DONE 多様性
    controller_panel->update();
    history.update();
    // GPT4からanswerを得られた時だけ更新。
    if (const auto ans = musicalGPT4.try_to_get_answer()){
        // #FIXME この実装は以下の（強い）仮定に基づいている。
        // | i) この地点で最後に入力したuser_inputは、ansとちょうど対応している。
        // |    つまり、musicalGPT4.request()を実効から、このコードの箇所に到達するまでに
        // |    musicalGPT4.request()をさらにもう一度呼び出していない。
        // |
        // | ii) このコードに入るまでに、必ず一度はmusicalGPT4.request()を呼び出している。
        // | 
        // | 将来的な実装の変更によってはこれは成り立たない可能性が高い。
        // | この実装を修正するには、MusicalGPT-4の実装を根本的に変える必要があり、手間がかかる。
        // |
        set_GPT_answer(musicalGPT4.last_user_input(), *ans);
    }
    // 履歴からの復元
    if (history.is_page_refreshed()){
        const auto& snapshot = history.pick_snapshot();
        restore(snapshot);
    }
    // デバッグ
    if (KeyUnderscore_JIS.down()){ musicalGPT4.dump_answer(); }
    // #TODO リファクタリング
    if (menu_area.leftClicked()){
        if (musicalGPT4.model == OpenAI::Model::GPT4){
            musicalGPT4.model = OpenAI::Model::GPT4_32K;
        } else if (musicalGPT4.model == OpenAI::Model::GPT4_32K){
            musicalGPT4.model = U"gpt-4o";
        } else if (musicalGPT4.model == U"gpt-4o") {
            musicalGPT4.model = OpenAI::Model::GPT4;
        }
    }
}

void EditRoom::render(){
    // GUI
    // テキストボックスを表示する
    GPT_answer_guide_font(U"GPT回答本文").drawBase(GPT_answer_guide_area.bl(), ColorF{0.7});
    SimpleGUI::TextArea(user_request_text_state, input_textbox_area.pos, input_textbox_area.size, 800, editable);
    SimpleGUI::TextArea(GPT_answer_text_state, GPT_answer_area.pos, GPT_answer_area.size, 5000, false);
    // ボタン表示
    bool button_enable = (
        not user_request_text_state.text.isEmpty()
        or (controller_panel and history.size() > 0)
    ) and editable;
    if (SimpleGUI::Button(U"\U000F1C4D 送信", input_button_area.pos, input_button_area.w, button_enable))
    { 
        const String input = user_request_text_state.text;
        //#DONE 多様性
        musicalGPT4.request(input, controller_panel->describe());
    }
    RoundRect{menu_area, 5}.drawFrame(5, ColorF{0.6});
    GPT_answer_guide_font(musicalGPT4.model).draw(menu_area.h * 0.5, Arg::center = menu_area.center(), ColorF{0.6});
    if (menu_area.mouseOver()) {RoundRect{menu_area, 5}.draw(ColorF{0.6, 0.5});}
    // 描画
    RoundRect{composed_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    RoundRect{quantitative_controll_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    composed_viewer.render(player);
    assert(controller_panel);
    controller_panel->render();
    history.render();
    if (musicalGPT4.is_downloading()){
        Scene::Rect().draw(ColorF{0, 0.4});
        Circle{ Scene::Center(), 50 }.drawArc((Scene::Time() * 120_deg), 300_deg, 4, 4);
    }
}

void EditRoom::reset(){
    musicalGPT4 = MusicalGPT4{GPT_API_KEY};
    player = Composed{};
    composed_viewer = ComposedViewer{};
    switch_panel(ParameterControllMode::HarmonicGuide);
    history = HistoryViewer{};
    composed_viewer.set_renderer_area(composed_area);
    //#DONE 多様性
    controller_panel->set_render_area(quantitative_controll_area);
    
    history.set_render_area(page_flipper_area);
    user_request_text_state = TextAreaEditState{U""};
    GPT_answer_text_state   = TextAreaEditState{U""};

    starting_time_session = DateTime::Now();
}

void EditRoom::switch_panel(ParameterControllMode next_mode) {
    mode = next_mode;
    switch (mode) {
        case ParameterControllMode::EmotionalController: 
            controller_panel = std::make_shared<EmotionalController>();
            return;
        case ParameterControllMode::HarmonicGuide: 
            controller_panel = std::make_shared<HarmonicGuide>();
            return;
    }
}

void EditRoom::restore(const HistoryViewer::Snapshot& snapshot){
    player.set_answer(snapshot.answer_from_LLM);
    //#DONE 多様性
    switch_panel(snapshot.params_type);
    user_request_text_state = TextAreaEditState{snapshot.request};
    GPT_answer_text_state   = TextAreaEditState{snapshot.answer_from_LLM};
    editable = (history.current_page == history.size() - 1);
    controller_panel->set_state(editable);
}
void EditRoom::set_GPT_answer(const String& user_to_LLM, const String& answer){
    player.set_answer(answer);
    
    composed_viewer.reset_scroll();
    GPT_answer_text_state = TextAreaEditState{answer};
    //#DONE 多様性
    history.remember(HistoryViewer::Snapshot{
        user_request_text_state.text,
        user_to_LLM,
        answer,
        mode,
        controller_panel->snapshot()
    });
    history.save(player.get_title(), starting_time_session);
}

void EditRoom::load_history(const FilePath& fp) {
    history.load_json(fp);
    musicalGPT4.remember_from_snapshots(history.see_snapshots());
}
void EditRoom::save_history() {
    const String title = player.get_title();
    history.save(title, starting_time_session);
}