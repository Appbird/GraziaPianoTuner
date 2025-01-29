# include "ComposedRenderer.hpp"

# include <algorithm>
# include <utility>

# include "NoteOccuranceEffect.hpp"
# include "Layout.hpp"
# include "Composed.hpp"

void ComposedViewer::set_renderer_area(const Rect& arg_render_area){
    const FilePath font_logotype_path = FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"ロゴたいぷゴシック.otf";
    const FilePath font_smart_path = FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"03スマートフォントUI.otf";
    
    render_area = arg_render_area;
    RectSlicer layout{ render_area, RectSlicer::Y_axis };
    information_area            = layout.to(0.1).stretched(-2);
    RectSlicer piano_role_parts = layout.slice(1.0, RectSlicer::X_axis);
    {
        auto [piano_role_left, piano_role_right]
            = piano_role_parts.devide_at(0.12, RectSlicer::Y_axis);
        piano_renderer.piano_area = piano_role_left.from(0.1).to(1.0).stretched(-2);
        chord_area = piano_role_right.to(0.1).stretched(-2);
        notes_area = piano_role_right.to(1.0).stretched(-2);
    }

    font_for_keynumber = Font{int(height_per_pitch()),font_logotype_path, };
    font_for_meta_info = Font{int(information_area.h * 0.8), font_logotype_path};
    font_for_chord     = Font{int(chord_area.h * 0.8), font_smart_path};
}

void ComposedViewer::render_frame() const{
    notes_area
    .drawFrame(5, HSV{main_color.h, main_color.s * 0.3, main_color.v * 1.5})
    .draw(HSV{main_color.h, 0, main_color.v * 0.3});
} 

void ComposedViewer::occur_particle(
    const NoteIterator& occur_begin,
    const NoteIterator& occur_end
) const{
    for (auto iter = occur_begin; iter < occur_end; iter++){
        // #NOTE 画面外にあるエフェクトは描画しなくても良い[最適化]
        effect.add<NoteOccuranceEffect>(*this, *iter, 0.8, 20);
    }
}

void ComposedViewer::render_beat() const{
    for (int b = int(ceil(earliest_beat)); b <= int(floor(latest_beat())); b++){
        const double delta_beat = b - earliest_beat;
        Size offset{int(this->width_per_beat() * delta_beat), 0};
        const int thickness         = (b % 4 == 0) ? 3 : 2;
        const double color_value    = (b % 4 == 0) ? 0.3 : 0.15;
        Line{
            notes_area.tl() + offset,
            notes_area.bl() + offset
        }.draw(thickness, HSV{0, 0, color_value});
        if (b % 4 == 0) {
            font_for_chord(ToString(b/4+1)).draw(height_per_pitch(), notes_area.tl() + offset, ColorF{0.6});
        }
        
    }
}
void ComposedViewer::render_pitch() const{
    for (int p = int(ceil(lowest_pitch)); p <= int(floor(highest_pitch())); p++){
        const double delta_pitch = highest_pitch() - p;
        Size offset{0, int(this->height_per_pitch() * delta_pitch)};
        Line{
            notes_area.tl() + offset,
            notes_area.tr() + offset
        }.draw(2,HSV{0, 0, 0.15});
    }
}
void ComposedViewer::render_current_position(const double current_position) const{
    if (not (InRange(current_position, earliest_beat, latest_beat()))){ return; }
    Size offset{(int)round(this->width_per_beat() * (current_position - earliest_beat) ), 0};
    const Line sequence_line{
        notes_area.tl() + offset,
        notes_area.bl() + offset
    };
    sequence_line.draw(5, HSV{200, 0.05, 0.9});
    const double marker_size = 20;
    const Point marker_offset = Point{0, 10};
    Triangle{sequence_line.begin - marker_offset, marker_size, Math::Pi}.draw(HSV{200, 0.05, 0.9});
    Triangle{sequence_line.end + marker_offset, marker_size, 0}.draw(HSV{200, 0.05, 0.9});
    
}

void ComposedViewer::render_notes(
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
        if (end_beat < earliest_beat or latest_beat() < noteiter->start_beats)                  { continue; }
        if (not InRange(double(noteiter->note_number), lowest_pitch, highest_pitch() + 1))      { continue; }
        
        // すでに引かれているか否かで描画色を変える。
        const bool is_playing = noteiter->duration_beats >= current_beat - noteiter->start_beats;
        const bool confronting = noteiter >= occur_end;
        const HSV note_color = 
            (confronting)   ?   HSV{0, 0, 0.8} :
            (is_playing)    ?   HSV{180, 0.2,  1} :
                                HSV{200, 0.2,  0.4};

        note_rect(*noteiter)
            .drawFrame(2.0, HSV{main_color.h, main_color.s, 0.5})
            .draw(note_color);
    }
}

