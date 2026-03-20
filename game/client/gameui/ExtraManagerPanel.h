#pragma once

#include "vgui/ISurface.h"
#include "GameUI_Interface.h"
#include "vgui/ISystem.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/PanelListPanel.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/RichText.h"
#include "vgui_controls/ComboBox.h"

#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"

#ifdef ANDROID
#include <SDL_misc.h>
#endif

#include "utlvector.h"
#include "utlmap.h"

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

private:
    vgui::ImagePanel *m_pImagePanelPlaceholder; 
    vgui::Label      *m_pTitle;
    
    Color m_clrBgNormal;
    Color m_clrBgHover;
    
    int m_iMargin; 
    int m_nTextureID; 
    char m_szImagePath[MAX_PATH];
    char m_szUIMapName[MAX_PATH];
    bool m_bAttemptedLoad; // 是否尝试过加载，防止失败后死循环
};

// ---------------------------------------------------------
// 列表页面：管理纹理生命周期
// ---------------------------------------------------------
class ExtraListPage : public vgui::PropertyPage {
    DECLARE_CLASS_SIMPLE(ExtraListPage, vgui::PropertyPage);
public:
    ExtraListPage(vgui::Panel *parent, const char *panelName);
    virtual ~ExtraListPage(); 

    virtual void PerformLayout() override;
    void RefreshList(); 

    // 提供给 MapCardPanel 调用的纹理加载接口
    int GetTextureForPath(const char *fullPath);

private:
    int CreateTextureFromPNG(const char *fullPath);
    void CleanUpTextures();

    vgui::PanelListPanel *m_pMapListPanel; 

    // 纹理缓存：Key 是路径哈希或字符串，Value 是 TextureID
    CUtlMap<unsigned int, int> m_TextureCache; 
};

// ---------------------------------------------------------
// 主窗口
// ---------------------------------------------------------
class ExtraManagerPanel : public vgui::Frame {
    DECLARE_CLASS_SIMPLE(ExtraManagerPanel, vgui::Frame);
public:
    ExtraManagerPanel(vgui::Panel *parent);
    virtual ~ExtraManagerPanel();

    virtual void Activate() override;
    virtual void OnCommand(const char *command) override;
    virtual void OnClose() override;
    virtual void PerformLayout() override;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    virtual void OnKeyCodePressed(vgui::KeyCode code) override;

    void StartGame();
    MESSAGE_FUNC_PARAMS( OnMapCardSelected, "MapCardSelected", data );

    CCreateMultiplayerGameServerPage *GetServerPage() { return m_pServerPage; }

    vgui::EditablePanel *m_pLeftPanel;   
    vgui::PropertySheet *m_pTabSheet;
    ExtraListPage       *m_pMapListPage;

    vgui::EditablePanel *m_pRightPanel;  
    vgui::Label         *m_pDetailsLabel;
    
    vgui::Button        *m_pRefreshButton;
    vgui::Button        *m_pCloseButton;
    vgui::Button        *m_pStartButton;

    // tabs
    CCreateMultiplayerGameServerPage   *m_pServerPage;
    CCreateMultiplayerGameGameplayPage *m_pGameplayPage;
    CCreateMultiplayerGameBotPage      *m_pBotPage;

    // for loading/saving game config
    KeyValues *m_pSavedData;
};


