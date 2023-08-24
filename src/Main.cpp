# include <algorithm>
# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include <cassert>
# include <regex>
# include <stdlib.h>
# include <MidiFile.h>
#define DEBUG_PART(x) x
#define INFO(x) DEBUG_PART(std::cerr << x << std::endl)
#define snap(var) DEBUG_PART(std::cerr << "[snap] " << #var << " = " << (var) << std::endl)
#define snap_msg(msg, var) DEBUG_PART(std::cerr << "[snap:" << (msg) << "] " << #var << " = " << (var) << std::endl)

constexpr HSV main_color{210, 0.4, 0.2};

/// ------------------------------
///  DEBUG
/// ------------------------------

std::ostream& operator<<(std::ostream& os, Rect r){
    return os << "(" << r.x << r.y << ") ~ (" << r.br().x << ", " << r.br().y << ")";
}

/// ------------------------------
///  Implementation
/// ------------------------------

struct Note{
    private:
        void validate() const{
            assert(note_number >= 0);
            assert(start_beats >= 0);
            assert(duration_beats >= 0);
        }
    public:
        int note_number;
        double start_beats;
        double duration_beats;
        Note(int arg_note_nubmer, int arg_start_ticks, int arg_duration_tick, double quater_note_per_ticks):
            note_number(arg_note_nubmer),
            start_beats(arg_start_ticks * quater_note_per_ticks),
            duration_beats(arg_duration_tick * quater_note_per_ticks)
        {
            validate();
        }
};
struct TempoEvent{
    private:
        void validate() const{
            assert(beats_per_seconds >= 0);
            assert(start_beats >= 0);
        }
    public:
        double start_beats;
        double beats_per_seconds;
        TempoEvent(double arg_beats_per_seconds, int arg_start_ticks, double quater_note_per_ticks):
            start_beats(arg_start_ticks * quater_note_per_ticks),
            beats_per_seconds(arg_beats_per_seconds)
        {
            validate();
        }
        TempoEvent(double arg_beats_per_seconds, int arg_start_beat):
            start_beats(arg_start_beat),
            beats_per_seconds(arg_beats_per_seconds)
        {
            validate();
        }
};

/**
 * @brief ABC記譜法で与えられた楽譜をMIDIに変換する。
 * 出力を安定させるため、abcjsで実装されているMIDI変換エンジンを使用する。
 * そのため、外部のNode.jsプログラムを一旦生成させている。
 */
class ABCParser{
        Audio audio;
    public:
        smf::MidiFile parse(const String& abc_notation)
        {
            FilePath in_abc = FileSystem::UniqueFilePath();
            
            TextWriter abc_writer{ in_abc };
            assert(abc_writer);
            abc_writer.write(abc_notation);
            abc_writer.close();
            
            String input_command = U"npm run app " + in_abc;
            int result = system(input_command.toUTF8().c_str());
            assert(result == 0);
            const String out_path = in_abc + U".out";
            audio = Audio{out_path};
            return smf::MidiFile{out_path.toUTF8()};
        }
        Audio& get_audio(){
            assert(audio);
            return audio;
        }
};

/**
 * @brief 
 * 
 */
class Music{
    private:
        //const GMInstrument instrument = GMInstrument::Piano1;
        bool is_ready = false;
        smf::MidiFile midi;
        double music_length_in_beat;
        double quarternote_per_ticks;
        Array<Note> notes;
        Array<TempoEvent> tempo_events;