Rect ComposedViewer::note_rect(const Note& note) const{
    const Size note_offsetVec2{
        (int)round((note.start_beats - earliest_beat) * width_per_beat()),
        (int)round((highest_pitch() - note.note_number) * height_per_pitch())
    };
    const Size note_size{
        (int)round(note.duration_beats * width_per_beat()),
        (int)round(1 * height_per_pitch())
    };
    Rect note_rect{notes_area.tl() + note_offsetVec2, note_size};
    return note_rect;
    
}

void ComposedViewer::render_chord_information(const Music& music) const{
    Graphics2D::SetScissorRect(chord_area);
    RasterizerState rs = RasterizerState::Default2D;
    rs.scissorEnable = true;
    const ScopedRenderStates2D rasterizer{rs};

    const auto chord_start = music.get_next_chord_index(earliest_beat) - 1;
    const auto chord_end = music.get_next_chord_index(latest_beat());
    for (auto iter = chord_start; iter < chord_end; iter++){
        const double offset = (iter->start_beats - earliest_beat) * width_per_beat();
        font_for_chord(iter->chord).drawBaseAt(
            chord_area.leftX() + font_for_chord.fontSize() * 1.5 + offset,
            chord_area.bottomY() - font_for_chord.fontSize() * 0.2,
            ColorF{0.7}
        );
    }
}


void ComposedViewer::update_scroll(){
    if (not notes_area.mouseOver()) { return; }
    const double m_x = Mouse::WheelH();
    const double m_y = -Mouse::Wheel();
    if (m_x != 0 and is_autoscrolling) { is_autoscrolling = false; }
    double mousewheeel_pitch = 0.1; double mousewheel_beat = 0.1;
    earliest_beat   += m_x * mousewheel_beat;
    lowest_pitch    += m_y * mousewheeel_pitch;
}

void ComposedViewer::render(const Composed& composed) const {
    const auto [occur_begin, occur_end] = composed.get_passing_notes();
    const Music& music                  = composed.get_music();
    const double current_beat           = composed.get_current_beat();
    // InRangeを使うとなぜか型エラーが起こる。
    assert(music.get_notes().begin() <= occur_begin and occur_begin <= music.get_notes().end());
    assert(music.get_notes().begin() <= occur_end   and occur_end <= music.get_notes().end());
    piano_renderer.render_piano(lowest_pitch, highest_pitch(), height_per_pitch(), font_for_keynumber);
    {
        Graphics2D::SetScissorRect(notes_area);
        RasterizerState rs = RasterizerState::Default2D;
        rs.scissorEnable = true;
        render_frame();
        render_pitch();
        render_beat();
    }
    if (music)
    {
        render_meta_information(music);
        if (music.is_chord_progression_exist())
        {
            render_chord_information(music);
        }

        {   
            Graphics2D::SetScissorRect(notes_area);
            RasterizerState rs = RasterizerState::Default2D;
            rs.scissorEnable = true;
            const ScopedRenderStates2D rasterizer{rs};
            render_notes(music, current_beat, occur_end);
            occur_particle(occur_begin, occur_end);
            {
                const ScopedRenderStates2D blender{ BlendState::Additive };
                effect.update();
            }
        }
        render_current_position(current_beat);
    }
}

void ComposedViewer::update(Composed& composed, bool stopper_enabled)
{
    
    if (not composed.get_music()){ return; }
    const double max_beat               = composed.get_music().get_music_length_in_beat();
    const auto  [min_pitch, max_pitch]  = composed.get_music().get_min_max_pitch();
    const double current_beat           = composed.get_current_beat();

    if (is_autoscrolling) {update_autoscrool(current_beat); }

    update_scroll();
    const double bottom_pitch = (min_pitch >= n_halfpitch_in_area) ? min_pitch - n_halfpitch_in_area : 0;
    earliest_beat   = Clamp(earliest_beat,  0.0, max_beat - n_beats_in_area);
    lowest_pitch    = Clamp(lowest_pitch,   bottom_pitch - n_halfpitch_in_area, max_pitch + n_halfpitch_in_area);

    // ユーザー入力
    if (const auto beat = clicked_beat()){ composed.seek(*beat); }   
    if (KeySpace.down() and stopper_enabled)
    {
        if (composed.get_is_playing())
        {
            composed.stop();
            is_autoscrolling = false;
        }
        else
        {
            composed.play();
            is_autoscrolling = true;
        }
    }
}