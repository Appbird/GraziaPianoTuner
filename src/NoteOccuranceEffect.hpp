# pragma once
# include <Siv3D.hpp>

# include "Basics.hpp"

class ComposedRenderer;

class NoteOccuranceEffect: public IEffect{
    private:
        const ComposedRenderer& renderer;
        const Note& note;
        const double lifetime;
        const double max_size;
    public:
        NoteOccuranceEffect(
            const ComposedRenderer& arg_renderer,
            const Note& arg_note,
            const double arg_lifetime,
            const double arg_max_size
        );
        bool update(double t) override;
};