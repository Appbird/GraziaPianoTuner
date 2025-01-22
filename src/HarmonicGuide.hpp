#pragma once
# include <Siv3D.hpp>
# include "util.hpp"
# include "Concepts.hpp"
# include "Interface/PanelComponents.hpp"

class HarmonicGuide : public ParameterControllerPanel {
    public:
        HarmonicGuide() {}
        
        // Panel Component
        void update() override;
        void render() override;
        void set_state(bool state) override;
        void set_render_area(const Rect& render_area) override;
        bool terminated() override { return false; }
        
        // Describable
        String describe() const override { return snapshot_internal().describe(); }

        // JSONRepresentableState
        JSON snapshot() const override  { return snapshot_internal().encode(); }
        void memento(const JSON& json) override {
            Snapshot snapshot = Snapshot::decode(json);
            assert(snapshot.guide_version == 1);
            count_of_bars = snapshot.count_of_bars;
            control_unit_length = snapshot.control_unit_length;
            sequence = snapshot.sequence;
            y_axis_text_state.text = snapshot.axis_name;
        }

    private:
        Array<double> sequence { 0, 1.9, -1.0, 1.7 };
        int32_t count_of_bars = 16;
        int32_t control_unit_length = 4;
        
    private:
        struct Snapshot;

        HSV axis_color                  {88, 0.68, 0.51};
        HSV limitzone_color             {126, 0.35, 0.21};
        HSV panel_background_color      {90, 0.25, 0.09};
        HSV font_color                  {0, 0.0, 1.0};
        HSV control_point_color         {125, 0.69, 1.0};
        HSV overlimit_point_color       {21, 0.79, 0.91};
        HSV semantic_target_button_color{104, 0.72, 0.65};
        HSV separator_color             {125, 0.34, 0.25};

        int point_inner_radius = 5;
        int point_outer_radius = 9;
        double lifetime = 0.5;
        bool active = true;
        bool dragging = false;
        // 軌跡
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
        double plotted_point_y(double intensity) const;
        Color plotted_point_color(double intensity) const;


        void draw_background() const;
        void draw_bar_separators() const;
        void plot_sequence() const;
        
        Point display_point(int32_t index);
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
