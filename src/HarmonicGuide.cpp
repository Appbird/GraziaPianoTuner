# include "HarmonicGuide.hpp"
# include "Layout.hpp"
# include <regex>

void HarmonicGuide::set_render_area(const Rect& render_area){
    display_rect    = render_area;
}
void HarmonicGuide::update(){
    
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

