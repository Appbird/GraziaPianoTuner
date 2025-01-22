# pragma once
# include <Siv3D.hpp>
# include "RichButton.hpp"

class SideMenuButtons {
    public:
    HashTable<String, RichButton> buttons;
    SideMenuButtons() {}
    SideMenuButtons(const Rect& menu_rect) {
        constexpr HSV credits_color{0, 0.4, 0.7};
        constexpr HSV folder_op_color{30, 0.5, 0.8};
        constexpr HSV qr_color{60, 0.4, 0.7};
        constexpr HSV example_color{90, 0.4, 0.7};
        //ボタン
        buttons = {
            {U"credits",    {U"\U000F0189", U"クレジット", Rect{}, true, credits_color}},
            {U"save",       {U"\U000F0193", U"保存", Rect{}, false, folder_op_color}},
            {U"load",       {U"\U000F024B", U"ロード", Rect{}, true, folder_op_color}},
            {U"reset",      {U"\U000F0A7A", U"リセット", Rect{}, true, folder_op_color}},
            {U"qr",         {U"\U000F0432", U"QRコード", Rect{}, false, qr_color}},
            {U"ex1",        {U"\U000F0387", U"Bright Sun", Rect{}, true, example_color}},
            {U"ex2",        {U"\U000F0387", U"Cafe Serenity", Rect{}, true, example_color}},
            {U"ex3",        {U"\U000F0387", U"Bright Daybreak", Rect{}, true, example_color}}
        };
        Array<String> order {
            U"credits",
            U"save",
            U"load",
            U"reset",
            U"qr",
            U"ex1",
            U"ex2",
            U"ex3"
        };
        RectSlicer layout_for_buttons{menu_rect, RectSlicer::Y_axis};
        {
            for (int i = 0; i < buttons.size(); i++){
                const Rect rect = layout_for_buttons.to((i+1) / 10.0).stretched(-5);
                buttons[order[i]].set_rect(rect);
            }
        }
    }
    RichButton& ref(const String& button_id) {
        assert(buttons.contains(button_id));
        return buttons.at(button_id);
    }  
};