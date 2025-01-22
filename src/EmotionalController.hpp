# pragma once
# include <Siv3D.hpp>
# include "Layout.hpp"
# include "util.hpp"
# include "Interface/PanelComponents.hpp"

class EmotionalController : public ParameterControllerPanel {
    public:
        bool using_param = true;
        Vec2 emotional_point{0, 0};

        EmotionalController() {}
        
        // Panel Component
        void update() override;
        void render() override;
        void set_render_area(const Rect& render_area) override;
        void set_state(bool arg_active) override{ active = arg_active; }
        bool terminated() override { return false; }

        // JSONRepresentableState
        JSON snapshot() const override { return snapshot_internal().encode(); }
        void memento(const JSON& json) override {
            Snapshot snapshot = Snapshot::decode(json);
            x_axis_text_state   = TextEditState{snapshot.X_axis};
            y_axis_text_state   = TextEditState{snapshot.Y_axis};
            emotional_point     = snapshot.emotional_parameters;
            using_param         = snapshot.is_used;
        }

        // descriable
        String describe() const override { return snapshot_internal().describe(); }
        
    private:
        struct Snapshot;
        
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
    
        Point display_point(); 
        void set_emotional_point(const Point& touched_point);
        Snapshot snapshot_internal() const;

        struct Snapshot {
            String X_axis;
            String Y_axis;
            Vec2 emotional_parameters;
            bool is_used;
            Snapshot(
                String X_axis,
                String Y_axis,
                Vec2 emotional_parameters,
                bool is_used
            ):
                X_axis(X_axis),
                Y_axis(Y_axis),
                emotional_parameters(emotional_parameters),
                is_used(is_used)
            {}
            JSON encode() const;
            static Snapshot decode(const JSON& json);
            String describe() const; 
        };
        
};