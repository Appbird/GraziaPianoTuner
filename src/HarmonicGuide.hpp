#pragma once
# include <Siv3D.hpp>
# include "util.hpp"
# include "Concepts.hpp"
# include "Interface/PanelComponents.hpp"
# include "ComposedRenderer.hpp"

class HarmonicGuide : public ParameterControllerPanel {
    public:
        HarmonicGuide() {}
        
        // Panel Component
        void update() override;
        void render() override;
        void set_state(bool state) override;
        void set_render_area(const Rect& render_area) override;
        bool terminated() override { return false; }
        void on_submit() override;
        
        // Describable
        String describe() const override { return snapshot_internal().describe(); }

        // JSONRepresentableState
        JSON snapshot() const override  { return snapshot_internal().encode(); }
        void memento(const JSON& json) override;

        double left_bar = 0;
        double right_bar = 0;
    private:
        Array<double> sequence { 0, 0.25, 0.5, 1.0 };
        int32_t count_of_bars = 16;
        int32_t control_unit_length = 4;
        Optional<int32_t> current_control_unit_index = 0;

        double parameter_min = 0.0;
        double parameter_max = 1.0;
        double parameter_length() const { return parameter_max - parameter_min; }
        double parameter_exceedtop() const { return parameter_max + parameter_length(); }
        double parameter_exceedbottom() const { return parameter_min - parameter_length(); }
        void set_parameter_min_max();
    private:
        struct Snapshot;

        HSV axis_color                  {88, 0.68, 0.80};
        HSV limitzone_color             {126, 0.35, 0.21};
        HSV panel_background_color      {90, 0.25, 0.09};
        HSV panel_slider_color           {90, 0.30, 0.20};
        HSV font_color                  {0, 0.0, 1.0};
        HSV control_point_color         {125, 0.69, 1.0};
        HSV control_point_color_darker         {125, 0.69, 0.90};
        HSV overlimit_point_color       {21, 0.79, 0.91};
        HSV semantic_target_button_color{104, 0.72, 0.65};
        HSV separator_color             {125, 0.34, 0.25};

        int point_inner_radius = 5;
        int point_outer_radius = 9;
        double lifetime = 0.5;
        bool active = true;
        bool dragging = false;
        // 軌跡を表示する
        Trail trail{lifetime};
        
        // UIの位置情報
        Rect display_rect;
        Rect top_limitzone;
        Rect middlezone;
        Rect bottom_limitzone;
        Rect sequence_panel;
        Rect semantic_panel;
        TextEditState y_axis_text_state{U"明るさ"};
        
        Font guide_font{24};

        double bar_separator_x(int32_t bar) const;
        double bar_separator_x(double bar) const;
        double bar_separator_width() const;
        double intensity_to_plotted_point_y(double intensity) const;
        double plotted_point_y_to_intensity(double y) const;
        Color plotted_point_color(double intensity) const;

        void draw_sequence_panel() const;
        void draw_bar_separators() const;
        void draw_semantic_panel();
        void plot_sequence() const;
        
        void write_sequence(const Point& touched_point);
        Snapshot snapshot_internal() const;
    private:
        struct Snapshot {
            int32_t guide_version;
            int32_t count_of_bars;
            int32_t control_unit_length;
            String axis_name;
            Array<double> sequence;

            Snapshot() {}
            Snapshot(
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

            JSON encode() const;
            static Snapshot decode(const JSON& json);
            String describe() const;
        };
};
