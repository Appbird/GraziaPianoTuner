# pragma once
# include <Siv3D.hpp>
# include "PianoRenderer.hpp"
# include "Music.hpp"
# include "util.hpp"


class PianoRenderer;
class Composed;
class Music;

/**
 * @brief 生成した音楽をピアノロールとしてレンダリングする。
 */
class ComposedViewer{
    private:
        Effect effect;
        Rect render_area;
        Rect information_area;
        PianoRenderer piano_renderer;
        Rect chord_area;
        Rect notes_area;
        
        double n_beats_in_area = 12;    double n_halfpitch_in_area = 24; 
        double lowest_pitch = 65;       double earliest_beat = 0;
        double highest_pitch()      const { return lowest_pitch     + n_halfpitch_in_area;  }
        double latest_beat()        const { return earliest_beat    + n_beats_in_area;      }
        double width_per_beat()     const { return notes_area.w     / n_beats_in_area;      }
        double height_per_pitch()   const { return notes_area.h     / n_halfpitch_in_area;  }
        
        Font font_for_keynumber;
        Font font_for_meta_info;
        Font font_for_chord;
        bool is_autoscrolling = false;

        void render_beat() const;
        void render_pitch() const;
        void render_current_position(const double current_position) const;
        void render_frame() const;
        void render_notes(
            const Music& music,
            const double current_beat,
            const NoteIterator& occur_end
        ) const;
        void occur_particle(
            const NoteIterator& occur_begin,
            const NoteIterator& occur_end
        ) const;
        
        // #FIXME 変拍子になるとバグる 
        // 元のchord_eventの割り当てアルゴリズムがよくない。
        void render_chord_information(const Music& music) const;
        void update_scroll();

        void update_autoscrool(const double current_beat)
        {
            const double offset_beat = 4;
            earliest_beat = current_beat - offset_beat;
        }
        void render_meta_information(const Music& music) const
        {
            //https://siv3d.github.io/ja-jp/tutorial2/gui/?h=%E3%82%A2%E3%82%A4%E3%82%B3%E3%83%B3#283-gui-%E3%81%AB%E3%81%8A%E3%81%91%E3%82%8B%E3%82%A2%E3%82%A4%E3%82%B3%E3%83%B3%E3%81%AE%E4%BD%BF%E7%94%A8
            font_for_meta_info(
                U"{} / Key : {}"_fmt(music.get_title(), music.get_key())
            ).drawBase(
                Point{information_area.leftX() + 10, information_area.bottomY()}
            );
        }
        Optional<double> clicked_beat(){
            if (notes_area.leftClicked()){
                return earliest_beat + n_beats_in_area * (Cursor::Pos().x - notes_area.leftX())/ notes_area.w;
            }
            return none;
        }
        Optional<int> clicked_pitch(){
            throw Error(U"#TODO: Not implemented.");
        }

    public:
        ComposedViewer()                              {  }
        ComposedViewer(const Rect& arg_render_area)   { set_renderer_area(arg_render_area); }
        void set_renderer_area(const Rect& arg_render_area);
        void set_autoscroll(bool x)                     { is_autoscrolling = x; }
        Rect note_rect(const Note& note) const;

        // #NOTE renderメソッドをconst化する。
        /**
         * @brief ピアノロールを描画する
         * 
         * @param music             音楽データ
         * @param current_beat      現在の再生位置
         * @param occur_begin       今フレームで再生エフェクトを生起させるべきノートのうち、もっとも番号が若いもの
         * @param occur_end         今フレームで再生エフェクトを生起させるべきノートのうち、最も番号が大きいものの次のイテレーター。
         */
        void render(const Composed& composed) const;
        void update(Composed& composed, bool stopper_enabled);
        void reset_scroll(){
            earliest_beat = 0;
        }
        double left_bar() const     { return earliest_beat / 4; }
        double right_bar() const    { return latest_beat() / 4; }
};
