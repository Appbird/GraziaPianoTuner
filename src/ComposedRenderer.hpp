# pragma once
# include <Siv3D.hpp>
# include <algorithm>
# include "Music.hpp"
# include "NoteOccuranceEffect.hpp"
# include "util.hpp"

static int mod(int a, int p){
    const int m = a % p;
    const int result = m + ((m >= 0) ? 0 : p);
    assert(InRange(result, 0, p - 1));
    return result;
}


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
        double piano_role_boundary = 50;
        double highest_pitch()      const { return lowest_pitch + n_halfpitch_in_area; }
        double latest_beat()        const { return earliest_beat + n_beats_in_area; }
        double width_per_beat()     const { return rendered_area.w / n_beats_in_area; }
        double height_per_pitch()   const { return rendered_area.h / n_halfpitch_in_area; }
        
        void render_beat();
        void render_pitch();
        void render_current_position(const double current_position);
        void render_frame(){
            rendered_area
            .drawFrame(5, HSV{main_color.h, main_color.s * 0.3, main_color.v * 1.5})
            .draw(HSV{main_color.h, 0, main_color.v * 0.3});
        }
        void render_notes(
            const Music& music,
            const double current_beat,
            const NoteIterator& occur_end
        ) const;
        void occur_particle(
            const NoteIterator& occur_begin,
            const NoteIterator& occur_end
        ){
            for (auto iter = occur_begin; iter < occur_end; iter++){
                // #NOTE 画面外にあるエフェクトは描画しなくても良い[最適化]
                effect.add<NoteOccuranceEffect>(*this, *iter, 0.8, 20);
            }
        }

        Rect piano_area {
            0, 200,
            int(piano_role_boundary), 300
        };   /** 描画領域 */
        Rect rendered_area{
            int(piano_role_boundary), 200,
            Scene::Size().x - int(piano_role_boundary) - 5, 300
        };   /** 描画領域 */
        
        // thanks to https://twitter.com/masaka_k/status/1536397542879297536?s=20
        void render_piano() const{
            constexpr int oct = 12; // オクターブ
            int key = mod((int)floor(lowest_pitch - 60), oct);
            int intital_pitch = floor(lowest_pitch);

            for (
                int p = intital_pitch; p < highest_pitch();
                p++, key = (key + 1 == oct) ? 0 : key + 1
            ){
                const std::array<int, 8> white_keys{0, 2, 4, 5, 7, 9, 11, 12};
                Point offset{0, -int((p - intital_pitch) * height_per_pitch())};
                const Color frame_color = ColorF{0.8, 0.8, 0.8};
                // 白鍵の描画
                // #NOTE バグ:: (tl, size)を(tl, br)と勘違いしていたせいで時間を溶かす
                Rect {
                    piano_area.bl() - Point{0, (int)height_per_pitch()} + offset,
                    Point{piano_area.w, (int)height_per_pitch()}
                }
                .drawFrame(2, frame_color)
                .draw(ColorF{0.9, 0.9, 0.9});
                // #NOTE ここは最適化できる [最適化]
                const auto white_keys_iter = std::lower_bound(white_keys.begin(), white_keys.end(), key);
                
                if (*white_keys_iter != key) {
                    //黒鍵だった場合
                    Rect {
                        piano_area.bl() - Point{0, (int)height_per_pitch()}     + offset,
                        Point{int(piano_area.w * 0.6), (int)height_per_pitch()}
                    }
                    .drawFrame(2, frame_color)
                    .draw(ColorF{0.05, 0.05, 0.05});
                }
            }
        }
        /**
         * @brief 与えられた点pがノーツ描画領域内に収まるように修正する。
         */
        Point clamp_X(const Point& p) const{
            return Point{Clamp(p.x, rendered_area.leftX() + 1, rendered_area.rightX()), p.y};
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
            const Point actual_tl = rendered_area.tl() + note_offsetVec2 + Size(2, 2);
            const Point tl = clamp_X(actual_tl);
            const Point scraped_tl = tl - actual_tl;
            Rect note_rect{
                tl,
                clamp_X(tl + note_size) - tl - scraped_tl
            };
            return note_rect;
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
            render_piano();
            if (music) { render_notes(music, current_beat, occur_end); }
            render_current_position(current_beat);
            occur_particle(occur_begin, occur_end);
            {
                const ScopedRenderStates2D blender{ BlendState::Additive };
                effect.update();
            }
        }
        void update_autoscrool(const double current_beat, const double max_beat){
            const double offset_beat = 4;
            const double current_scroll = current_beat - offset_beat;
            const double min_scroll = 0.0;
            const double max_scroll = max_beat - n_beats_in_area;
            earliest_beat = Clamp(current_scroll, min_scroll, max_scroll);
        }
};