    public:
        Music(){}
        Music(smf::MidiFile arg_midi):
            midi(arg_midi)
        {
            assert(midi.getTrackCount() >= 2);
            quarternote_per_ticks = 1.0 / midi.getTicksPerQuarterNote();
            // メロディパートの解析
            smf::MidiEventList& melody_track = midi[1];
            midi.linkNotePairs();
            for (int i = 0; i < melody_track.getEventCount(); i++){
                const auto& event = melody_track[i];
                if (event.isNoteOn()){
                    notes << Note{event[1], event.tick, int(event.getTickDuration()), quarternote_per_ticks};
                }
            }
            // テンポの解析
            smf::MidiEventList& tempo_track = midi[0];
            for (const int i : step(tempo_track.getEventCount())){
                const smf::MidiEvent& event = tempo_track[i];
                if (event.isTempo()){
                    INFO("find tempo message");
                    snap(event.getTempoBPM());
                    tempo_events << TempoEvent{1 / event.getTempoSeconds(), event.tick, quarternote_per_ticks};
                }
            }
            // 曲の長さを求める。
            // #NOTE これMidiFileの内部に保存されてそうなんだけどな〜〜〜！？
            smf::MidiFile midi_tmp = midi;
            midi_tmp.joinTracks();
            assert(midi_tmp[0].back().isEndOfTrack());
            music_length_in_beat = midi_tmp[0].back().tick * quarternote_per_ticks;
            is_ready = true;
        }
        const Array<Note>& get_notes() const { return notes; }
        double get_beats_per_seconds(const double current_beat) const {
            assert(tempo_events.size() > 0);
            constexpr int ARBITARY = 0;
            // current_beat以下のはじめの要素を見つける
            const auto target = std::lower_bound(
                tempo_events.begin(),
                tempo_events.end(),
                TempoEvent{current_beat, ARBITARY},
                [](const TempoEvent& a, const TempoEvent& b){ return a.start_beats > b.start_beats; }
            );
            if (target == tempo_events.end()) { assert(false); }
            return target->beats_per_seconds;
        }
        
        explicit operator bool() const{
            return is_ready;
        }
        bool operator!() const{
            return not is_ready;
        }
        double get_max_beat(){
            return music_length_in_beat;
        }
        
};

class ComposedRenderer{
    private:
        double n_beats_in_area = 12;
        double n_halfpitch_in_area = 24; 
        double lowest_pitch = 70;
        double ealriest_beat = 0;
        Rect rendered_area{0, 200, Scene::Size().x, 300};
        double highest_pitch()      { return lowest_pitch + n_halfpitch_in_area; }
        double latest_beat()        { return ealriest_beat + n_beats_in_area; }
        double width_per_beat()     { return rendered_area.w / n_beats_in_area; }
        double height_per_pitch()   { return rendered_area.h / n_halfpitch_in_area; }
        
        void render_pitch(){
            for (int b = 0; b < n_beats_in_area; b++){
                Size offset{int(this->width_per_beat() * b), 0};
                const int thickness = (b % 4 == 0) ? 3 : 2;
                const double color_value = (b % 4 == 0) ? 0.3 : 0.15;
                Line{
                    rendered_area.tl() + offset,
                    rendered_area.bl() + offset
                }.draw(thickness, HSV{0, 0, color_value});
            }
        }
        void render_beat(){
            for (int p = 0; p < n_halfpitch_in_area; p++){
                Size offset{0, int(this->height_per_pitch() * p)};
                Line{
                    rendered_area.tl() + offset,
                    rendered_area.tr() + offset
                }.draw(2,HSV{0, 0, 0.15});
            }
        }
        void render_current_position(const double current_position){
            if (not (InRange(current_position, ealriest_beat, latest_beat()))){ return; }
            Size offset{int(this->width_per_beat() * (current_position - ealriest_beat) ), 0};
            Line{
                rendered_area.tl() + offset,
                rendered_area.bl() + offset
            }.draw(5, HSV{200, 0.05, 0.9});
            
        }
        void render_frame(){
            rendered_area
            .drawFrame(5, HSV{main_color.h, main_color.s * 0.3, main_color.v * 1.5})
            .draw(HSV{main_color.h, 0, main_color.v * 0.3});
        }
        void render_notes(const Music& music){
            assert(music);
            for (const Note& note:music.get_notes()){
                // ノートの開始拍数
                // q = quater note(一拍)
                const int end_beat = note.start_beats + note.duration_beats;
                
                if (end_beat < ealriest_beat or latest_beat() < note.start_beats) { continue; }
                if (not InRange(double(note.note_number), lowest_pitch, highest_pitch())) { continue; }
                
                const Size note_offsetVec2{
                    (int)round((note.start_beats - ealriest_beat) * width_per_beat()),
                    (int)round((highest_pitch() - note.note_number) * height_per_pitch())
                };
                const Size note_size{
                    (int)round(note.duration_beats * width_per_beat()),
                    (int)round(1 * height_per_pitch())
                };
                Rect{rendered_area.tl() + note_offsetVec2 + Size(2, 2), note_size - Size(2, 2)}
                .drawFrame(2.0, HSV{main_color.h, main_color.s, 0.5})
                .draw(HSV{0, 0, 1});
            }
        }
    public:
        void render(
            const Music& music,
            const double current_beat
        ){
            render_frame();
            render_pitch();
            render_beat();
            render_current_position(current_beat);
            if (music) {render_notes(music); }
        }
        void update_autoscrool(const double current_beat, const double max_beat){
            ealriest_beat = Clamp(current_beat - 4, 0, max_beat) * width_per_beat();
        }
};


