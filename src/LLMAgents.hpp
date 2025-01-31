# pragma once
# include "Siv3D.hpp"
# include "MusicalGPT4.hpp"

class LLMAgents {
    MusicalGPT4 leader              {U"prompts/leader.txt"};
    MusicalGPT4 chord_agent         {U"prompts/chord_agent.txt"};
    MusicalGPT4 melody_agent        {U"prompts/melody_agent.txt"};
    MusicalGPT4 instruments_agent   {U"prompts/instruments_agent.txt"};
    Optional<String> answer_including_score = none;
    String model;
    enum class State {
        Idle,
        AskToLeader,
        AskToChordAgent,
        AskToMelodyAgent,
        AskToInstrumentAgent,
        WaitForTakingAnswer
    };
    State agents_state = State::Idle;

    void update() {
        if (const auto answer = leader.try_to_get_answer()) {
            chord_agent.tell(*answer);
            melody_agent.tell(*answer);
            instruments_agent.tell(*answer);

            chord_agent.request();
            agents_state = State::AskToChordAgent;
        }
        else if (const auto answer = chord_agent.try_to_get_answer()) {
            melody_agent.request(*answer);
            agents_state = State::AskToMelodyAgent;
        }
        else if (const auto answer = melody_agent.try_to_get_answer()) {
            instruments_agent.request(*answer);
            agents_state = State::AskToInstrumentAgent;
        }
        else if (const auto answer = instruments_agent.try_to_get_answer()) {
            answer_including_score = answer;
            agents_state = State::WaitForTakingAnswer;
        }
    }

public:
    LLMAgents(const String& model) {
        this->model = model;
        leader.model            = model;
        chord_agent.model       = model;
        melody_agent.model      = model;
        instruments_agent.model = model;
    }
    LLMAgents(): LLMAgents(U"gpt-4o") {}
    StringView model_name() { return model; }
    void request(const String& prompt) {
        leader.request(prompt);
        chord_agent.tell(prompt);
        melody_agent.tell(prompt);
        instruments_agent.tell(prompt);
        agents_state = State::AskToLeader;
    }
    Optional<String> result() { 
        try {
            update();
            if (answer_including_score) { agents_state = State::Idle; }
            
        } catch (Error e) {
            INFO(e);
        }
        return std::exchange(answer_including_score, none);
    }
    bool is_downloading() { 
        return agents_state != State::Idle;
    }
    StringView state_message() {
        switch (agents_state)
        {
            case State::Idle : return U"";
            case State::AskToLeader : return U"曲の構成を考えています";
            case State::AskToChordAgent : return U"コード進行をつくっています";
            case State::AskToMelodyAgent : return U"メロディをつくっています";
            case State::AskToInstrumentAgent : return U"楽器の編成を決めています";
            case State::WaitForTakingAnswer : return U"楽譜をつくっています";
        }
    }
    void remember(const JSON& snapshots) {
        leader.remember(snapshots[U"leader"]);
        chord_agent.remember(snapshots[U"chord"]);
        melody_agent.remember(snapshots[U"melody"]);
        instruments_agent.remember(snapshots[U"instruments"]);
    }
    JSON snapshot() {
        JSON json;
        json[U"leader"] = leader.snapshot();
        json[U"chord"] = chord_agent.snapshot();
        json[U"melody"] = melody_agent.snapshot();
        json[U"instruments"] = instruments_agent.snapshot();
        return json;
    }
};