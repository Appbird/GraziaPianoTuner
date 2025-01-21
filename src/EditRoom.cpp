# include "EditRoom.hpp"

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
    if (mode == Mode::EmotionalController)  { emotional_controller.update(); }
    else if (mode == Mode::HarmonicGuide)   { harmonic_guide.update(); }
    history.update();
    

    // GPT4からanswerを得られた時だけ更新。
    if (const auto ans = musicalGPT4.try_to_get_answer()){ set_GPT_answer(*ans); }
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
        or (emotional_controller.using_param and history.size() > 0)
    ) and editable;
    if (SimpleGUI::Button(U"\U000F1C4D 送信", input_button_area.pos, input_button_area.w, button_enable))
    { 
        const String input = user_request_text_state.text;
        musicalGPT4.request(input, emotional_controller);
        
    }
    
    RoundRect{menu_area, 5}.drawFrame(5, ColorF{0.6});
    GPT_answer_guide_font(musicalGPT4.model).draw(menu_area.h * 0.5, Arg::center = menu_area.center(), ColorF{0.6});
    if (menu_area.mouseOver()) {RoundRect{menu_area, 5}.draw(ColorF{0.6, 0.5});}
    // 描画
    RoundRect{composed_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    RoundRect{quantitative_controll_area.stretched(5), 5}.drawFrame(2, ColorF{0.3});
    composed_viewer.render(player);
    if (mode == Mode::EmotionalController)  { emotional_controller.render(); }
    else if (mode == Mode::HarmonicGuide)   { harmonic_guide.render(); }
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
    emotional_controller = EmotionalController{};
    harmonic_guide = HarmonicGuide{};
    history = HistoryViewer{};
    composed_viewer.set_renderer_area(composed_area);
    if (mode == Mode::EmotionalController) {
        emotional_controller.set_render_area(quantitative_controll_area);
    } else if (mode == Mode::HarmonicGuide) {
        harmonic_guide.set_render_area(quantitative_controll_area);
    }
    history.set_render_area(page_flipper_area);
    user_request_text_state = TextAreaEditState{U""};
    GPT_answer_text_state   = TextAreaEditState{U""};

    starting_time_session = DateTime::Now();
}