class Composed{
    private:
        const std::regex regex{"OUTPUT\\n```\\n([^`]+?)\\n```"};
        String title;
        String key;
        Audio audio;
        Music music;
        ComposedRenderer renderer;
        double current_beat;

        // ./Appを実行している
        String find_last_abc_block(const String& GPT_answer)
        {
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
        void operator=(Composed&& composed)
        {
            key = std::move(composed.key);
            title = std::move(composed.title);
            audio = std::move(composed.audio);
            // #NOTE なぜかOptionalを使うと怒られる。
            music = std::move(composed.music);
        }
        
        void play()
        {
            assert(audio);
            audio.playOneShot();
        }
        void update(){
            if (not music) { return; }
            current_beat += music.get_beats_per_seconds(current_beat) * Scene::DeltaTime();
            renderer.update_autoscrool(current_beat);
        }

        void render(){
            renderer.render(music, current_beat);
        }
};

class MusicalGPT4{
    private:
        String GPT_API_AKY;
        String construct_prompt(String request){
            return request;
        }
    public:
        MusicalGPT4(String arg_GPT_API_AKY):
            GPT_API_AKY(arg_GPT_API_AKY)
        {}
        Composed request(String requset){
            // const String prompt = construct_prompt(request);
            // OpenAI::Chat::Complete(API_KEY, prompt);
            return Composed(U"生成しました。次のとおりです。\n```\nX:3\nT: false score \nM:4/4\nL:1/4\nK:D\nQ:1/4 = 225\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"Aade|\"Em7\"fz3|\"A7\"z4|\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"A2\"F#sus4\"c2|\"Dmaj\"d4|z4|\n```\nOUTPUT\n```\nX:3\nT: Browned Bread\nM:4/4\nL:1/4\nK:D\nQ:1/4 = 225\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"Aade|\"Em7\"fz3|\"A7\"z4|\n|\"Dmaj\"fefg|\"Amaj\"f2 e2|\"Bm\"edef|\"Amaj\"e2d2|\n|\"Gmaj\"dcBF|\"F#m\"A2\"F#sus4\"c2|\"Dmaj\"d4|z4|\n```");
        }
};

void Main()
{
	Scene::SetBackground(HSV{210, 0.4, 0.2});

	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	// 環境変数から API キーを取得する
	const String API_KEY = U"SECRET_KEY"; // EnvironmentVariable::Get(U"MY_OPENAI_API_KEY");

	// テキストボックスの中身
	TextEditState textEditState;

	// 回答を格納する変数
	Composed answer;
    MusicalGPT4 musicalGPT4(API_KEY);

	while (System::Update())
	{
		// テキストボックスを表示する
		SimpleGUI::TextBox(textEditState, Vec2{ 40, 40 }, 600);

		if (SimpleGUI::Button(U"送信", Vec2{ 660, 40 }, 80, (not textEditState.text.isEmpty()))){ 
			const String input = textEditState.text;
			answer = musicalGPT4.request(U"create a shiny piano music.");
            answer.play();
        }
        answer.update();
        answer.render();
	}
}
