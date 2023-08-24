# include "ABCParser.hpp"

smf::MidiFile ABCParser::parse(const String& abc_notation)
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