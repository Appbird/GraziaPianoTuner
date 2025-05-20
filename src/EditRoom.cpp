# include "EditRoom.hpp"
# include "EmotionalController.hpp"
# include "HarmonicGuide.hpp"

# include "utility/notificator.hpp"

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
            menu_area               = window_GPT_layout.to(0.08).stretched(-10);
            GPT_answer_guide_area   = window_GPT_layout.to(0.13).stretched(-10);
            GPT_answer_guide_font   = Font{int(GPT_answer_guide_area.h * 0.6), font_smart_path};
            GPT_answer_area         = window_GPT_layout.to(0.95).stretched(-10);
        }
    }
}

void EditRoom::update(){
    // 入力と状態アップデート
    player.update();
    composed_viewer.update(player, not user_request_text_state.active);
    assert(controller_panel);
    controller_panel->update();
    history.update();
    // #FIXME downcastを利用した方法はできるだけ避けたい。
    // | 別の方法で、これらの情報をcontroller_panelに伝達したいところ。
    std::shared_ptr<HarmonicGuide> harmonic_guide = std::dynamic_pointer_cast<HarmonicGuide>(controller_panel);
    if (harmonic_guide == controller_panel) {
        harmonic_guide->left_bar    = composed_viewer.left_bar();
        harmonic_guide->right_bar   = composed_viewer.right_bar();
    }

    // GPT4からanswerを得られた時だけ更新。
    if (const auto ans = composers.result()){
        try {
            set_GPT_answer(*ans);
            NotificationAddon::Show(U"楽曲を生成しました。", NotificationAddon::Type::Success);
        } catch (Error e) {
            NotificationAddon::Show(e.what(), NotificationAddon::Type::Failure);
        }
    }
    // 履歴からの復元
    if (history.is_page_refreshed()){
        const auto& snapshot = history.pick_snapshot();
        restore(snapshot);
    }
}

void EditRoom::render(){
    // GUI
    // テキストボックスを表示する
    GPT_answer_guide_font(U"GPT回答本文").drawBase(GPT_answer_guide_area.bl(), ColorF{0.7});
    SimpleGUI::TextArea(user_request_text_state, input_textbox_area.pos, input_textbox_area.size, 800, editable);
    SimpleGUI::TextArea(GPT_answer_text_state, GPT_answer_area.pos, GPT_answer_area.size, 5000, false);
    // ボタン表示
    bool button_enable = editable;
    if (SimpleGUI::Button(U"\U000F1C4D 送信", input_button_area.pos, input_button_area.w, button_enable)) { 
        const String input = user_request_text_state.text;
        const String user_input = U"# ユーザからの入力 \n\n" + input + U"\n\n" + controller_panel->describe();
        // #FIXME Agentsに渡せるように！
        composers.request(user_input);
    }
    
    RoundRect{menu_area, 5}.drawFrame(5, ColorF{0.6});
    GPT_answer_guide_font(composers.model_name()).draw(menu_area.h * 0.5, Arg::center = menu_area.center(), ColorF{0.6});
    if (menu_area.mouseOver()) { RoundRect{menu_area, 5}.draw(ColorF{0.6, 0.5}); }

    // 描画
    RoundRect{composed_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    RoundRect{quantitative_controll_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    composed_viewer.render(player);
    assert(controller_panel);
    controller_panel->render();
    history.render();

    if (composers.is_downloading()){
        Scene::Rect().draw(ColorF{0, 0.4});
        Circle{ Scene::Center(), 50 }.drawArc((Scene::Time() * 120_deg), 300_deg, 4, 4);
        String msg = composers.state_message() + U"...";
        FontAsset(U"default")(msg).draw(Arg::topCenter = Scene::Center() + Vec2{0, 60});
    }
}

void EditRoom::reset(){
    composers = LLMAgents{model_name};
    player = Composed{};
    composed_viewer = ComposedViewer{};
    history = HistoryViewer{};
    composed_viewer.set_renderer_area(composed_area);
    switch_panel(ParameterControllMode::HarmonicGuide);

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
    controller_panel->set_render_area(quantitative_controll_area);
}

void EditRoom::restore(const HistoryViewer::Snapshot& snapshot){
    player.set_answer(snapshot.answer_from_LLM);

    user_request_text_state = TextAreaEditState{snapshot.request};
    GPT_answer_text_state   = TextAreaEditState{snapshot.answer_from_LLM};
    editable = (history.current_page == history.size() - 1);
    switch_panel(snapshot.params_type);
    controller_panel->on_submit();
    controller_panel->set_state(editable);
    controller_panel->set_render_area(quantitative_controll_area);
}
void EditRoom::set_GPT_answer(const String& answer){
    player.set_answer(answer);
    
    controller_panel->on_submit();
    GPT_answer_text_state = TextAreaEditState{answer};
    history.remember(HistoryViewer::Snapshot{
        user_request_text_state.text,
        GPT_answer_text_state.text,
        mode,
        controller_panel->snapshot()
    });
    history.LLM_dialog = composers.snapshot();
    history.save(player.get_title(), starting_time_session);
}

void EditRoom::load_history(const FilePath& fp) {
    history.load_json(fp);
    composers.remember(history.LLM_dialog);
}

void EditRoom::save_history() {
    const String title = player.get_title();
    history.LLM_dialog = composers.snapshot();
    history.save(title, starting_time_session);
}