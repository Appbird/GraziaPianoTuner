# pragma once
# include <Siv3D.hpp>
# include "Composed.hpp"
# include "Layout.hpp"
# include "EmotionalController.hpp"
# include "HarmonicGuide.hpp"

class HistoryViewer
{
    public:
        struct Snapshot{
            String request;
            String answer;
            Optional<EmotionalController::EmotionalParameters> emotional_params;
            Optional<HarmonicGuide::GuideParameter> guide_params;
            Snapshot(){}
            Snapshot(
                const String& arg_request,
                const String& arg_answer,
                const EmotionalController::EmotionalParameters& arg_params
            ):
                request(arg_request),
                answer(arg_answer),
                emotional_params(arg_params)
            {}
            Snapshot(
                const String& arg_request,
                const String& arg_answer,
                const HarmonicGuide::GuideParameter& arg_params
            ):
                request(arg_request),
                answer(arg_answer),
                guide_params(arg_params)
            {}
            JSON encode() const {
                JSON result;
                result[U"request"] = request;
                result[U"answer"] = answer;
                if (emotional_params) {
                    result[U"emotional_params"] = emotional_params->encode();
                }
                if (guide_params) {
                    result[U"guide_params"] = guide_params->encode();
                }
                return result;
            }
            static Snapshot decode(const JSON& json){
                assert(json[U"request"].isString());
                assert(json[U"answer"].isString());
                if (json.hasElement(U"emotional_params")) {
                    return {
                        json[U"request"].getString(),
                        json[U"answer"].getString(),
                        EmotionalController::EmotionalParameters::decode(json[U"emotional_params"])
                    };
                }
                else if (json.hasElement(U"guide_params")) {
                    return {
                        json[U"request"].getString(),
                        json[U"answer"].getString(),
                        HarmonicGuide::GuideParameter::decode(json[U"guide_params"])
                    };
                }
                else {
                    assert(0);
                }
            }
        };
    private:
        Rect render_area;
        Rect left_button_area;
        Rect index_label_area;
        Rect right_button_area;
        bool m_is_page_refreshed = false;
        
        Snapshot stashed;
        Array<Snapshot> snapshots;
        Font font{30, FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"03スマートフォントUI.otf", FontStyle::Bold};
        
    public:
        int current_page = 0;
        HistoryViewer(){}
        HistoryViewer(const Rect& arg_render_area){
            set_render_area(arg_render_area);
        }
        void set_render_area(const Rect& arg_render_area);
        
        Snapshot& at(){
            assert(0 <= current_page and current_page < int(snapshots.size()));            
            return snapshots[current_page];
        }
        int size(){ return int(snapshots.size()); }

        void remember(const Snapshot& snapshot){
            if (size() != 0) { current_page = current_page + 1; }
            snapshots.push_back(snapshot);
        }

        const Snapshot& pick_snapshot(){
            if (0 <= current_page and current_page < int(snapshots.size())){
                return snapshots[current_page];
            } else {
                assert(0);
            }
        }
        bool is_page_refreshed(){
            if (m_is_page_refreshed) {
                m_is_page_refreshed = false;
                return true;
            } else {
                return false;
            }
        }
        void stash(const Snapshot& snapshot){
            stashed = snapshot;
        }
        void update(){
            if (left_button_area.leftClicked() and current_page > 0) {
                current_page--;
                m_is_page_refreshed = true;
            }
            if (right_button_area.leftClicked() and current_page < int(snapshots.size() - 1)) {
                current_page++;
                m_is_page_refreshed = true;
            }
        }
        void render();
        void save(const String& title, const DateTime& timestamp);
        void load_json(const FilePath& path);
        const Array<Snapshot>& see_snapshots(){
            return snapshots;
        }

        void reset(){
            snapshots.clear();
        }
        


};

