# include <Siv3D.hpp> // OpenSiv3D v0.6.11
# include <cassert>
# include <regex>
# include <stdlib.h>
# include <MidiFile.h>


struct Note{
    private:
        void validate() const{
            assert(note_number > 0);
            assert(start_beats > 0);
            assert(duration_beats > 0);
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

/**
 * @brief ABC記譜法で与えられた楽譜をMIDIに変換する。
 * 出力を安定させるため、abcjsで実装されているエンジン
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

class Music{
    private:
        const GMInstrument instrument = GMInstrument::Piano1;
        bool is_ready = false;
        smf::MidiFile midi;
        Array<Note> notes;
    public:
        Music(){}
        Music(smf::MidiFile arg_midi):
            midi(arg_midi)
        {
            assert(midi.getTrackCount() >= 2);
            const double qst = 1.0 / midi.getTicksPerQuarterNote();
            smf::MidiEventList melody_track = midi[1];
            for (int i = 0; i < melody_track.getEventCount(); i++){
                const auto& event = melody_track[i];
                if (event.isNoteOn()){ notes << Note{event[0], event.tick, event.getTickDuration(), qst}; }
            }
            is_ready = true;
        }
        const Array<Note>& get_notes() const { return notes; }
        // #NOTE 値を返さないとOptionaでテンプレートがマッチせず、演算子=が使えない。
        Music& operator=(Music&& music){
            midi = std::move(music.midi);
            notes = std::move(music.notes);
            return *this;
        }
        Music& operator=(const Music& music){
            midi = music.midi;
            notes = music.notes;
            return *this;
        }
        explicit operator bool(){
            return is_ready;
        }
        
};



class Composed{
    private:
        const std::regex regex{"OUTPUT\\n```\\n([^`]+?)\\n```"};
        bool is_ready = false;
        String title;
        String key;
        Audio audio;
        smf::MidiFile midi;
        Music music;
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
            key = std::move(composed.key); title = std::move(composed.title);
            audio = std::move(composed.audio);
            // #NOTE なぜかOptionalを使うと怒られる。
            music = std::move(composed.music);
        }
        
        void play()
        {
            assert(audio);
            audio.playOneShot();
        }

        void render(){
            const Rect rendered_area{0, 200, Scene::Size().x, 200};
            const double n_beats_in_area = 16; const double n_halfpitch_in_area = 24; 
            
            const int lowest_pitch = 60; const int ealriest_beat = 0;
            const int highest_pitch = lowest_pitch + n_halfpitch_in_area;
            const int latest_beat = ealriest_beat + n_beats_in_area;
            
            const int width_per_beat    = rendered_area.w / n_beats_in_area;
            const int height_per_pitch  = rendered_area.h / n_halfpitch_in_area;

            assert(music);
            for (const Note& note:music.get_notes()){
                // ノートの開始拍数
                // q = quater note(一拍)
                const int end_beat = note.start_beats + note.duration_beats;
                if (end_beat < ealriest_beat or latest_beat < note.start_beats) { continue; }
                if (note.note_number < lowest_pitch or highest_pitch < note.note_number){ continue; }
                
                const Size note_offsetVec2{
                    (int)round((note.duration_beats - ealriest_beat) * width_per_beat),
                    (int)round((highest_pitch - note.note_number) * height_per_pitch)
                };
                const Size note_size{
                    (int)round(note.duration_beats * width_per_beat),
                    (int)round(1 * height_per_pitch)
                };
                Rect{rendered_area.tl() + note_offsetVec2, note_size}.draw(ColorF(1,1,1));
            }

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
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });

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
        
        if (answer){
            answer.render();
        }
	}
}
