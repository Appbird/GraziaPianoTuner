# include "Composed.hpp"
# include "util.hpp"

void Composed::play()
{
    if (not music) { return; }
    assert(audio and music);
    is_playing = true;
    latest_notes    = music.get_next_note_index(current_beat) - 1;
    next_note       = music.get_next_note_index(current_beat);

    const double start_sec = music.beats_to_seconds(current_beat);
    audio.seekTime(start_sec);
    
    audio.play();
}
void Composed::update(){
    if (not music) { return; }
    latest_notes    = next_note;
    next_note       = music.get_next_note_index(current_beat);
    if (get_is_playing()){
        current_beat += music.get_beats_per_seconds(current_beat) * Scene::DeltaTime();
        if (current_beat >= music.get_music_length_in_beat()){
            current_beat = music.get_music_length_in_beat();
            is_playing = false;
        }
    }
}