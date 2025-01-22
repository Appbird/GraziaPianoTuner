#pragma once
#include <Siv3D.hpp>

namespace concepts {
    
/** @brief Tは画面要素の一つであることを示す条件群 */
template <typename T>
concept PanelComponent = requires(T panel, const Rect& rect, bool active) {
    { panel.update() };
    { panel.render() };
    { panel.set_render_area(rect) };
    { panel.set_state(active) };
};

template <typename T>
concept Descriable =
    requires(T panel, const JSON& json) {
        // スナップショットとしての出力可能性
        { panel.snapshot() } -> std::same_as<JSON>;
        // スナップショットからの復元可能性
        { panel.memento(json) };
    };
    
// パラメータコンポーネントは次を満たさなくてはならない。
template <typename T>
concept ParameterControllerPanel = Descriable<T> and PanelComponent<T>;

}