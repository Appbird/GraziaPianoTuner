# include "HarmonicGuide.hpp"
# include "Layout.hpp"
# include <regex>

void HarmonicGuide::set_render_area(const Rect& render_area){
    display_rect    = render_area;

    Rect panel_table = display_rect.stretched(-5);
    {
        RectSlicer slicer{panel_table, RectSlicer::X_axis};
        slicer.from(0.03);
        sequence_panel = slicer.to(0.7).stretched(-5);
        semantic_panel = slicer.to(0.97).stretched(-5);
    }
    {
        RectSlicer slicer{sequence_panel, RectSlicer::Y_axis};
        top_limitzone      = slicer.to(0.15); 
        middlezone         = slicer.to(0.85);
        bottom_limitzone   = slicer.to(1.0);
    }
}
void HarmonicGuide::update(){
    // #TODO 実装
}

double HarmonicGuide::bar_separator_x(int32_t bar) const {
    return double(sequence_panel.w) * bar / count_of_bars + sequence_panel.tl().x;
};

void HarmonicGuide::render(){
    draw_background();
    draw_bar_separators();
    
    // axis
    sequence_panel.left().drawArrow(5.0, {10.0, 10.0}, axis_color);
    bottom_limitzone.top().drawArrow(5.0, {10.0, 10.0}, axis_color);

    // 点の系列
    plot_sequence();            

    RoundRect{semantic_panel, 5}
        .draw(panel_background_color)
        .drawFrame(1, axis_color);    
}

void HarmonicGuide::draw_background() const {
    RoundRect{sequence_panel, 5}.draw(panel_background_color);
    top_limitzone.draw(limitzone_color);
    bottom_limitzone.draw(limitzone_color);
    RoundRect{sequence_panel, 5}.drawFrame(5, separator_color);
}

void HarmonicGuide::draw_bar_separators() const {
    for (int32_t bar = 0; bar < count_of_bars; bar += control_unit_length) {
        RectF control_unit = RectF::FromPoints(
            Vec2{bar_separator_x(bar), sequence_panel.topY()},
            Vec2{bar_separator_x(bar + control_unit_length), sequence_panel.bottomY()}
        );
        control_unit.right().draw(2, separator_color);
        guide_font(U" {}"_fmt(bar)).draw(
            16, Arg::bottomLeft = Vec2{control_unit.leftX(), middlezone.bottomY()},
            axis_color
        );
    }
}
double HarmonicGuide::plotted_point_y(double intensity) const {
    intensity = Clamp(intensity, -1.0, 2.0);
    if (intensity < 0.0) {
        return std::lerp(bottom_limitzone.bottomY(), bottom_limitzone.topY(), intensity + 1.0);
    }
    else if (intensity < 1.0) {
        return std::lerp(middlezone.bottomY(), middlezone.topY(), intensity);
    }
    else {
        return std::lerp(top_limitzone.bottomY(), top_limitzone.topY(), intensity - 1.0);
    }
}
Color HarmonicGuide::plotted_point_color(double intensity) const {
    intensity = Clamp(intensity, -1.0, 2.0);
    if (intensity < 0.0) {
        return Color{overlimit_point_color}.lerp(control_point_color, intensity + 1.0);
    }
    else if (intensity < 1.0) {
        return control_point_color;
    }
    else {
        return Color{control_point_color}.lerp(overlimit_point_color, intensity - 1.0);
    }
}

void HarmonicGuide::plot_sequence() const {
    Vec2 previous;
    int32_t i = 0;
    for (int32_t bar = control_unit_length/2; bar < count_of_bars; bar += control_unit_length, i++) {
        const double point_x = bar_separator_x(bar);
        const double point_y = plotted_point_y(sequence[i]); 
        Vec2 current = {point_x, point_y};
        Circle(current, point_inner_radius)
            .drawFrame(
                point_outer_radius - point_inner_radius,
                plotted_point_color(sequence[i])
            );
        if (i > 0) {
            Line line{previous, current};
            line.stretched(-point_inner_radius)
                .draw(2.0, plotted_point_color(sequence[i-1]), plotted_point_color(sequence[i]));
        }
        previous = current;
    }
}

void HarmonicGuide::set_state(bool active) {
    this->active = active;
}

Point HarmonicGuide::display_point(int32_t index){
    //#TODO どの位置を触ったかによって、sequenceのどの位置が書き換えられたかを計算する。
    return {0, 0};
}
void HarmonicGuide::write_sequence(const Point& touched_point){
    //#TODO どの位置を触ったかによって、sequenceのどの位置が書き換えられたかを計算する。
    return;
}
HarmonicGuide::Snapshot HarmonicGuide::snapshot_internal() const {
    return Snapshot{
        1,
        count_of_bars,
        control_unit_length,
        y_axis_text_state.text,
        sequence
    };
}

// ------------------------------------------------------
// Snapshot
// ------------------------------------------------------

JSON HarmonicGuide::Snapshot::encode() const{
    JSON result;

    Field2JSON(result, guide_version);
    Field2JSON(result, count_of_bars);
    Field2JSON(result, axis_name);
    Field2JSON(result, control_unit_length);
    Field2JSON_Array(result, sequence);

    return result;
}
HarmonicGuide::Snapshot HarmonicGuide::Snapshot::decode(const JSON& json){
    // c.f. TODO: JSON Validator（JSON Schema）の使い方 https://scrapbox.io/Siv3D-instances/TODO:_JSON_Validator%EF%BC%88JSON_Schema%EF%BC%89%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9
    {
        //#TODO schemaにcontrol_unit_lengthを加える。
        JSONValidator validator = JSONValidator::Load(U"HarmonicGuideParams.schema.json");
        validator.validationAssert(json);
    }
    Snapshot param;
    JSON2Field(json, param, guide_version);
    JSON2Field(json, param, count_of_bars);
    JSON2Field(json, param, axis_name);
    JSON2Field(json, param, control_unit_length);
    JSON2Field_Array(json, param, sequence, double);
    return param;
}
String HarmonicGuide::Snapshot::describe() const {
    String title = U"# Conceptual Parameters";
    
    String header_line = U"| Parameter name |";
    String under_header = U"|---|";
    String param_seq = U"| {} |"_fmt(axis_name);
    for (const auto [idx, value]: Indexed(sequence)){
        const int32_t bar_start = control_unit_length*int32_t(idx) + 1;
        const int32_t bar_end   = bar_start + control_unit_length - 1;
        header_line     += U" bar {} - {} |"_fmt(bar_start, bar_end);
        under_header    += U"---|";
        param_seq       += U" {} |"_fmt(value);
    }
    String result = title   + U"\n\n";
    result += header_line   + U"\n";
    result += under_header  + U"\n";
    result += param_seq     + U"\n";
    return result;
}
