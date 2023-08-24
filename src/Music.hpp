# pragma once
# include <Siv3D.hpp>
# include <MidiFile.h>

# include "Basics.hpp"
# include "DebugTools.hpp"
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
        Array<Note> notes;
        Array<TempoEvent> tempo_events;
        void prepare_melody_notes();
        void prepare_tempo_information();
        void prepare_music_length();
    public:
        Music(){}
        Music(smf::MidiFile arg_midi):
            midi(arg_midi)
        {
            assert(midi.getTrackCount() >= 2);
            quarternote_per_ticks = 1.0 / midi.getTicksPerQuarterNote();
            prepare_melody_notes();
            prepare_tempo_information();
            prepare_music_length();
            is_ready = true;
        }
        const Array<Note>& get_notes() const { return notes; }
        /**
         * @brief `current_beat`拍目におけるBPSを返す。
         */
        double get_beats_per_seconds(const double current_beat) const;
        /** 
         * @brief `current_beat`拍目に演奏されたノーツのうち、最小のものを求める。
        */
        Array<Note>::const_iterator get_next_note_index(const double current_beat) const;
        /**
         * @brief 拍数を秒数に変換する。
         */
        double beats_to_seconds(const double beats) const;
        
        explicit operator bool() const  { return is_ready; }
        bool operator!() const          { return not is_ready; }
        
        double get_music_length_in_beat()   {
            assert(is_ready);
            return music_length_in_beat;
        }
        
};