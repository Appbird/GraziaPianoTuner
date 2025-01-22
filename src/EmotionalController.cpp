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

Point EmotionalController::display_point(){
    return (emotional_point * Vec2{1, -1} * display_panel.size / 2 + display_panel.center()).asPoint();
}
void EmotionalController::set_emotional_point(const Point& touched_point){
    emotional_point = (Vec2(touched_point - display_panel.center()) / (display_panel.size / 2)) * Vec2{1, -1};
    emotional_point.x = Clamp(emotional_point.x, -1.0, 1.0);
    emotional_point.y = Clamp(emotional_point.y, -1.0, 1.0);
}

EmotionalController::Snapshot EmotionalController::snapshot_internal() const{
    return {
        x_axis_text_state.text,
        y_axis_text_state.text,
        emotional_point,
        using_param
    };
}


JSON EmotionalController::Snapshot::encode() const{
    JSON result;
    result[U"X_axis"] = X_axis;
    result[U"Y_axis"] = Y_axis;
    result[U"Vec2"][U"x"] = emotional_parameters.x;
    result[U"Vec2"][U"y"] = emotional_parameters.y;
    result[U"is_used"] = is_used;
    return result;
}
EmotionalController::Snapshot EmotionalController::Snapshot::decode(const JSON& json) {
    assert(json[U"X_axis"].isString());
    assert(json[U"Y_axis"].isString());
    assert(json[U"Vec2"].isObject());
        assert(json[U"Vec2"][U"x"].isNumber());
        assert(json[U"Vec2"][U"y"].isNumber());
    assert(json[U"is_used"].isBool());
    
    return {
        json[U"X_axis"].getString(),
        json[U"Y_axis"].getString(),
        Vec2{
            json[U"Vec2"][U"x"].get<double>(),
            json[U"Vec2"][U"y"].get<double>()
        },
        json[U"is_used"].get<bool>()
    };
}
String EmotionalController::Snapshot::describe() const {
    String parameters_description = U"\t{}:{: .2f}\n\t{}:{: .2f}\n"_fmt(
        X_axis,
        emotional_parameters.x,
        Y_axis,
        emotional_parameters.y
    );
    return (
        U"\n# Current Conceptual Parameters\n"
        "\n"
    ) + parameters_description;
}


