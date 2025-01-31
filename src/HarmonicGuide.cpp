# include "HarmonicGuide.hpp"
# include "Layout.hpp"
# include <regex>
# include "DebugTools.hpp"

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
    if (sequence_panel.leftPressed() or current_control_unit_index) { write_sequence( Cursor::Pos() ); }
}

double HarmonicGuide::bar_separator_x(int32_t bar) const {
    return bar_separator_width() * bar + sequence_panel.tl().x;
}

double HarmonicGuide::bar_separator_x(double bar) const {
    return bar_separator_width() * bar + sequence_panel.tl().x;
}
double HarmonicGuide::bar_separator_width() const {
    return double(sequence_panel.w) / count_of_bars;
}

void HarmonicGuide::render(){
    draw_sequence_panel();
    draw_semantic_panel();
}

void HarmonicGuide::draw_semantic_panel() {
    RoundRect{semantic_panel, 5}
        .draw(panel_background_color)
        .drawFrame(1, axis_color);
    {

        RectSlicer slicer{semantic_panel.stretched(-5), RectSlicer::Y_axis};
        {
            Rect first_row = slicer.to(0.333);
            Rect textbox_axis = labelled_region(first_row, guide_font(U"Y axis"), font_color);
            SimpleGUI::TextBox(y_axis_text_state, textbox_axis.tl(), textbox_axis.w, unspecified, true);
        }
        {
            //Rect second_row = slicer.to(0.666);
            //Rect button_target = labelled_region(second_row, guide_font(U"Target"), font_color);
            //SimpleGUI::Button(U"コード進行", button_target.tl(), button_target.w, true);
        }
        Rect third_row = slicer.to(1.0);
        SimpleGUI::Button(U"\U000F0493 モード切り替え", third_row.tl(), third_row.w, true);
    }
}

void HarmonicGuide::draw_sequence_panel() const {
    RoundRect{sequence_panel, 5}.draw(panel_background_color);
    top_limitzone.draw(limitzone_color);
    bottom_limitzone.draw(limitzone_color);
    RoundRect{sequence_panel, 5}.drawFrame(5, separator_color);
    const double left_bar_x     = bar_separator_x(left_bar);
    const double right_bar_x    = bar_separator_x(right_bar);
    RectF::FromPoints(Vec2{left_bar_x, top_limitzone.topY()}, Vec2{right_bar_x, bottom_limitzone.bottomY()}).draw(HSV{axis_color, 0.15});


    guide_font(U"  y:{}"_fmt(y_axis_text_state.text)).draw(20, Arg::topLeft = top_limitzone.tl(), font_color);
    guide_font(U"  max").draw(20, Arg::topLeft = middlezone.tl(), axis_color);
    guide_font(U"  min").draw(20, Arg::bottomLeft = middlezone.bl(), axis_color);
    guide_font(U"t:小節 ").draw(20, Arg::topRight = bottom_limitzone.tr(), font_color);
    
    draw_bar_separators();
    sequence_panel.left().drawArrow(5.0, {10.0, 10.0}, axis_color);
    bottom_limitzone.top().drawArrow(5.0, {10.0, 10.0}, axis_color);
    plot_sequence(); 
}

void HarmonicGuide::draw_bar_separators() const {
    for (int32_t bar = 0; bar < count_of_bars; bar += control_unit_length) {
        RectF control_unit = RectF::FromPoints(
            Vec2{bar_separator_x(bar), sequence_panel.topY()},
            Vec2{bar_separator_x(bar + control_unit_length), sequence_panel.bottomY()}
        );
        control_unit.right().draw(2, separator_color);
        guide_font(U"{} "_fmt(bar + control_unit_length)).draw(
            16, Arg::bottomRight = Vec2{control_unit.rightX(), middlezone.bottomY()},
            axis_color
        );
    }
}
double HarmonicGuide::intensity_to_plotted_point_y(double intensity) const {
    intensity = Clamp(intensity, -1.0, 2.0);
    if (intensity < 0.0) {
        return std::lerp(bottom_limitzone.bottomY(), bottom_limitzone.topY(), intensity + 1.0);
    } else if (intensity < 1.0) {
        return std::lerp(middlezone.bottomY(), middlezone.topY(), intensity);
    } else {
        return std::lerp(top_limitzone.bottomY(), top_limitzone.topY(), intensity - 1.0);
    }
}

double HarmonicGuide::plotted_point_y_to_intensity(double y) const {
    y = Clamp<double>(y, sequence_panel.topY(), sequence_panel.bottomY());
    if (y > bottom_limitzone.topY()) {
        return double(bottom_limitzone.bottomY() - y) / bottom_limitzone.h - 1.0;
    } else if (y > middlezone.topY()) {
        return double(middlezone.bottomY() - y) / middlezone.h;
    } else {
        return double(top_limitzone.bottomY() - y) / top_limitzone.h + 1.0;
    }
}

Color HarmonicGuide::plotted_point_color(double intensity) const {
    intensity = Clamp(intensity, -1.0, 2.0);
    if (intensity < 0.0) {
        return Color{overlimit_point_color}.lerp(control_point_color, intensity + 1.0);
    } else if (intensity < 1.0) {
        return control_point_color;
    } else {
        return Color{control_point_color}.lerp(overlimit_point_color, intensity - 1.0);
    }
}

void HarmonicGuide::plot_sequence() const {
    Vec2 previous;
    int32_t i = 0;
    for (int32_t bar = control_unit_length/2; bar < count_of_bars; bar += control_unit_length, i++) {
        const double point_x = bar_separator_x(bar);
        const double point_y = intensity_to_plotted_point_y(sequence[i]); 
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

void HarmonicGuide::write_sequence(const Point& touched_point){
    if (sequence_panel.leftClicked()) {
        const int32_t bar = int32_t((touched_point.x - sequence_panel.x) / bar_separator_width());
        current_control_unit_index = bar / control_unit_length;
    } else if (not MouseL.pressed()) {
        current_control_unit_index = none;
    }
    if (current_control_unit_index and InRange<int32_t>(*current_control_unit_index, 0, sequence.size()-1)) {
        sequence[*current_control_unit_index] = plotted_point_y_to_intensity(touched_point.y);
    }
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
void HarmonicGuide::memento(const JSON& json) {
    Snapshot snapshot = Snapshot::decode(json);
    assert(snapshot.guide_version == 1);
    count_of_bars = snapshot.count_of_bars;
    control_unit_length = snapshot.control_unit_length;
    sequence = snapshot.sequence;
    y_axis_text_state.text = snapshot.axis_name;
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
    String title = U"# Current Customizable Semantic Parameters";
    
    String header_line = U"| Parameter name |";
    String under_header = U"|---|";
    String param_seq = U"| {} |"_fmt(axis_name);
    for (const auto [idx, value]: Indexed(sequence)){
        const int32_t bar_start = control_unit_length*int32_t(idx) + 1;
        const int32_t bar_end   = bar_start + control_unit_length - 1;
        header_line     += U" {} - {} 小節 |"_fmt(bar_start, bar_end);
        under_header    += U"---|";
        param_seq       += U" {:.2f} |"_fmt(value);
    }
    String result = title   + U"\n\n";
    result += header_line   + U"\n";
    result += under_header  + U"\n";
    result += param_seq     + U"\n";
    return result;
}
