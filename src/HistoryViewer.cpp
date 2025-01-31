# include "HistoryViewer.hpp"
void HistoryViewer::render(){
    ColorF fontcolor{0.9};
    String page_display =
        (size() != 0)   ? U"{} / {}"_fmt(current_page + 1, size())
                        : U"- / -";
    font(page_display).draw(index_label_area.h, Arg::center = index_label_area.center(), fontcolor);
    if (current_page > 0){
        font(U"<").draw(int(left_button_area.h * 0.8), Arg::center = left_button_area.center(), fontcolor);
        if (left_button_area.mouseOver())     { RoundRect{left_button_area, 5}.draw(ColorF{0, 0.4}); }
        RoundRect{left_button_area, 5}.drawFrame(2, fontcolor);
    }
    if (current_page < int(ui_snapshot.size() - 1)){
        font(U">").draw(int(right_button_area.h), Arg::center = right_button_area.center(), fontcolor);
        if (right_button_area.mouseOver())    { RoundRect{right_button_area, 5}.draw(ColorF{0, 0.4}); }
        RoundRect{right_button_area, 5}.drawFrame(2, fontcolor);
    }
}

void HistoryViewer::save(const String& title, const DateTime& timestamp){
    const FilePath directory_path = U"./archive/";
    const FilePath jsonpath = directory_path + U"{}.json"_fmt(timestamp).replace(U" ", U"_");
    JSON json;
    for (const Snapshot& snapshot:ui_snapshot){
        json[U"ui_snapshot"].push_back(snapshot.encode());
    }
    json[U"llm_snapshot"] = LLM_dialog;
    json.save(jsonpath);
}

void HistoryViewer::load_json(const FilePath& path){
    JSON json = JSON::Load(path);
    assert(json);
    assert(json.isArray());
    assert(json.size() > 0);
    ui_snapshot.clear();
    for (const auto& [key, element]:json[U"ui_snapshot"]){
        const auto& result = Snapshot::decode(element);
        ui_snapshot.push_back(result);
    }
    current_page = (int)ui_snapshot.size() - 1;
    LLM_dialog = json[U"llm_snapshot"];
    m_is_page_refreshed = true;
}
void HistoryViewer::set_render_area(const Rect& arg_render_area){
    render_area = arg_render_area;
    RectSlicer layout{render_area, RectSlicer::X_axis};
    left_button_area = layout.from(0.2).to(0.4);
    index_label_area = layout.to(0.6);
    right_button_area = layout.to(0.8);
}
