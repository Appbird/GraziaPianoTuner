# pragma once
# include <Siv3D.hpp>

static int mod(int a, int p){
    const int m = a % p;
    const int result = m + ((m >= 0) ? 0 : p);
    assert(InRange(result, 0, p - 1));
    return result;
}


class PianoRenderer{
    private:
        RasterizerState rs = RasterizerState::Default2D;
    public:
        Rect piano_area;
        PianoRenderer() : piano_area(){
            rs.scissorEnable = true;
        }
        PianoRenderer(Rect arg_piano_area) : piano_area(arg_piano_area){
            rs.scissorEnable = true;
        }
        // thanks to https://twitter.com/masaka_k/status/1536397542879297536?s=20
        void render_piano(
            const double lowest_pitch, const double highest_pitch,
            const double height_per_pitch,
            const Font& font
        ) const{
            constexpr int oct = 12; // オクターブ
            int key = mod((int)floor(lowest_pitch - 60), oct);
            double intital_pitch = floor(lowest_pitch);
            {
                Graphics2D::SetScissorRect(piano_area);
                ScopedRenderStates2D rasterizer{rs};
                for (
                    double p = intital_pitch;
                    p <= highest_pitch;
                    p++, key = (key + 1 == oct) ? 0 : key + 1
                ){
                    const std::array<int, 8> white_keys{0, 2, 4, 5, 7, 9, 11, 12};
                    Point offset{0, -int((p - lowest_pitch) * height_per_pitch)};
                    const Color frame_color = ColorF{0.8, 0.8, 0.8};
                    // 白鍵の描画
                    // #NOTE バグ:: (tl, size)を(tl, br)と勘違いしていたせいで時間を溶かす
                    const Rect white_key_rect{
                        piano_area.bl() - Point{0, (int)height_per_pitch} + offset,
                        Point{piano_area.w, (int)height_per_pitch}
                    };

                    white_key_rect
                        .drawFrame(2, frame_color)
                        .draw(ColorF{0.9, 0.9, 0.9});
                    if (key == 0) {
                        font(U"{}"_fmt(p)).drawAt(white_key_rect.center(), ColorF{0.5, 0.5, 0.5});
                    }
                    // #NOTE ここは最適化できる [最適化]
                    const auto white_keys_iter = std::lower_bound(white_keys.begin(), white_keys.end(), key);
                    
                    if (*white_keys_iter != key) {
                        //黒鍵だった場合
                        Rect {
                            piano_area.bl() - Point{0, (int)height_per_pitch}     + offset,
                            Point{int(piano_area.w * 0.6), (int)height_per_pitch}
                        }
                        .drawFrame(2, frame_color)
                        .draw(ColorF{0.05, 0.05, 0.05});
                    }
                }
            }
        }
};