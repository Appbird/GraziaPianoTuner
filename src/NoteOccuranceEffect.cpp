#include "NoteOccuranceEffect.hpp"
#include "ComposedRenderer.hpp"

NoteOccuranceEffect::NoteOccuranceEffect(
    const ComposedRenderer& arg_renderer,
    const Note& arg_note,
    const double arg_lifetime,
    const double arg_max_size
):
    renderer(arg_renderer),
    note(arg_note),
    lifetime(arg_lifetime),
    max_size(arg_max_size)
{}

bool NoteOccuranceEffect::update(double t){
    const Rect target = renderer.note_rect(note);
    const double e = EaseOutExpo(t / lifetime);
    const int current_inner_size = (int)round(e * e * max_size);
    const int current_outer_size = (int)round(e * max_size);
    Rect{
        target.tl() - current_inner_size * Point{1,1},
        target.size + 2 * current_inner_size * Point{1,1}
    }.drawFrame(current_outer_size - current_inner_size, HSV{180, 0.5, 0.6});
    return t < lifetime;
};