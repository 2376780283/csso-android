#pragma once

#include "vgui/ISurface.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui/IInput.h"

// 前向声明
class ExtraListPage;

// ---------------------------------------------------------
// 地图卡片控件：支持延迟加载
// ---------------------------------------------------------
class MapCardPanel : public vgui::EditablePanel {
    DECLARE_CLASS_SIMPLE(MapCardPanel, vgui::EditablePanel);
public:
    MapCardPanel(vgui::Panel *parent, const char *name, const char *title);

    void SetImagePath(const char *path);
    virtual void PerformLayout() override;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void Paint() override;

    virtual void OnCursorEntered() override;
    virtual void OnCursorExited() override;
    virtual void OnMousePressed(vgui::MouseCode code) override;

    // 异步加载接口
    void QueueForLoad();
    void ExecuteLoad();
    bool IsQueuedForLoad() const { return m_bQueuedForLoad; }
    bool IsLoadComplete() const { return m_bAttemptedLoad; }

private:
    vgui::ImagePanel *m_pImagePanelPlaceholder;
    vgui::Panel      *m_pContainer; // 底部容器：包含标题和背景
    vgui::Label      *m_pTitle;

    Color m_clrBgNormal;
    Color m_clrBgHover;

    int m_iMargin;
    int m_nTextureID;
    char m_szImagePath[MAX_PATH];
    char m_szUIMapName[MAX_PATH];
    bool m_bAttemptedLoad; // 是否尝试过加载，防止失败后死循环
    bool m_bQueuedForLoad; // 是否已在加载队列中
};
