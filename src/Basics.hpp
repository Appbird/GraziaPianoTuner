# pragma once
# include <Siv3D.hpp>
# include <cassert>

/** @brief ピアノロールのノーツに対応するクラス */
struct Note{
    private:
        void validate() const{
            assert(note_number >= 0);
            assert(start_beats >= 0);
            assert(duration_beats >= 0);
        }
    public:
        int note_number;        /** MIDIノートナンバー */
        double start_beats;     /** 曲の何拍目に音が生起するか */
        double duration_beats;  /** この音が何拍ほど鳴り続けるか */
        Note(int arg_note_nubmer, int arg_start_ticks, int arg_duration_tick, double quater_note_per_ticks):
            note_number(arg_note_nubmer),
            start_beats(arg_start_ticks * quater_note_per_ticks),
            duration_beats(arg_duration_tick * quater_note_per_ticks)
        {
            validate();
        }
        Note(int arg_note_nubmer, int arg_start_beat, int arg_duration_beat):
            note_number(arg_note_nubmer),
            start_beats(arg_start_beat),
            duration_beats(arg_duration_beat)
        {
            validate();
        }
};

using NoteIterator = Array<Note>::const_iterator;

struct TempoEvent{
    private:
        void validate() const{
            assert(beats_per_seconds >= 0);
            assert(start_beats >= 0);
        }
    public:
        double start_beats;     /** このテンポイベントが何拍目に正規するか */
        double start_seconds;   /** 曲の開始から何秒でこのイベントが生起するか。拍数から秒数の変換に使用 */
        double beats_per_seconds;   /** BPS。BPMではない。 */
        TempoEvent(double arg_beats_per_seconds, int arg_start_ticks, double arg_start_seconds, double quater_note_per_ticks):
            start_beats(arg_start_ticks * quater_note_per_ticks),
            start_seconds(arg_start_seconds),
            beats_per_seconds(arg_beats_per_seconds)
        {
            validate();
        }
        TempoEvent(double arg_beats_per_seconds, int arg_start_beat, int arg_start_seconds):
            start_beats(arg_start_beat),
            start_seconds(arg_start_seconds),
            beats_per_seconds(arg_beats_per_seconds)
        {
            validate();
        }
};