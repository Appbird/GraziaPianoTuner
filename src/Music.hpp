# pragma once
# include <Siv3D.hpp>
# include <MidiFile.h>
# include <utility>

# include "Basics.hpp"
# include "DebugTools.hpp"
# include "ABCParser.hpp"
/**
 * @brief Midiを扱うためのクラス。ピアノロールのレンダリングや、MIDIファイルを編集するにあたって重要な機能を提供する
 */
class Music{
    private:
        //const GMInstrument instrument = GMInstrument::Piano1;
        bool is_ready = false;      /** 音楽が用意されているか */
        smf::MidiFile midi;
        String title;
        String key;
        double music_length_in_beat; /** 曲の全体拍数 */
        double quarternote_per_ticks; /** 1tickあたり拍数 */
        double m_min_pitch;
        double m_max_pitch;
        Array<Note> notes;
        Array<TempoEvent> tempo_events;
        Array<ChordEvent> chord_events;
        void prepare_melody_notes();
        void prepare_tempo_information();
        void prepare_music_length();
    public:
        const String& get_title() const{
            return title;
        }
        const String& get_key() const{
            return key;
        }
        
        Music(){}
        // #NOTE ここのメモリ使用について改善の余地あり[最適化]
        Music(
            const ABCParser& parser
        ):
            midi(parser.get_midi()),
            title(parser.get_title()),
            key(parser.get_key()),
            chord_events(parser.get_chord_event())
        {
            quarternote_per_ticks = 1.0 / midi.getTicksPerQuarterNote();
            prepare_melody_notes();
            prepare_tempo_information();
            prepare_music_length();
            is_ready = true;
        }
        bool is_chord_progression_exist() const{
            return chord_events.size() != 0;
        }
        const Array<Note>& get_notes() const { return notes; }
        const Array<ChordEvent>& get_chords_progressions() const { return chord_events; }
        /**
         * @brief `current_beat`拍目におけるBPSを返す。
         */
        double get_beats_per_seconds(const double current_beat) const;
        /** 
         * @brief `current_beat`拍目において、次に演奏されるであろうnoteを求める。
         * @return 戻り値のイテレータは必ずbeginとの距離が必ず1以上であることが保証されている。
        */
        Array<Note>::const_iterator get_next_note_index(const double current_beat) const;
        /** 
         * @brief `current_beat`拍目において、次に演奏されるであろうコードを求める。
         * @return 戻り値のイテレータは必ずbeginとの距離が1以上であることが保証されている。
        */
        Array<ChordEvent>::const_iterator get_next_chord_index(const double current_beat) const;
        /**
         * @brief 拍数を秒数に変換する。
         */
        double beats_to_seconds(const double beats) const;
        
        explicit operator bool() const  { return is_ready; }
        bool operator!() const          { return not is_ready; }
        
        double get_music_length_in_beat() const {
            assert(is_ready);
            return music_length_in_beat;
        }
        /// @brief この曲の最小, 最大音高を求める。
        std::pair<double, double> get_min_max_pitch() const {
            assert(is_ready);
            return {m_min_pitch, m_max_pitch};
        }
        
};