# pragma once
# include <Siv3D.hpp>
# include "Layout.hpp"

class EmotionalController{
    public:
        bool using_param = true;
        struct EmotionalParameters{
            String X_axis;
            String Y_axis;
            Vec2 emotional_parameters;
            bool is_used;
            JSON encode() const{
                JSON result;
                result[U"X_axis"] = X_axis;
                result[U"Y_axis"] = Y_axis;
                result[U"Vec2"][U"x"] = emotional_parameters.x;
                result[U"Vec2"][U"y"] = emotional_parameters.y;
                result[U"is_used"] = is_used;
                return result;
            }
            static EmotionalParameters decode(const JSON& json){
                assert(json[U"X_axis"].isString());
                assert(json[U"Y_axis"].isString());
                assert(json[U"Vec2"].isObject());
                    assert(json[U"Vec2"][U"x"].isNumber());
                    assert(json[U"Vec2"][U"y"].isNumber());
                assert(json[U"is_used"].isBool());
                
                return {
                    json[U"X_axis"].getString(),
                    json[U"Y_axis"].getString(),
                    Vec2{
                        json[U"Vec2"][U"x"].get<double>(),
                        json[U"Vec2"][U"y"].get<double>()
                    },
                    json[U"is_used"].get<bool>()
                };
            }
            String describe() const{
                return (is_used) ? U"\nEmotional Parameters::\n\t{}:{: .2f}\n\t{}:{: .2f}\n"_fmt(
                    X_axis,
                    emotional_parameters.x,
                    Y_axis,
                    emotional_parameters.y
                ) : U"";
            }
        };


        Vec2 emotional_point{0, 0};
    private:
        int point_inner_radius = 5;
        int point_outer_radius = 9;
        double lifetime = 0.5;
        bool active = true;
        bool dragging = false;
        // 軌跡
        Trail trail{lifetime};

        // UIの位置情報
        Rect display_rect;
        Rect display_panel;
        Rect display_params;
            Font display_axis_guide_font{30};
            Font param_value_font{15, FileSystem::GetFolderPath(SpecialFolder::UserFonts) + U"MesloLGS NF Regular.ttf"};
            
            Rect display_x_axis_label;
            Rect display_x_axis_contents;
            Rect display_x_axis_value;
            TextEditState x_axis_text_state{U"躍動感"};
            
            Rect display_y_axis_label;
            Rect display_y_axis_contents;
            Rect display_y_axis_value;
            TextEditState y_axis_text_state{U"明るさ"};

            Rect display_checkbox_text; 
    
        Point display_point(){
            return (emotional_point * Vec2{1, -1} * display_panel.size / 2 + display_panel.center()).asPoint();
        }
        void set_emotional_point(const Point& touched_point){
            emotional_point = (Vec2(touched_point - display_panel.center()) / (display_panel.size / 2)) * Vec2{1, -1};
            emotional_point.x = Clamp(emotional_point.x, -1.0, 1.0);
            emotional_point.y = Clamp(emotional_point.y, -1.0, 1.0);
        }
    public:
        String describe() const{
            return EmotionalParameters{
                x_axis_text_state.text,
                y_axis_text_state.text,
                Vec2{emotional_point.x, emotional_point.y},
                using_param
            }.describe();
        }
        void set_state(bool arg_active){
            active = arg_active;
        }
        EmotionalController(){}
        EmotionalController(Rect& render_area){
            set_render_area(render_area);
        }
        void set_render_area(const Rect& render_area);
        void update();
        void render();
        EmotionalParameters snapshot(){
            return {
                x_axis_text_state.text,
                y_axis_text_state.text,
                emotional_point,
                using_param
            };
        }
        void memento(EmotionalParameters snapshot){
            x_axis_text_state   = TextEditState{snapshot.X_axis};
            y_axis_text_state   = TextEditState{snapshot.Y_axis};
            emotional_point     = snapshot.emotional_parameters;
            using_param          = snapshot.is_used;
        }
};