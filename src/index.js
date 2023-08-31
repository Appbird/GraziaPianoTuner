"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const abcjs_1 = require("abcjs");
const promises_1 = require("fs/promises");
const process_1 = require("process");
const midi_1 = require("@tonejs/midi");
const strict_1 = __importDefault(require("node:assert/strict"));
function is_array_of_Uint8Array(x) {
    return Array.isArray(x) && x.every(t => t instanceof Uint8Array);
}
function modify_midi(midi_raw) {
    const midi = new midi_1.Midi(midi_raw);
    if (midi.tracks.length <= 2) {
        return midi_raw;
    }
    strict_1.default.ok(midi.tracks.length > 2, `midi.tracks.length ${midi.tracks.length} <= 2`);
    const melody_track = midi.tracks[1];
    const chord_track = midi.tracks[midi.tracks.length - 1];
    melody_track.name = "melody_track";
    chord_track.name = "chord_track";
    //arrange_chord_notes(chord_track);
    return midi.toArray();
}
function arrange_chord_notes(chord_track) {
    (0, strict_1.default)(chord_track.notes.length > 0, `chord_track.notes.length(= ${chord_track.notes.length}) > 0`);
    const beat_tick = 480 * 1.5;
    let root_note = chord_track.notes[0];
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
            }
            else {
                note.ticks = root_note.ticks;
                note.durationTicks = beat_tick;
            }
        }
        else {
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
function abc2midi(abc) {
    return __awaiter(this, void 0, void 0, function* () {
        const option = {
            midiOutputType: "binary"
        };
        console.log(abc);
        const midi = abcjs_1.synth.getMidiFile(abc, option);
        if (!is_array_of_Uint8Array(midi)) {
            throw new TypeError("The result from abcjs is not an instance of Uint8Array[].");
        }
        return midi;
    });
}
function main() {
    return __awaiter(this, void 0, void 0, function* () {
        strict_1.default.ok(process_1.argv.length == 3, `argv.length is not 3 but ${process_1.argv.length}.`);
        const in_path = process_1.argv[2];
        const abc_text = yield (0, promises_1.readFile)(in_path, { encoding: "utf-8" });
        const midi = (yield abc2midi(abc_text))[0];
        const revised_midi = modify_midi(midi);
        (0, promises_1.writeFile)(in_path + ".out", revised_midi);
    });
}
main().catch((error) => { console.error(error); (0, process_1.exit)(1); });
