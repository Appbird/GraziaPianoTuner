# include "EmotionalController.hpp"
# include <regex>

void EmotionalController::set_render_area(const Rect& render_area){
    display_rect    = render_area;
    {
        RectSlicer layout{ display_rect, RectSlicer::X_axis };
        display_panel   = layout.from(0.15).to(0.55).stretched(-5);
        display_params   = layout.from(0.6).to(0.9);
    }
    {
        RectSlicer layout{ display_params, RectSlicer::Y_axis };
        Rect display_x_axis_area  = layout.from(0.05).to(0.35).stretched(-5);
        {
            RectSlicer x_axis_layout{display_x_axis_area, RectSlicer::X_axis};
            display_x_axis_label    = x_axis_layout.to(0.2);
            display_x_axis_contents = x_axis_layout.to(0.8);
            display_x_axis_value    = x_axis_layout.to(1.0);
        }
        Rect display_y_axis_area   = layout.to(0.65).stretched(-5);
        {
            RectSlicer y_axis_layout{display_y_axis_area, RectSlicer::X_axis};
            display_y_axis_label    = y_axis_layout.to(0.2);
            display_y_axis_contents = y_axis_layout.to(0.8);
            display_y_axis_value    = y_axis_layout.to(1.0);
        }
        display_checkbox_text = layout.to(0.95).stretched(-5);
    }
}
void EmotionalController::update(){
    if (not active) { return; }
    trail.update();
    if (display_point().asCircle(point_outer_radius).mouseOver()){
        Cursor::RequestStyle(CursorStyle::Hand);
    }
    if (display_point().asCircle(point_outer_radius).leftClicked()){
        dragging = true;
        using_param = true;
    }
    if (MouseL.up()) { dragging = false; }
    if (dragging){
        set_emotional_point(Cursor::Pos());
        trail.add(display_point(), HSV{180, (using_param) ? 0.6 : 0, 1}, point_outer_radius);
    }
}

void EmotionalController::render(){
    const HSV background_color  {180, 0.05, 0.1};
    const HSV axis_color        {180, 0.5, 1, 0.5};
    const HSV basepoint_color   {180, 0.2, 0.4};
    const HSV point_color       {180, (using_param and active) ? 0.6 : 0, (using_param and active) ? 1 : 0.5};
    const SizeF axis_arrow_head {20.0, 20.0};

    const Point label_x_lt{(int)display_panel.centerX() + 10,      display_panel.topY()};
    const Point label_y_br{display_panel.rightX(),     (int)display_panel.centerY() - 10};
    const double axis_label_fontsize = display_panel.h / 9;

    RoundRect{display_panel, 5}.draw(background_color).drawFrame(1, axis_color);
    Line{Point{(int)display_panel.centerX(), display_panel.bottomY()}, Point{(int)display_panel.centerX(), display_panel.topY()}}.drawArrow(2, axis_arrow_head, axis_color);
    Line{Point{display_panel.leftX(), (int)display_panel.centerY()}, Point{display_panel.rightX(), (int)display_panel.centerY()}}.drawArrow(2, axis_arrow_head, axis_color);
    
    // 感情パラメータ点の描画
    display_axis_guide_font(y_axis_text_state.text).draw(axis_label_fontsize, Arg::topLeft = label_x_lt);
    display_axis_guide_font(x_axis_text_state.text).draw(axis_label_fontsize, Arg::bottomRight = label_y_br);
    display_panel.center().asCircle(point_inner_radius).drawFrame(point_outer_radius - point_inner_radius, basepoint_color);
    trail.draw(); // パラメータ点の軌跡の描画
    display_point().asCircle(point_inner_radius).drawFrame(point_outer_radius - point_inner_radius, point_color);


    // パラメータ画面描画
    RoundRect{display_params.stretched(2), 5}.draw(background_color);
    {
        // SimpleGUI::CheckBox(using_param, U"感情パラメータ指定", display_checkbox_text.pos, display_checkbox_text.w, active);
        display_axis_guide_font(U"X軸").draw(int(display_x_axis_label.h * 0.7), Arg::center = display_x_axis_label.center());
        SimpleGUI::TextBox(x_axis_text_state, display_x_axis_contents.pos, display_x_axis_contents.w, 100UL, using_param and active);
        param_value_font(U"{: .2f}"_fmt(emotional_point.x)).draw(int(display_x_axis_label.h * 0.5), Arg::center = display_x_axis_value.center(), ColorF{0.8});

        display_axis_guide_font(U"Y軸").draw(int(display_y_axis_label.h * 0.7), Arg::center = display_y_axis_label.center());
        SimpleGUI::TextBox(y_axis_text_state, display_y_axis_contents.pos, display_y_axis_contents.w, 100UL, using_param and active);
        param_value_font(U"{: .2f}"_fmt(emotional_point.y)).draw(int(display_y_axis_label.h * 0.5), Arg::center = display_y_axis_value.center(), ColorF{0.8});    
    }
    
}


Optional<EmotionalController::EmotionalParameters> extract_axis_parameters_from_prompt(const String& GPT_answer) {
    const String axis_part = find_last_axis_block(GPT_answer);
    if (axis_part == U"") { return none; }
    Console << axis_part;
    const std::regex regex{R"(([^:]+?)\s*:\s*([+-]?[0-9.]+)\s*)"};

    std::string str_utf8 = axis_part.toUTF8();
    std::string::const_iterator text_iter = str_utf8.cbegin();

    std::vector<String> axis_names;
    std::vector<double> axis_values;

    for (
        std::sregex_iterator it(str_utf8.begin(), str_utf8.end(), regex);
        it != std::sregex_iterator();
        ++it
    ) {
        const std::smatch match = *it;
        assert(match.size() >= 3);
        axis_names.push_back(Unicode::FromUTF8(match[1].str()));
        axis_values.push_back(std::strtod(match[2].str().c_str(), NULL));
        
    }
    for (const auto axis_name: axis_names) { Console << axis_name; }
    for (const auto axis_value: axis_values) { Console << axis_value; }
    if (axis_names.size() < 2) { return none; }
    assert(axis_names.size() == axis_values.size());
    
    return EmotionalController::EmotionalParameters{
        axis_names[0], axis_names[1],
        Vec2{axis_values[0], axis_values[1]},
        true
    };
}

void EmotionalController::set_answer(const String& answer) {
    const auto extracted_result = extract_axis_parameters_from_prompt(answer);
    if (extracted_result) {
        Console << extracted_result->encode();
        memento(*extracted_result);
    }
}