# include "Music.hpp"

void Music::prepare_melody_notes(){
    // メロディパートの解析
    smf::MidiEventList& melody_track = midi[1];    
    assert(melody_track.getEventCount() > 0);
    // #最初のNOTE_ONメッセージをとってくるのがめんどくさいので、適当に初期化
    m_min_pitch = 999;
    m_max_pitch = 0;
    midi.linkNotePairs();
    for (int i = 0; i < melody_track.getEventCount(); i++){
        const auto& event = melody_track[i];
        if (event.isNoteOn()){
            const int pitch = event[1];
            if (m_min_pitch > pitch) { m_min_pitch = pitch; }
            if (m_max_pitch < pitch) { m_max_pitch = pitch; }
            notes << Note{pitch, event.tick, int(event.getTickDuration()), quarternote_per_ticks};
        }
    }
}

void Music::prepare_tempo_information(){
    // テンポの解析
    // #NOTE なぜかdoTimeAnalysisをするとTempoメタメッセージが消えるので、TimeAnalysis専用の領域を確保
    // ta = time analysis
    smf::MidiFile midi_for_ta = midi;
    midi_for_ta.doTimeAnalysis();
    const smf::MidiEventList& tempo_track_ta    = midi_for_ta[0];
    const smf::MidiEventList& tempo_track       = midi[0];
    assert(tempo_track.getEventCount() == tempo_track_ta.getEventCount());
    for (const int i : step(tempo_track.getEventCount())){
        const smf::MidiEvent& event = tempo_track[i];
        const double time = tempo_track_ta[i].seconds;
        if (event.isTempo()){
            INFO("find tempo message");
            snap(event.getTempoBPM());
            tempo_events << TempoEvent{1 / event.getTempoSeconds(), event.tick, time, quarternote_per_ticks};
        }
    }
}

void Music::prepare_music_length(){
    // 曲の長さを求める。
    // #NOTE これMidiFileの内部に保存されてそうなんだけどな〜〜〜！？
    smf::MidiFile midi_tmp = midi;
    midi_tmp.joinTracks();
    assert(midi_tmp[0].back().isEndOfTrack());
    music_length_in_beat = midi_tmp[0].back().tick * quarternote_per_ticks;
}

double Music::get_beats_per_seconds(const double current_beat) const {
    assert(is_ready);
    assert(tempo_events.size() > 0);
    // current_beat以下のはじめの要素を見つける
    const auto target = std::partition_point(
        tempo_events.begin(),
        tempo_events.end(),
        [&](const TempoEvent& a){ return a.start_beats <= current_beat; }
    );
    assert(std::distance(tempo_events.begin(), target) > 0);
    return (target - 1)->beats_per_seconds;
}


Array<Note>::const_iterator Music::get_next_note_index(const double current_beat) const {
    assert(is_ready);
    assert(notes.size() > 0);
    // current_beat以下のはじめの要素を見つける
    const auto target = std::partition_point(
        notes.begin(),
        notes.end(),
        [&](const Note& a){ return a.start_beats <= current_beat; }
    );
    assert(std::distance(notes.begin(), target) > 0);
    return target;
}
Array<ChordEvent>::const_iterator Music::get_next_chord_index(const double current_beat) const {
    assert(is_ready);
    assert(chord_events.size() > 0);
    // current_beat以下のはじめの要素を見つける
    const auto target = std::partition_point(
        chord_events.begin(),
        chord_events.end(),
        [&](const ChordEvent& a){ return a.start_beats <= current_beat; }
    );
    assert(std::distance(chord_events.begin(), target) > 0);
    return target;
}


double Music::beats_to_seconds(const double beats) const {
    assert(is_ready);
    assert(tempo_events.size() > 0);
    const auto last_tempo_track = std::partition_point(
        tempo_events.begin(),
        tempo_events.end(),
        [&](const TempoEvent& a){ return a.start_beats <= beats; }
    ) - 1;
    assert(std::distance(tempo_events.begin(), last_tempo_track + 1) > 0);
    assert(last_tempo_track != tempo_events.end());
    const double remainder_beats = beats - last_tempo_track->start_seconds;
    return remainder_beats / last_tempo_track->beats_per_seconds;
}

