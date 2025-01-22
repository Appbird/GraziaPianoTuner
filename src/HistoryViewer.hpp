# pragma once
# include <Siv3D.hpp>
# include "Composed.hpp"
# include "Layout.hpp"
#include "util.h"

class HistoryViewer
{
    public:
        struct Snapshot{
            String request;
            String user_to_LLM;
            String answer_from_LLM;
            ParameterControllMode params_type;
            JSON params;

            Snapshot(){}
            Snapshot(
                const String& arg_request,
                const String& arg_user,
                const String& arg_answer,
                ParameterControllMode params_type,
                const JSON& parameter_panel_snapshot
            ):
                request(arg_request),
                user_to_LLM(arg_user),
                answer_from_LLM(arg_answer),
                params_type(params_type),
                params(parameter_panel_snapshot)
            {}
            JSON encode() const {
                JSON result;
                Field2JSON(result, request);
                Field2JSON(result, user_to_LLM);
                Field2JSON(result, answer_from_LLM);
                Field2JSON(result, params);
                result[U"params_type"] = ToString(params_type);
                // #DONE 多様性
                // | params.name基準で分類するようにしたい
                return result;
            }
            static Snapshot decode(const JSON& json){
                assert(json[U"request"].isString());
                assert(json[U"user_to_LLM"].isString());
                assert(json[U"answer_from_LLM"].isString());
                assert(json[U"params_type"].isString());
                //#DONE 多様性
                // | ここで分岐発生するのなんかやだな（分岐が発生するのに、のちの工程でエラーが発生する可能性が抑制されていない）
                // | JSONで持たせるか？
                assert(json[U"params"].isString());
                return {
                    json[U"request"].getString(),
                    json[U"user_to_LLM"].getString(),
                    json[U"answer_from_LLM"].getString(),
                    ToEnumParameterControllMode(json[U"params_type"].getString()),
                    json[U"params"]
                };
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

