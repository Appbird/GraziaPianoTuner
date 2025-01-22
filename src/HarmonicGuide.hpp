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
        Array<double> sequence;
        int32_t count_of_bars;
        int32_t control_unit_length;
        
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
        TextEditState y_axis_text_state{U"明るさ"};
    
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
