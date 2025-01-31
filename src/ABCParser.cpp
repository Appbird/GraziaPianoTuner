# include <regex>
# include <iostream>
# include <memory>
# include <cstdio>
# include <cstdlib>
# include <string>

# include "ABCParser.hpp"
# include "DebugTools.hpp"

static void abc2midi(const String& in_abc, const String& out_midi);
static void abcjs(const String& in_abc);



//複数個該当するヘッダがあった場合は最後のものを採用する。
static String get_title_from_str(const std::string& abc_notation_in_str){
    std::regex title_reg{"T:\\s*([^\\n]+)\\n"};
    std::smatch match;
    std::regex_search(abc_notation_in_str, match, title_reg);
    assert(match.size() > 0);
    return Unicode::FromUTF8(match.str(match.size() - 1));
    
}
static String get_key_from_str(const std::string& abc_notation_in_str){
    std::regex key_reg{"K:\\s*([^\\n]+)\\n"};
    std::smatch match;
    std::regex_search(abc_notation_in_str, match, key_reg);
    snap(match.size());
    assert(match.size() > 0);
    return Unicode::FromUTF8(match.str(match.size() - 1));   
}
/**
 * @brief ABC記譜法で表された楽譜`str`から、コード進行の情報を抽出する。
 */
static Array<ChordEvent> get_chord_progression_from_str(const std::string& str){
    Array<ChordEvent> chord_info;
    bool at_new_line = true;
    int start_index = 0;
    // #NOTE 正規表現に書き換えたい
    for (const int i :step((int)str.size())){
        if (not at_new_line and str[i] == '\n') { at_new_line = true; continue; }
        if (at_new_line and (str[i] == '|' or str[i] == '"')) {
            start_index = i;
            break;
        }
    }
    snap(start_index);
    assert(start_index != 0);

    int section = 0; // 小節
    bool in_chord = false;
    bool in_comment = false;
    ChordEvent info{ U"", 0 };
    for (const int i :Range(start_index, (int)str.size()-1)){
        // 行頭の文字であった場合
        if (at_new_line) {
            // |が先頭文字であれば楽譜は続いている。そうでなければ読み込みを終了する。
            if (str[i] == '|')      { at_new_line = false; }
            else if (str[i] == '"') { at_new_line = false; in_chord = true; }
            else { at_new_line = false; in_comment = true; }
        } else {
            // 改行文字にあたった場合、行頭に切り替える。
            if (str[i] == '\n') { at_new_line = true; in_chord = false; continue; }
            if (in_comment) { in_comment = false; continue; }
            // 小節区切り文字にあたれば、小節を区切る。
            if (str[i] == '|')  { section += 1; }
            // 文字"にあたれば、ギターコード内への入出を切り替える。
            // ギターコード部とは、"C"のようにコードが表記されている部分のことを指す。
            if (str[i] == '"'){
                // コード部が終了すれば、コードリストに結果を書き込む。
                if (in_chord){
                    info.start_beats = section * 4;
                    INFO(info.chord << " " << info.start_beats);
                    chord_info << info;
                    info.chord.clear();
                }
                in_chord = not in_chord;
                continue;
            }
            // コードであれば一文字ずつ文字を記録していく。
            if (in_chord){ info.chord += str[i]; }
        }
    }
    // ここまでの処理だと、同じ拍数に異なるコードが記録されるケースがある。
    // 前の要素とbeatが重複している場合、それは二拍ごとに変化しているコード進行だといえる。
    // そのため、あとの方のbeat値を2だけ後にずらす修正を行う。
    snap(chord_info.size());
    for (const int i:Range(1, chord_info.size()-1)){
        snap(i);
        if (chord_info[i-1].start_beats == chord_info[i].start_beats){
            chord_info[i].start_beats += 2;
        }
    }

    return chord_info;
}

void ABCParser::parse(const String& abc_notation)
{
    std::string abc_notation_in_str = abc_notation.toUTF8();
    
    Console << U"[INFO] getting title, key, chord_info";
    
    // メタ情報から一部読み取る。
    title       = get_title_from_str(abc_notation_in_str);
    key         = get_key_from_str(abc_notation_in_str);
    chord_info  = get_chord_progression_from_str(abc_notation_in_str);
    // 外部のnode.jsにMIDI解析を委託
    FilePath in_abc = FileSystem::UniqueFilePath();
    
    INFO("Compiling ABC format text.");
    
    // #FIXED UTF-8 BOMだとabc2midiがなくなっていた。
    TextWriter abc_writer{ in_abc, TextEncoding::UTF8_NO_BOM };
    assert(abc_writer);
    abc_writer.write(abc_notation);
    abc_writer.close();
    
    const String out_path = in_abc + U".out";
    // abcjs(in_abc);
    abc2midi(in_abc, out_path);
    
    snap(in_abc);
    INFO("Loading Audio and MIDI.");
    SoundFont sf{ U"sf2/GeneralUser_GS_1.471/GeneralUser_GS_v1.471.sf2" };
    audio = Audio{sf.renderMIDI(out_path)};
    midi = smf::MidiFile{out_path.toUTF8()};

}

static void abc2midi(const String& in_abc, const String& out_midi) {
    String command = U"abc2midi {} -o {}"_fmt(in_abc, out_midi);
    snap(command);
    
    // popen を使って標準出力を取得
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.toUTF8().c_str(), "r"), pclose);
    if (not pipe) {
        Console << U"Failed to run abc2midi!";
        throw Error(U"abc2midiを実行できませんでした。");
    }

    std::string stdout_content;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        stdout_content += buffer;
    }

    // 標準出力にエラーや警告が含まれていないかチェック
    if (stdout_content.find("Error") != std::string::npos) {
        std::cerr << "abc2midi Error:\n" << (stdout_content) << std::endl;
        throw Error(U"abc形式のコンパイルでエラーが発生しました。");
    }
    if (stdout_content.find("Warning") != std::string::npos) {
        std::cerr << "abc2midi Warning:\n" << stdout_content << std::endl;
    }
}



static void abcjs(const String& in_abc) {
    String input_command = U"npm run app " + in_abc;
    int result = system(input_command.toUTF8().c_str());
    assert(result == 0);
}