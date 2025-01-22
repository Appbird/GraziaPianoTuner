# include "HarmonicGuide.hpp"
# include "Layout.hpp"
# include <regex>

void HarmonicGuide::set_render_area(const Rect& render_area){
    display_rect    = render_area;
}
void HarmonicGuide::update(){
    // #TODO 実装
}

void HarmonicGuide::render(){
    const HSV background_color  {180, 0.05, 0.1};
    const HSV axis_color        {180, 0.5, 1, 0.5};
    const HSV basepoint_color   {180, 0.2, 0.4};
    const HSV point_color       {180, (active) ? 0.6 : 0, (active) ? 1 : 0.5};
    const SizeF axis_arrow_head {20.0, 20.0};

    Rect panel_table = display_rect.stretched(-5);
    RectSlicer slicer{panel_table, RectSlicer::X_axis};
    Rect sequence_panel = slicer.from(0.05).to(0.7).stretched(-5);
    Rect semantic_panel = slicer.from(0.7).to(0.95).stretched(-5);
    RoundRect{sequence_panel, 5}
        .draw(background_color)
        .drawFrame(1, axis_color);
        
    RoundRect{semantic_panel, 5}
        .draw(background_color)
        .drawFrame(1, axis_color);
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
        const int32_t bar_start = control_unit_length*idx + 1;
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
