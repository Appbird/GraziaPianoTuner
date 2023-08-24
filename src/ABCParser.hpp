# pragma once
# include <Siv3D.hpp>
# include <MidiFile.h>
# include <stdlib.h>

/**
 * @brief ABC記譜法で与えられた楽譜をMIDIに変換するためのパーサー。
 * 出力を安定させるため、abcjsで実装されているMIDI変換エンジンを使用する。
 * そのため、外部のNode.jsプログラムを起動している。
 */
class ABCParser{
        Audio audio;
    public:
        smf::MidiFile parse(const String& abc_notation);
        
        Audio& get_audio(){
            assert(audio);
            return audio;
        }
};