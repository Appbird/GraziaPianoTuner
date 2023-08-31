import { MidiFileOptions, synth } from "abcjs"
import { readFile, writeFile } from "fs/promises"
import { argv, exit } from "process";
import { Midi, Track } from "@tonejs/midi"
import { Note } from "@tonejs/midi/dist/Note";
import assert from "node:assert/strict"


function is_array_of_Uint8Array(x:unknown):x is Uint8Array[]{
    return Array.isArray(x) && x.every(t => t instanceof Uint8Array);
}

function modify_midi(midi_raw:Uint8Array):Uint8Array{
    const midi = new Midi(midi_raw)
    if (midi.tracks.length <= 2) { return midi_raw; }
    assert.ok(midi.tracks.length > 2, `midi.tracks.length ${midi.tracks.length} <= 2`);
    const melody_track = midi.tracks[1];
    const chord_track = midi.tracks[midi.tracks.length - 1];
    melody_track.name = "melody_track";
    chord_track.name = "chord_track";
    
    
    //arrange_chord_notes(chord_track);
    return midi.toArray();
}

function arrange_chord_notes(chord_track:Track) {
    assert(chord_track.notes.length > 0, `chord_track.notes.length(= ${chord_track.notes.length}) > 0`);

    const beat_tick = 480 * 1.5;
    let root_note: Note = chord_track.notes[0];
    root_note.durationTicks = beat_tick;
    let chord_ticks = -1;
    let searching_root = false;
    let on_first_note = true;
    for (let note of chord_track.notes) {
        if (on_first_note) {
            chord_ticks = note.durationTicks;
            on_first_note = false;
            continue;
        }
        // 構成音を探索中
        if (searching_root) {
            // 異なる時刻になるルート音を見つけた場合
            if (note.ticks > chord_ticks) {
                root_note = note;
                note.durationTicks = beat_tick;
                searching_root = false;
                // 引き続き同じ時刻になるノーツが見つかった場合
            } else {
                note.ticks = root_note.ticks;
                note.durationTicks = beat_tick;
            }
        } else {
            // ルート音に引き続いてコード音を見つけた時
            if (note.ticks !== root_note.ticks) {
                chord_ticks = note.ticks;
                note.ticks = root_note.ticks;
                note.durationTicks = beat_tick;
                searching_root = true;
            }
        }
    }
}

async function abc2midi(abc:string):Promise<Uint8Array[]>{
    const option:MidiFileOptions = {
        midiOutputType: "binary"
    };
    console.log(abc);
    const midi = synth.getMidiFile(abc, option)
    if (!is_array_of_Uint8Array(midi)){
        throw new TypeError("The result from abcjs is not an instance of Uint8Array[].")
    }
    return midi;
}



async function main(){
    assert.ok(argv.length == 3, `argv.length is not 3 but ${argv.length}.`);
    const in_path = argv[2];
    const abc_text = await readFile(in_path, {encoding:"utf-8"})
    const midi = (await abc2midi(abc_text))[0]
    const revised_midi = modify_midi(midi);
    writeFile(in_path + ".out", revised_midi);
}

main().catch((error:Error) => {console.error(error); exit(1); })