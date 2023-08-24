# pragma once
# include <Siv3D.hpp>
# include <regex>

# include "util.hpp"
# include "Music.hpp"
# include "ComposedRenderer.hpp"
# include "ABCParser.hpp"

class Composed{
    private:
        NoteIterator latest_notes;          /** すでに演奏された最後のノーツ */
        NoteIterator next_note; /** すでに演奏された最後のノーツ */
        Audio audio;
        Music music;
        bool is_playing = false;
        ComposedRenderer renderer;
        double current_beat = 3;

        /** 
         * @brief GPTの解答からOUTPUTから始まるコードブロックを捕捉し、そこからABC記譜法の楽譜を抽出する
         */
        String find_last_abc_block(const String& GPT_answer);
        
    public:
        Composed(){}
        Composed(const String& GPT_answer)
        {
            // #TODO Siv3Dで正規表現は利用可能か？
            // 無理でした...
            // https://siv3d.github.io/ja-jp/tutorial2/string/#1919-%E4%BB%96%E3%81%AE%E6%96%87%E5%AD%97%E5%88%97%E5%9E%8B%E3%81%B8%E5%A4%89%E6%8F%9B%E3%81%99%E3%82%8B
            String abc_score = find_last_abc_block(GPT_answer);
            ABCParser parser;
            smf::MidiFile midi = parser.parse(abc_score);
            audio = parser.get_audio();
            music = Music{midi};
        }
        explicit operator bool() { return (audio and music); }
        
        void play();
        void update();

        void render(){
            renderer.render(music, current_beat, latest_notes, next_note);
        }
};