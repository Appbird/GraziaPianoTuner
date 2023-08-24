# pragma once
# include <Siv3D.hpp>

# include "Music.hpp"
# include "NoteOccuranceEffect.hpp"
# include "util.hpp"
/**
 * @brief 生成した音楽をピアノロールとしてレンダリングする。
 */
class ComposedRenderer{
    private:
        Effect effect;
        double n_beats_in_area = 12;
        double n_halfpitch_in_area = 24; 
        double lowest_pitch = 65;
        double earliest_beat = 0;
        Rect rendered_area{0, 200, Scene::Size().x, 300};   /** 描画領域 */
        double highest_pitch()      const { return lowest_pitch + n_halfpitch_in_area; }
        double latest_beat()        const { return earliest_beat + n_beats_in_area; }
        double width_per_beat()     const { return rendered_area.w / n_beats_in_area; }
        double height_per_pitch()   const { return rendered_area.h / n_halfpitch_in_area; }
        
        void render_beat(){
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
        void render_pitch(){
            for (int p = int(ceil(lowest_pitch)); p <= int(floor(highest_pitch())); p++){
                const double delta_pitch = highest_pitch() - p;
                Size offset{0, int(this->height_per_pitch() * delta_pitch)};
                Line{
                    rendered_area.tl() + offset,
                    rendered_area.tr() + offset
                }.draw(2,HSV{0, 0, 0.15});
            }
        }
        void render_current_position(const double current_position){
            if (not (InRange(current_position, earliest_beat, latest_beat()))){ return; }
            Size offset{int(this->width_per_beat() * (current_position - earliest_beat) ), 0};
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
        void render_frame(){
            rendered_area
            .drawFrame(5, HSV{main_color.h, main_color.s * 0.3, main_color.v * 1.5})
            .draw(HSV{main_color.h, 0, main_color.v * 0.3});
        }
        void render_notes(const Music& music, const double current_beat, const NoteIterator& occur_end){
            assert(music);
            const auto start    = music.get_notes().begin();
            const auto end      = music.get_notes().end();
            for (NoteIterator noteiter = start; noteiter < end; noteiter++){
                // ノートの開始拍数
                // q = quater note(一拍)
                const int end_beat = noteiter->start_beats + noteiter->duration_beats;
                if (end_beat < earliest_beat or latest_beat() < noteiter->start_beats)           { continue; }
                if (not InRange(double(noteiter->note_number), lowest_pitch, highest_pitch()))   { continue; }
                
                // すでに引かれているか否かで描画色を帰る
                const bool is_playing = noteiter->duration_beats >= current_beat - occur_end->start_beats;
                const bool confronting = noteiter >= occur_end;
                const HSV note_color = 
                    (confronting)   ? HSV{0, 0, 1} :
                    (is_playing)    ? HSV{220, 0.3,  0.7} :
                                      HSV{200, 0.2,  0.5};

                note_rect(*noteiter)
                .drawFrame(2.0, HSV{main_color.h, main_color.s, 0.5})
                .draw(note_color);
            }
        }
        void occur_particle(
            const NoteIterator& occur_begin,
            const NoteIterator& occur_end
        ){
            for (auto iter = occur_begin; iter < occur_end; iter++){
                // #NOTE 画面外にあるエフェクトは描画しなくても良い[最適化]
                effect.add<NoteOccuranceEffect>(*this, *iter, 0.8, 10);
            }
        }
    public:
        Rect note_rect(const Note& note) const{
            const Size note_offsetVec2{
                (int)round((note.start_beats - earliest_beat) * width_per_beat()),
                (int)round((highest_pitch() - note.note_number) * height_per_pitch())
            };
            const Size note_size{
                (int)round(note.duration_beats * width_per_beat()),
                (int)round(1 * height_per_pitch())
            };
            return Rect{rendered_area.tl() + note_offsetVec2 + Size(2, 2), note_size - Size(2, 2)};
        }
        /**
         * @brief ピアノロールを描画する
         * 
         * @param music             音楽データ
         * @param current_beat      現在の再生位置
         * @param occur_begin       今フレームで再生エフェクトを生起させるべきノートのうち、もっとも番号が若いもの
         * @param occur_end         今フレームで再生エフェクトを生起させるべきノートのうち、最も番号が大きいもの。
         */
        void render(
            const Music& music,
            const double current_beat,
            const NoteIterator& occur_begin,
            const NoteIterator& occur_end
        ){
            // InRangeを使うとなぜか型エラーが起こる。
            assert(music.get_notes().begin() <= occur_begin and occur_begin <= music.get_notes().end());
            assert(music.get_notes().begin() <= occur_end and occur_end <= music.get_notes().end());
            
            render_frame();
            render_pitch();
            render_beat();
            render_current_position(current_beat);
            occur_particle(occur_begin, occur_end);
            effect.update();
            if (music) { render_notes(music, current_beat, occur_end); }
        }
        void update_autoscrool(const double current_beat, const double max_beat){
            const double offset_beat = 4;
            const double current_scroll = current_beat - offset_beat;
            const double min_scroll = 0.0;
            const double max_scroll = max_beat - n_beats_in_area;
            earliest_beat = Clamp(current_scroll, min_scroll, max_scroll);
        }
};
