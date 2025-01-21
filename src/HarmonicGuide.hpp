#pragma once
#include <Siv3D.hpp>
#include "util.hpp"

class HarmonicGuide {
    public:
        struct GuideParameter{
            int32_t guide_version;
            int32_t count_of_bars;
            int32_t control_unit_length;
            String axis_name;
            Array<double> sequence;

            GuideParameter() {}
            GuideParameter(
                int32_t guide_version,
                int32_t count_of_bars,
                int32_t control_unit_length,
                const String& axis_name,
                const Array<double>& sequence
            ): 
                guide_version(guide_version),
                count_of_bars(count_of_bars),
                control_unit_length(control_unit_length),
                axis_name(axis_name),
                sequence(sequence)
            {}

            JSON encode() const{
                JSON result;
                Field2JSON(result, guide_version);
                Field2JSON(result, count_of_bars);
                Field2JSON(result, axis_name);
                Field2JSON(result, control_unit_length);
                Field2JSON_Array(result, sequence);
                return result;
            }
            static GuideParameter decode(const JSON& json){
                // c.f. TODO: JSON Validator（JSON Schema）の使い方 https://scrapbox.io/Siv3D-instances/TODO:_JSON_Validator%EF%BC%88JSON_Schema%EF%BC%89%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9
                {
                    //#TODO schemaにcontrol_unit_lengthを加える。
                    JSONValidator validator = JSONValidator::Load(U"HarmonicGuideParams.schema.json");
                    validator.validationAssert(json);
                }
                GuideParameter param;
                JSON2Field(json, param, guide_version);
                JSON2Field(json, param, count_of_bars);
                JSON2Field(json, param, axis_name);
                JSON2Field(json, param, control_unit_length);
                JSON2Field_Array(json, param, sequence, double);
                return param;
            }
            String describe() const{
                String title = U"# Conceptual Parameters";
                
                String header_line = U"| Parameter name |";
                String under_header = U"|---|";
                String param_seq = U"| {} |"_fmt(axis_name);
                for (const auto [idx, value]: Indexed(sequence)){
                    const int32_t bar_start = control_unit_length*idx + 1;
                    const int32_t bar_end   = bar_start + control_unit_length - 1;
                    header_line     += U" bar {} - {} |"_fmt(bar_start, bar_end);
                    under_header    += U"---|";
                    param_seq       += U" {} |"_fmt(value);
                }
                String result = title   + U"\n\n";
                result += header_line   + U"\n";
                result += under_header  + U"\n";
                result += param_seq     + U"\n";
                return result;
            }
        };

        Array<double> sequence;
        int32_t count_of_bars;
        int32_t control_unit_length;
    private:
        int point_inner_radius = 5;
        int point_outer_radius = 9;
        double lifetime = 0.5;
        bool active = true;
        bool dragging = false;
        // 軌跡
        Trail trail{lifetime};
        
        // #FIXME ここら辺諸々調節
        // UIの位置情報
        Rect display_rect;
        TextEditState y_axis_text_state{U"明るさ"};
    
        Point display_point(int32_t index){
            //#TODO どの位置を触ったかによって、sequenceのどの位置が書き換えられたかを計算する。
            return {0, 0};
        }
        void write_sequence(const Point& touched_point){
            //#TODO どの位置を触ったかによって、sequenceのどの位置が書き換えられたかを計算する。
            return;
        }
        
    public:
        String describe() const{
            return snapshot().describe();
        }
        HarmonicGuide() {}
        HarmonicGuide(Rect& render_area){
            set_render_area(render_area);
        }
        void set_render_area(const Rect& render_area);
        void update();
        void render();
        GuideParameter snapshot() const{
            return GuideParameter{
                1,
                count_of_bars,
                control_unit_length,
                y_axis_text_state.text,
                sequence
            };
        }
        void memento(const GuideParameter& snapshot){
            assert(snapshot.guide_version == 1);
            count_of_bars = snapshot.count_of_bars;
            control_unit_length = snapshot.control_unit_length;
            sequence = snapshot.sequence;
            y_axis_text_state.text = snapshot.axis_name;
        }
};