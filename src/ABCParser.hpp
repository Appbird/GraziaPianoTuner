# pragma once
# include <Siv3D.hpp>
# include <MidiFile.h>
# include <stdlib.h>
# include "Basics.hpp"

/**
 * @brief ABC記譜法で与えられた楽譜をMIDIに変換するためのパーサー。
 * 出力を安定させるため、abcjsで実装されているMIDI変換エンジンを使用する。
 * そのため、外部のNode.jsプログラムを起動している。
 */
class ABCParser{
        Audio audio;
        String title;
        String key;
        Array<ChordEvent> chord_info;
        smf::MidiFile midi;
    public:
        void parse(const String& abc_notation);
        const String& get_title() const{
            return title;
        }
        const String& get_key() const{
            return key;
        }
        const Array<ChordEvent>& get_chord_event() const{
            return chord_info;
        }
        const smf::MidiFile& get_midi() const{
            return midi;
        }
        Audio& get_audio(){
            assert(audio);
            return audio;
        }
};