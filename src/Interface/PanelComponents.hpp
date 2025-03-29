#pragma once
#include <Siv3D.hpp>

/** @brief ユーザインタフェースとして画面に描画可能であり、毎フレーム更新可能である。 */
class PanelComponent {
public:
    virtual ~PanelComponent() = default;

    // 更新処理
    virtual void update() = 0;

    virtual void on_submit() {};

    // 描画処理
    virtual void render() = 0;

    // 描画領域の設定
    virtual void set_render_area(const Rect& rect) = 0;

    // アクティブ状態の設定
    virtual void set_state(bool active) = 0;

    // 終了状態かを確認できる
    virtual bool terminated() = 0;
};

/** @brief コンポーネントの内部状態はJSONによって完全に表現可能である。（健全である必要はない） */
class JSONRepresentableState {
public:
    virtual ~JSONRepresentableState() = default;

    // 内部状態がJSONでシリアライズ可能である
    virtual JSON snapshot() const = 0;

    // 適切な形式を持ったJSONから内部状態を復元することができる
    virtual void memento(const JSON& json) = 0;
};

/** @brief コンポーネントの内部状態を、LLMに対して説明可能である。 */
class Describale {
public:
    virtual ~Describale() = default;
    // 内部状態を説明可能
    virtual String describe() const = 0;
};


/** @brief 楽曲をパラメータで制御できるユーザインタフェースが満たすべき要件 */
class ParameterControllerPanel :
    public PanelComponent,
    public JSONRepresentableState,
    public Describale
{
public:
    virtual ~ParameterControllerPanel() = default;
};
