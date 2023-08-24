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

        // OUTPUTから始まるブロックを捕捉し、そこからABC記譜法の楽譜を抽出する
        String find_last_abc_block(const String& GPT_answer)
        {
            const std::regex regex{"OUTPUT\\n```\\n([^`]+?)\\n```"};
            std::string str_utf8 = GPT_answer.toUTF8();
            std::smatch matches;
            std::regex_search(str_utf8.cbegin(), str_utf8.cend(), matches, regex);
            assert(matches.size() > 0);
            
            const std::string last_match_str = matches.str(matches.size() - 1);
            return Unicode::FromUTF8(last_match_str);
        }
        
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
        
        void play()
        {
            assert(audio and music);
            is_playing = true;
            latest_notes    = music.get_next_note_index(current_beat) - 1;
            next_note       = music.get_next_note_index(current_beat);

            const double start_sec = music.beats_to_seconds(current_beat);
            audio.seekTime(start_sec);

            audio.play();
        }
        void update(){
            if (not music) { return; }
            latest_notes    = next_note;
            next_note       = music.get_next_note_index(current_beat);

            renderer.update_autoscrool(current_beat, music.get_music_length_in_beat());
            if (is_playing){
                current_beat += music.get_beats_per_seconds(current_beat) * Scene::DeltaTime();
                if (current_beat >= music.get_music_length_in_beat()){
                    current_beat = music.get_music_length_in_beat();
                    is_playing = false;
                }
            }
        }

        void render(){
            renderer.render(music, current_beat, latest_notes, next_note);
        }
};