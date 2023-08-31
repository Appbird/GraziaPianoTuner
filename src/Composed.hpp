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
        String abc_score;
        bool is_playing = false;
        double current_beat = 0;
        
    public:
        Composed(){}
        explicit operator bool() { return (audio and music); }
        void set_answer(const String& GPT_answer){
            if (audio and music) {stop(); seek(0);}
            abc_score = find_last_abc_block(GPT_answer);
            ABCParser parser;
            parser.parse(abc_score);
            audio = parser.get_audio();
            music = Music{parser};
            latest_notes = music.get_next_note_index(current_beat) - 1;
            next_note = music.get_next_note_index(current_beat);
        }
        const String& get_title(){
            return music.get_title();
        }
        bool get_is_playing(){ return is_playing; }
        void play();
        void update();
        double get_current_beat() const{
            return current_beat;
        }
        std::pair<NoteIterator, NoteIterator> get_passing_notes() const{
            return {latest_notes, next_note};
        }
        const Music& get_music() const{
            return music;
        }
        void seek(double beats){
            current_beat = beats;
            if (is_playing) { play(); }
        }
        void stop(){
            is_playing = false;
            audio.stop();
        }
        String get_abc_score(){
            return abc_score;
        }
};