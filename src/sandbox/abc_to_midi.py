from music21 import converter

abc_score = converter.parse("./src/sandbox/example.abc", format="abc")
abc_score.write('midi', './src/sandbox/example.midi')
