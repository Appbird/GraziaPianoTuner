# pragma once
# include <Siv3D.hpp>

# include "Basics.hpp"

class ComposedViewer;

class NoteOccuranceEffect: public IEffect{
    private:
        const ComposedViewer& renderer;
        const Note& note;
        const double lifetime;
        const double max_size;
    public:
        NoteOccuranceEffect(
            const ComposedViewer& arg_renderer,
            const Note& arg_note,
            const double arg_lifetime,
            const double arg_max_size
        );
        bool update(double t) override;
};