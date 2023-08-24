#include "ComposedRenderer.hpp"

void ComposedRenderer::render_beat(){
    for (int b = int(ceil(earliest_beat)); b <= int(floor(latest_beat())); b++){
        const double delta_beat = b - earliest_beat;
        Size offset{int(this->width_per_beat() * delta_beat), 0};
        const int thickness = (b % 4 == 0) ? 3 : 2;
        const double color_value = (b % 4 == 0) ? 0.3 : 0.15;
        Line{
            rendered_area.tl() + offset,
            rendered_area.bl() + offset
        }.draw(thickness, HSV{0, 0, color_value});
    }
}
void ComposedRenderer::render_pitch(){
    for (int p = int(ceil(lowest_pitch)); p <= int(floor(highest_pitch())); p++){
        const double delta_pitch = highest_pitch() - p;
        Size offset{0, int(this->height_per_pitch() * delta_pitch)};
        Line{
            rendered_area.tl() + offset,
            rendered_area.tr() + offset
        }.draw(2,HSV{0, 0, 0.15});
    }
}
void ComposedRenderer::render_current_position(const double current_position){
    if (not (InRange(current_position, earliest_beat, latest_beat()))){ return; }
    Size offset{(int)round(this->width_per_beat() * (current_position - earliest_beat) ), 0};
    const Line sequence_line{
        rendered_area.tl() + offset,
        rendered_area.bl() + offset
    };
    sequence_line.draw(5, HSV{200, 0.05, 0.9});
    const double marker_size = 20;
    const Point marker_offset = Point{0, 10};
    Triangle{sequence_line.begin - marker_offset, marker_size, Math::Pi}.draw(HSV{200, 0.05, 0.9});
    Triangle{sequence_line.end + marker_offset, marker_size, 0}.draw(HSV{200, 0.05, 0.9});
    
}

void ComposedRenderer::render_notes(
    const Music& music,
    const double current_beat,
    const NoteIterator& occur_end
) const {
    assert(music);
    const auto start    = music.get_notes().begin();
    const auto end      = music.get_notes().end();
    // note iterator
    for (NoteIterator noteiter = start; noteiter < end; noteiter++){
        // ノートの開始拍数
        // q = quater note(一拍)
        const int end_beat = noteiter->start_beats + noteiter->duration_beats;
        if (end_beat < earliest_beat or latest_beat() < noteiter->start_beats)           { continue; }
        if (not InRange(double(noteiter->note_number), lowest_pitch, highest_pitch()))   { continue; }
        
        // すでに引かれているか否かで描画色を変える。
        const bool is_playing = noteiter->duration_beats >= current_beat - noteiter->start_beats;
        const bool confronting = noteiter >= occur_end;
        const HSV note_color = 
            (confronting)   ? HSV{0, 0, 0.8} :
            (is_playing)    ? HSV{180, 0.2,  1} : HSV{200, 0.2,  0.4};

        note_rect(*noteiter)
        .drawFrame(2.0, HSV{main_color.h, main_color.s, 0.5})
        .draw(note_color);
    }
}