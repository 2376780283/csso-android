#include "ExtraManagerPanel.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "tier1/checksum_crc.h" // 用于路径哈希
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"

#include "CreateMultiplayerGameServerPage.h"
#include "CreateMultiplayerGameGameplayPage.h"
#include "CreateMultiplayerGameBotPage.h"
#include "EngineInterface.h"
#include "ModInfo.h"
#include "GameUI_Interface.h"
#include "vgui/ILocalize.h"
#include "gametypes.h"

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

extern IFileSystem *g_pFullFileSystem;

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

// ========
// 辅助函数：加载 PNG 并返回 TextureID
// ========
static int CreatePNGTextureHelper(const char *szPath) {
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(szPath, "MOD", buf)) return -1;

    int width, height, channels;
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;

    int targetW = 128; // 统一缩放大小
    int targetH = 128;
    unsigned char *resizedData = (unsigned char *)malloc(targetW * targetH * 4);
    int textureID = -1;

    if (resizedData) {
        if (stbir_resize_uint8(data, width, height, width * 4, resizedData, targetW, targetH, targetW * 4, 4)) {
            textureID = vgui::surface()->CreateNewTextureID(true);
            vgui::surface()->DrawSetTextureRGBA(textureID, resizedData, targetW, targetH, true, false);
        }
        free(resizedData);
    }

    stbi_image_free(data);
    return textureID;
}

// =========================================================
// ImageUrlButton
// =========================================================
class ImageUrlButton : public vgui::Panel
{
public:
    ImageUrlButton(Panel *parent, const char *name, const char *imagePath, const char *url) : Panel(parent, name)
    {
        m_szUrl = url;
        m_bSelected = false;
        m_textureID = CreatePNGTextureHelper(imagePath);
        
        SetMouseInputEnabled(true);
        SetPaintBackgroundEnabled(false);
    }

    virtual ~ImageUrlButton() {
        if (vgui::surface()->IsTextureIDValid(m_textureID)) {
            vgui::surface()->DeleteTextureByID(m_textureID);
        }
    }

    virtual void Paint()
    {
        if (m_textureID == -1) return;
        int alpha = m_bSelected ? 150 : 255;        
        vgui::surface()->DrawSetColor(255, 255, 255, alpha);
        vgui::surface()->DrawSetTexture(m_textureID);
        vgui::surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
    }

    virtual void OnMousePressed(MouseCode code) {
        if (code == MOUSE_LEFT) { m_bSelected = true; input()->SetMouseCapture(GetVPanel()); }
    }

    virtual void OnMouseReleased(MouseCode code) {
        if (code == MOUSE_LEFT) {
            m_bSelected = false;
            input()->SetMouseCapture(NULL);
            if (IsCursorOver() && m_szUrl) {
#ifdef ANDROID
                SDL_OpenURL(m_szUrl);
#else
                vgui::system()->ShellExecute("open", m_szUrl);
#endif
            }
        }
    }

private:
    bool m_bSelected;
    int m_textureID;
    const char *m_szUrl;
};



// =========================================================
// MapCardPanel 实现 (含延迟加载逻辑)
// =========================================================
MapCardPanel::MapCardPanel(vgui::Panel *parent, const char *name, const char *title) : BaseClass(parent, name) {
    m_nTextureID = -1;
    m_bAttemptedLoad = false;
    m_bQueuedForLoad = false;
    m_szImagePath[0] = '\0';
    Q_strncpy(m_szUIMapName, title ? title : "", sizeof(m_szUIMapName));

    SetPaintBackgroundEnabled(true);
    SetPaintBorderEnabled(false);
    SetMouseInputEnabled(true);
    m_iMargin = PROPVAL(6);

    m_clrBgNormal = Color(0, 0, 0, 0);
    m_clrBgHover = Color(89, 221, 242, 200);

    m_pImagePanelPlaceholder = new vgui::ImagePanel(this, "MapImage");
    m_pImagePanelPlaceholder->SetShouldScaleImage(true);
    m_pImagePanelPlaceholder->SetMouseInputEnabled(false);
    m_pImagePanelPlaceholder->SetVisible(false);

    m_pTitle = new vgui::Label(this, "MapTitle", title);
    m_pTitle->SetPaintBackgroundEnabled(false);
    m_pTitle->SetFgColor(Color(255, 255, 255, 255));
    m_pTitle->SetContentAlignment(vgui::Label::a_center);
    m_pTitle->SetMouseInputEnabled(false);

    int iImageSize = PROPVAL(120);
    int iLabelHeight = PROPVAL(36);
    SetSize(iImageSize + m_iMargin, iImageSize + iLabelHeight + m_iMargin);
}

void MapCardPanel::SetImagePath(const char *path) {
    if (path) { Q_strncpy(m_szImagePath, path, sizeof(m_szImagePath)); }
}

void MapCardPanel::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    m_pTitle->SetFont(pScheme->GetFont("DefaultVerySmall", IsProportional()));
}

void MapCardPanel::Paint() {
    BaseClass::Paint();

    int w, h;
    GetSize(w, h);
    int iMargin = PROPVAL(6);
    int contentW = w - iMargin;
    int drawX = iMargin / 2;
    int drawY = iMargin / 2;
    int imgSize = contentW;

    if (m_nTextureID != -1 && vgui::surface()->IsTextureIDValid(m_nTextureID)) {
        vgui::surface()->DrawSetColor(255, 255, 255, 255);
        vgui::surface()->DrawSetTexture(m_nTextureID);
        vgui::surface()->DrawTexturedRect(drawX, drawY, drawX + imgSize, drawY + imgSize);
    } else {
        // 加载中或无图：绘制深灰色占位背景
        vgui::surface()->DrawSetColor(30, 30, 30, 255);
        vgui::surface()->DrawFilledRect(drawX, drawY, drawX + imgSize, drawY + imgSize);
    }

    int labelY = drawY + imgSize;
    int labelH = PROPVAL(26);
    vgui::surface()->DrawSetColor(0, 0, 0, 150);
    vgui::surface()->DrawFilledRect(drawX, labelY, drawX + contentW, labelY + labelH);
}

// --- 异步加载：加入加载队列 ---
void MapCardPanel::QueueForLoad() {
    if (m_bQueuedForLoad || m_bAttemptedLoad || m_szImagePath[0] == '\0') return;
    m_bQueuedForLoad = true;

    // 向上寻找 ExtraListPage 并加入加载队列
    vgui::Panel *pPage = GetParent();
    while (pPage && !dynamic_cast<ExtraListPage *>(pPage)) { pPage = pPage->GetParent(); }

    if (pPage) {
        ExtraListPage *pListPage = static_cast<ExtraListPage *>(pPage);
        pListPage->QueueCardForLoad(this);
    }
}

// --- 执行实际的纹理加载 ---
void MapCardPanel::ExecuteLoad() {
    if (m_bAttemptedLoad || m_szImagePath[0] == '\0') return;
    
    // 向上寻找 ExtraListPage 以调用其缓存加载器
    vgui::Panel *pPage = GetParent();
    while (pPage && !dynamic_cast<ExtraListPage *>(pPage)) { pPage = pPage->GetParent(); }

    if (pPage) {
        ExtraListPage *pListPage = static_cast<ExtraListPage *>(pPage);
        m_nTextureID = pListPage->GetTextureForPath(m_szImagePath);
        m_bAttemptedLoad = true;
        m_bQueuedForLoad = false;
    }
}

void MapCardPanel::PerformLayout() {
    BaseClass::PerformLayout();
    int w, h;
    GetSize(w, h);

    int iMargin = PROPVAL(6);
    int contentW = w - iMargin;
    int drawX = iMargin / 2;
    int drawY = iMargin / 4;
    int imgSize = contentW;
    m_pImagePanelPlaceholder->SetBounds(drawX, drawY, imgSize, imgSize);
    int labelH = PROPVAL(26);
    int labelY = drawY + imgSize;
    m_pTitle->SetBounds(drawX, labelY, contentW, labelH);
}

void MapCardPanel::OnCursorEntered() {
    SetBgColor(m_clrBgHover);
}
void MapCardPanel::OnCursorExited() {
    SetBgColor(m_clrBgNormal);
}

void MapCardPanel::OnMousePressed(vgui::MouseCode code) {
    if (code == MOUSE_LEFT) { 
        KeyValues *msg = new KeyValues("MapCardSelected");
        msg->SetString("panelName", GetName());
        msg->SetString("uiMapName", m_szUIMapName);
        PostActionSignal(msg); 
    }
}





// =========================================================
// ExtraListPage 实现 (含纹理缓存)
// =========================================================
ExtraListPage::ExtraListPage(vgui::Panel *parent, const char *panelName) : BaseClass(parent, panelName) {
    // 创建过滤控件
    m_pFilterLabel = new vgui::Label(this, "FilterLabel", "#GameUI_Filtering_Mode");
    m_pFilterLabel->SetContentAlignment(vgui::Label::a_west);
    
    m_pGameTypeCombo = new vgui::ComboBox(this, "GameTypeCombo", 10, false);
    m_pGameTypeCombo->AddActionSignalTarget(this);
    
    m_pGameModeCombo = new vgui::ComboBox(this, "GameModeCombo", 10, false);
    m_pGameModeCombo->AddActionSignalTarget(this);
    
    m_pAllMapsCheck = new vgui::CheckButton(this, "AllMapsCheck", "#GameUI_AllMaps");
    m_pAllMapsCheck->AddActionSignalTarget(this);

    // 注册tick信号以驱动异步加载队列
    vgui::ivgui()->AddTickSignal(GetVPanel());
    
    // 初始化游戏类型列表
    int iGameTypeCount = g_pGameTypes->GetGameTypesCount();
    m_pGameTypeCombo->AddItem("#GameUI_AllMaps", new KeyValues("data", "game_type", -1));
    for (int i = 0; i < iGameTypeCount; i++) {
        const char* pszGameTypeNameID = g_pGameTypes->GetGameTypeNameID(i);
        if (pszGameTypeNameID) {
            m_pGameTypeCombo->AddItem(pszGameTypeNameID, new KeyValues("data", "game_type", i));
        }
    }
    m_pGameTypeCombo->ActivateItem(0);
    
    // 初始化游戏模式列表
    UpdateGameModeList();
    
    // 地图列表面板
    m_pMapListPanel = new vgui::PanelListPanel(this, "MapListPanel");
    m_pMapListPanel->SetFirstColumnWidth(0);
    m_pMapListPanel->SetNumColumns(4);
    m_pMapListPanel->SetVerticalBufferPixels(PROPVAL(12));

    m_TextureCache.SetLessFunc(DefLessFunc(unsigned int));
}

ExtraListPage::~ExtraListPage() {
    vgui::ivgui()->RemoveTickSignal(GetVPanel());
    CleanUpTextures();
}

void ExtraListPage::CleanUpTextures() {
    FOR_EACH_MAP(m_TextureCache, i) {
        int id = m_TextureCache[i];
        if (vgui::surface()->IsTextureIDValid(id)) { vgui::surface()->DeleteTextureByID(id); }
    }
    m_TextureCache.RemoveAll();
}

//-------------------------------------------------------------------------
// Purpose: 更新游戏模式列表
//-------------------------------------------------------------------------
void ExtraListPage::UpdateGameModeList()
{
    m_pGameModeCombo->DeleteAllItems();
    
    int nSelectedGameType = -1;
    KeyValues *pkvData = m_pGameTypeCombo->GetActiveItemUserData();
    if (pkvData) {
        nSelectedGameType = pkvData->GetInt("game_type", -1);
    }
    
    m_pGameModeCombo->AddItem("#GameUI_AllMaps", new KeyValues("data", "game_mode", -1));
    
    if (nSelectedGameType >= 0) {
        int iGameModeCount = g_pGameTypes->GetGameModesCount(nSelectedGameType);
        for (int i = 0; i < iGameModeCount; i++) {
            const char* pszGameModeNameID = g_pGameTypes->GetGameModeNameID(nSelectedGameType, i);
            if (pszGameModeNameID) {
                m_pGameModeCombo->AddItem(pszGameModeNameID, new KeyValues("data", "game_mode", i));
            }
        }
    }
    
    m_pGameModeCombo->ActivateItem(0);
}

int ExtraListPage::GetTextureForPath(const char *fullPath) {
    if (!fullPath || !fullPath[0]) return -1;

    // 使用 CRC 计算路径哈希作为 Key
    CRC32_t hash;
    CRC32_Init(&hash);
    CRC32_ProcessBuffer(&hash, fullPath, Q_strlen(fullPath));
    CRC32_Final(&hash);

    int index = m_TextureCache.Find(hash);
    if (index != m_TextureCache.InvalidIndex()) { return m_TextureCache[index]; }

    // 缓存中没有，执行实时加载
    int newID = CreateTextureFromPNG(fullPath);
    if (newID != -1) { m_TextureCache.Insert(hash, newID); }
    return newID;
}

int ExtraListPage::CreateTextureFromPNG(const char *fullPath) {
    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(fullPath, "MOD", buf)) return -1;

    int width, height, channels;
    // 使用 stb_image 解码
    unsigned char *data = stbi_load_from_memory((unsigned char *)buf.Base(), buf.TellPut(), &width, &height, &channels, 4);
    if (!data) return -1;

    // 性能优化：统一缩放到 128x128 节省显存
    int targetW = 128;
    int targetH = 128;
    unsigned char *resizedData = (unsigned char *)malloc(targetW * targetH * 4);
    if (resizedData) {
        if (stbir_resize_uint8(data, width, height, width * 4, resizedData, targetW, targetH, targetW * 4, 4)) {
            int textureID = vgui::surface()->CreateNewTextureID(true);
            vgui::surface()->DrawSetTextureRGBA(textureID, resizedData, targetW, targetH, true, false);
            stbi_image_free(data);
            free(resizedData);
            return textureID;
        }
        free(resizedData);
    }

    stbi_image_free(data);
    return -1;
}

void ExtraListPage::RefreshList() {
    // 清空加载队列
    m_LoadQueue.RemoveAll();
    
    m_pMapListPanel->DeleteAllItems();

    // 向上寻找 ExtraManagerPanel 以获取 ServerPage 引用
    vgui::Panel *pTarget = GetParent();
    while (pTarget && !dynamic_cast<ExtraManagerPanel *>(pTarget)) { 
        pTarget = pTarget->GetParent(); 
    }
    
    if (!pTarget) return;
    ExtraManagerPanel *pMain = static_cast<ExtraManagerPanel *>(pTarget);
    CCreateMultiplayerGameServerPage *pServerPage = pMain->GetServerPage();
    if (!pServerPage) return;

    // --- 1. 获取并预处理过滤条件 ---
    int nFilterGameType = -1;
    int nFilterGameMode = -1;
    bool bShowAllMaps = m_pAllMapsCheck->IsSelected();
    
    KeyValues *pkvGameTypeData = m_pGameTypeCombo->GetActiveItemUserData();
    if (pkvGameTypeData) {
        nFilterGameType = pkvGameTypeData->GetInt("game_type", -1);
    }
    
    KeyValues *pkvGameModeData = m_pGameModeCombo->GetActiveItemUserData();
    if (pkvGameModeData) {
        nFilterGameMode = pkvGameModeData->GetInt("game_mode", -1);
    }
    
    // 安全获取字符串标识，如果索引为 -1 则返回 NULL
    const char *pszFilterGameType = (nFilterGameType >= 0) ? g_pGameTypes->GetGameTypeFromInt(nFilterGameType) : NULL;
    const char *pszFilterGameMode = (nFilterGameType >= 0 && nFilterGameMode >= 0) ? g_pGameTypes->GetGameModeFromInt(nFilterGameType, nFilterGameMode) : NULL;

    // --- 2. 扫描地图文件 ---
    FileFindHandle_t findHandle = NULL;
    KeyValues *hiddenMaps = ModInfo().GetHiddenMaps();
    
    // 修改：使用 "GAME" 路径以搜索所有挂载的搜索路径，而不仅仅是 mod 文件夹
    const char *pszFilename = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "GAME", &findHandle);
    
    while (pszFilename)
    {
        char mapname[256];
        char *ext, *str;

        // 提取地图名逻辑
        str = Q_strstr(pszFilename, "maps");
        if (str)
        {
            Q_strncpy(mapname, str + 5, sizeof(mapname) - 1);
        }
        else
        {
            Q_strncpy(mapname, pszFilename, sizeof(mapname) - 1);
        }
        ext = Q_strstr(mapname, ".bsp");
        if (ext)
        {
            *ext = 0;
        }

        // 过滤隐藏地图
        if (hiddenMaps && hiddenMaps->GetInt(mapname, 0))
        {
            goto nextFile;
        }

        // --- 3. 核心过滤判断 ---
        if (!bShowAllMaps) 
        {
            // 如果 pszFilterGameType 为 NULL (选择了“全部”), 
            // 则应检查该地图是否【至少支持任何一种】已知的游戏模式，或者根据你的需求决定是否放行。
            // 这里对齐 ServerPage 的逻辑：如果指定了特定类型，则强制校验。
            if (pszFilterGameType)
            {
                if (!g_pGameTypes->IsValidMapForTypeAndMode(mapname, pszFilterGameType, pszFilterGameMode))
                    goto nextFile;
            }
            else
            {
                // 当用户选择“全部游戏类型”且未勾选“显示所有地图”时：
                // 建议：此处可以调用一个通用的校验，确保该地图不是背景地图(background)或无效地图
                if (Q_stristr(mapname, "background") || Q_stristr(mapname, "vactest"))
                    goto nextFile;
            }
        }

        {
            const char *szUIMapName = g_pGameTypes->GetMapNameID(mapname);
            if (!szUIMapName || !szUIMapName[0])
            {
                szUIMapName = mapname;
            }

            char szIconPath[MAX_PATH];
            Q_snprintf(szIconPath, sizeof(szIconPath), "materials/vgui/maps/%s.png", mapname);

            // 创建并配置卡片
            MapCardPanel *pCard = new MapCardPanel(m_pMapListPanel, mapname, szUIMapName);
            
            // 检查预览图是否存在 - 仅设置路径，不立即加载
            if (g_pFullFileSystem->FileExists(szIconPath, "GAME"))
            {
                pCard->SetImagePath(szIconPath);
                // 加入加载队列而不是立即加载
                pCard->QueueForLoad();
            }

            pCard->AddActionSignalTarget(pMain);
            m_pMapListPanel->AddItem(nullptr, pCard);
        }

    nextFile:
        pszFilename = g_pFullFileSystem->FindNext(findHandle);
    }
    
    g_pFullFileSystem->FindClose(findHandle);
}

// --- OnTick: 每帧处理加载队列 ---
void ExtraListPage::OnTick() {
    BaseClass::OnTick();
    ProcessLoadQueue();
}

// --- 将卡片加入加载队列 ---
void ExtraListPage::QueueCardForLoad(MapCardPanel *pCard) {
    if (!pCard) return;
    m_LoadQueue.AddToTail(pCard);
}

// --- 处理加载队列：每帧只加载少量 ---
void ExtraListPage::ProcessLoadQueue() {
    if (m_LoadQueue.IsEmpty()) return;

    // 每帧只处理少量，避免卡顿
    int loadsThisFrame = 0;
    while (!m_LoadQueue.IsEmpty() && loadsThisFrame < MAX_LOADS_PER_FRAME) {
        MapCardPanel *pCard = m_LoadQueue[0];
        m_LoadQueue.Remove(0);

        if (pCard && !pCard->IsLoadComplete() && pCard->IsQueuedForLoad()) {
            pCard->ExecuteLoad();
        }
        loadsThisFrame++;
    }
}

void ExtraListPage::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    
    if (m_pFilterLabel) m_pFilterLabel->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pGameTypeCombo) m_pGameTypeCombo->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pGameModeCombo) m_pGameModeCombo->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
    if (m_pAllMapsCheck) m_pAllMapsCheck->SetFont(pScheme->GetFont("DefaultSmall", IsProportional()));
}

void ExtraListPage::PerformLayout() {
    BaseClass::PerformLayout();
    int w, h;
    GetSize(w, h);
    int margin = PROPVAL(8);
    int iFilterHeight = PROPVAL(24);
    int iSpacing = PROPVAL(6);
    
    // 布局过滤控件
    int currentY = margin;
    m_pFilterLabel->SetBounds(margin, currentY, PROPVAL(50), iFilterHeight);
    
    int iComboX = margin + PROPVAL(55);
    int iComboWidth = (w - iComboX - margin - iSpacing) / 2;
    m_pGameTypeCombo->SetBounds(iComboX, currentY, iComboWidth, iFilterHeight);
    m_pGameModeCombo->SetBounds(iComboX + iComboWidth + iSpacing, currentY, iComboWidth, iFilterHeight);
    
    currentY += iFilterHeight + iSpacing;
    m_pAllMapsCheck->SetBounds(margin, currentY, w - margin * 2, iFilterHeight);
    
    currentY += iFilterHeight + iSpacing;
    int iListTop = currentY;
    m_pMapListPanel->SetBounds(margin, iListTop, w - (margin * 2), h - iListTop - margin);
}

// =========================================================
// ExtraManagerPanel 实现
// =========================================================
ExtraManagerPanel::ExtraManagerPanel(vgui::Panel *parent) : BaseClass(parent, "ExtraManagerPanel") {

    int screenW, screenH;
    vgui::surface()->GetScreenSize(screenW, screenH);
    SetSize(screenW, screenH);

    SetTitle("", false);
    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);
    SetMoveable(false);
    SetSizeable(false);
    SetCloseButtonVisible(false);
    
    // create KeyValues object to load/save config options
	m_pSavedData = new KeyValues( "ServerConfig" );
	
	int nGameType = 0;
	int nGameMode = 0;
	bool bAllMaps = false;
	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile( g_pFullFileSystem, "ServerConfig.vdf", "GAME" ); // this is game-specific data, so it should live in GAME, not CONFIG
		
		nGameType = m_pSavedData->GetInt( "game_type" );
		nGameMode = m_pSavedData->GetInt( "game_mode" );
		bAllMaps = m_pSavedData->GetBool( "all_maps" );
	}

    m_pLeftPanel = new vgui::EditablePanel(this, "LeftFloatingPanel");
    m_pTabSheet = new PropertySheet(m_pLeftPanel, "ExtraTabs");
    m_pMapListPage = new ExtraListPage(m_pTabSheet, "MapListPage");
    
    m_pServerPage = new CCreateMultiplayerGameServerPage(this, "ServerPage", nGameType, nGameMode, bAllMaps);
    m_pGameplayPage = new CCreateMultiplayerGameGameplayPage(this, "GameplayPage");
    m_pBotPage = NULL;
    
    m_pServerPage->UpdateGameplayPage(); // do it AFTER m_pGameplayPage has been added

    if ( m_pSavedData )
	{
		const char *startMap = m_pSavedData->GetString("map", "");
		if (startMap[0])
		{
			m_pServerPage->SetMap(startMap);
		}
		const char *hostname = m_pSavedData->GetString("hostname", "");
		if (hostname[0])
		{
			m_pServerPage->SetHostName(hostname);
		}
		const char *maxplayers = m_pSavedData->GetString("maxplayers", "");
		if (maxplayers[0])
		{
			m_pServerPage->SetMaxPlayers(maxplayers);
		}
		const char *sv_password = m_pSavedData->GetString("sv_password", "");
		if (sv_password[0])
		{
			m_pServerPage->SetPassword(sv_password);
		}
	}

    m_pTabSheet->AddPage(m_pMapListPage, "#GameUI_Map");
    
    m_pTabSheet->AddPage(m_pServerPage, "#GameUI_Server");
    
    if ( ModInfo().UseBots() )
	{
		m_pBotPage = new CCreateMultiplayerGameBotPage( m_pTabSheet, "BotPage", m_pSavedData );
		m_pTabSheet->AddPage( m_pBotPage, "#GameUI_CPUPlayerOptions" );
		m_pServerPage->EnableBots( m_pSavedData );
	}
    
    m_pTabSheet->AddPage(m_pGameplayPage, "#GameUI_Game");

    m_pRightPanel = new vgui::EditablePanel(this, "RightFloatingPanel");
    m_pDetailsLabel = new vgui::Label(m_pRightPanel, "DetailsLabel", "Information");
    
    m_pRefreshButton = new vgui::Button(m_pRightPanel, "RefreshBtn", "#GameUI_Refresh", this, "RefreshList");
    m_pStartButton = new vgui::Button(m_pRightPanel, "StartBtn", "#GameUI_Start", this, "StartGame");
    m_pCloseButton = new Button(this, "CloseBtn", "#GameUI_Close", this, "Close");
}

ExtraManagerPanel::~ExtraManagerPanel() {
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

void ExtraManagerPanel::OnMapCardSelected(KeyValues *data) {
    if (!data) return;
    const char *pPanelName = data->GetString("panelName", "");

    if (m_pServerPage) {
        m_pServerPage->SetMap(pPanelName);
    }
}

void ExtraManagerPanel::StartGame() {
    // 1. 先重置所有被修改的 ConVars，这样 ApplyChanges 才能覆盖它们
    if (g_pCVar) {
        g_pCVar->RevertFlaggedConVars(FCVAR_REPLICATED);
        g_pCVar->RevertFlaggedConVars(FCVAR_CHEAT);
    }

    DevMsg("FCVAR_CHEAT cvars reverted to defaults.\n");

    // 2. 调用选项卡的 ApplyChanges，这会触发所有 Page 的 OnApplyChanges
    if (m_pTabSheet) {
        m_pTabSheet->ApplyChanges();
    }

    // get these values from m_pServerPage and store them temporarily
    char szMapName[64], szHostName[64], szPassword[64];
    int iGameTypeID = m_pServerPage->GetGameTypeID();
    int iGameModeID = m_pServerPage->GetGameModeID();
    int iMaxPlayers = m_pServerPage->GetMaxPlayers();
    Q_strncpy(szMapName, m_pServerPage->GetMapName(), sizeof(szMapName));
    Q_strncpy(szHostName, m_pServerPage->GetHostName(), sizeof(szHostName));
    Q_strncpy(szPassword, m_pServerPage->GetPassword(), sizeof(szPassword));

    int iBotQuota = 0;

    // save the config data
    if (m_pSavedData) {
        if (m_pServerPage->IsRandomMapSelected()) {
            m_pSavedData->SetString("map", "");
        } else {
            m_pSavedData->SetString("map", szMapName);
        }

        m_pSavedData->SetInt("game_type", iGameTypeID);
        m_pSavedData->SetInt("game_mode", iGameModeID);
        m_pSavedData->SetBool("all_maps", m_pServerPage->IsAllMaps());
        m_pSavedData->SetString("hostname", szHostName);
        m_pSavedData->SetInt("maxplayers", iMaxPlayers);
        m_pSavedData->SetString("sv_password", szPassword);

        // 获取机器人数量并保存
        iBotQuota = m_pSavedData->GetInt("bot_quota", 0);
        // 如果难度选择为 0 (通常是 "无机器人" 选项)，则强制数量为 0
        if (m_pSavedData->GetInt("custom_bot_difficulty", 0) == 0)
            iBotQuota = 0;

        // save config to a file
        m_pSavedData->SaveToFile(g_pFullFileSystem, "ServerConfig.vdf", "GAME");
    }

    char szMapCommand[1024];

    // create the command to execute
    // 增加一些必要的等待和初始化命令，确保 ConVars 已经应用
    // 显式在命令中设置 bot_quota 以确保生效
    Q_snprintf(szMapCommand, sizeof(szMapCommand),
               "disconnect\nwait\nwait\nsv_lan 1\nsetmaster enable\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nbot_quota %i\nprogress_enable\ngame_type %d\ngame_mode %d\ngame_online 0\nmap %s\n",
               iMaxPlayers, szPassword, szHostName, iBotQuota, iGameTypeID, iGameModeID, szMapName);

    // exec
    engine->ClientCmd_Unrestricted(szMapCommand);

    Close();
}





void ExtraManagerPanel::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);

    if (m_pLeftPanel) {
        m_pLeftPanel->SetPaintBackgroundEnabled(true);
        m_pLeftPanel->SetPaintBorderEnabled(true);
        m_pLeftPanel->SetBorder(pScheme->GetBorder("FrameBorder"));
        m_pLeftPanel->SetBgColor(pScheme->GetColor("Frame.BgColor", Color(0, 0, 0, 200)));
    }

    if (m_pRightPanel) {
        m_pRightPanel->SetPaintBackgroundEnabled(true);
        m_pRightPanel->SetPaintBorderEnabled(true);
        m_pRightPanel->SetBorder(pScheme->GetBorder("FrameBorder"));
        m_pRightPanel->SetBgColor(pScheme->GetColor("Frame.BgColor", Color(0, 0, 0, 150)));
    }

    if (m_pDetailsLabel) m_pDetailsLabel->SetFont(pScheme->GetFont("DefaultLarge", IsProportional()));
    if (m_pCloseButton) m_pCloseButton->SetFont(pScheme->GetFont("DefaultLarge", IsProportional()));
}

void ExtraManagerPanel::PerformLayout() {
    BaseClass::PerformLayout();
    int sw, sh;
    GetSize(sw, sh);
    int iPadding = PROPVAL(0), iGap = PROPVAL(10);
    int leftW = (sw * 0.65) - (iPadding + iGap / 2);
    int rightW = sw - leftW - (iPadding * 2) - iGap;
    int panelH = sh - (iPadding * 2);

    m_pLeftPanel->SetBounds(iPadding, iPadding, leftW, panelH);
    m_pRightPanel->SetBounds(iPadding + leftW + iGap, iPadding, rightW, panelH);

    int tPadding = PROPVAL(12);
    m_pTabSheet->SetBounds(tPadding, tPadding, leftW - (tPadding * 2), panelH - (tPadding * 2));

    int rInnerPad = PROPVAL(15);
    m_pDetailsLabel->SetBounds(rInnerPad, rInnerPad, rightW - (rInnerPad * 2), PROPVAL(30));

    // Arrange buttons vertically at the bottom right
    int btnW = PROPVAL(110), btnH = PROPVAL(28);
    int btnGap = PROPVAL(10);
    int totalBtnHeight = (btnH * 3) + (btnGap * 2);
    int startY = panelH - rInnerPad - totalBtnHeight;
    
    // Vertical arrangement: Refresh at top, Start in middle, Close at bottom
    m_pRefreshButton->SetBounds(rInnerPad, startY, btnW, btnH);
    m_pStartButton->SetBounds(rInnerPad, startY + btnH + btnGap, btnW, btnH);
    m_pCloseButton->SetBounds(sw - iPadding - rInnerPad - btnW, sh - iPadding - rInnerPad - btnH, btnW, btnH);
}

void ExtraManagerPanel::OnCommand(const char *command) {
    if (!Q_stricmp(command, "Close"))
        Close();
    else if (!Q_stricmp(command, "RefreshList") && m_pMapListPage)
        m_pMapListPage->RefreshList();
    else if (!Q_stricmp(command, "StartGame"))
        StartGame();
    else
        BaseClass::OnCommand(command);
}

void ExtraManagerPanel::Activate() {
    BaseClass::Activate();
    if (m_pMapListPage) m_pMapListPage->RefreshList();
}

void ExtraManagerPanel::OnKeyCodePressed( vgui::KeyCode code )
{
	// Handle close here, CBasePanel parent doesn't support "DialogClosing" command
	ButtonCode_t nButtonCode = GetBaseButtonCode( code );

	if ( nButtonCode == KEY_XBUTTON_B )
	{
		OnCommand( "Close" );
	}
	else if ( nButtonCode == KEY_XBUTTON_A || nButtonCode == STEAMCONTROLLER_A )
	{
		StartGame();
	}
	else if ( nButtonCode == KEY_XBUTTON_UP || 
			  nButtonCode == KEY_XSTICK1_UP ||
			  nButtonCode == KEY_XSTICK2_UP ||
			  nButtonCode == STEAMCONTROLLER_DPAD_UP ||
			  nButtonCode == KEY_UP )
	{
		if (m_pServerPage && m_pServerPage->GetMapList())
		{
			int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) - 1;
			if ( nItem < 0 )
			{
				nItem = m_pServerPage->GetMapList()->GetItemCount() - 1;
			}
			m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
		}
	}
	else if ( nButtonCode == KEY_XBUTTON_DOWN || 
			  nButtonCode == KEY_XSTICK1_DOWN ||
			  nButtonCode == KEY_XSTICK2_DOWN || 
			  nButtonCode == STEAMCONTROLLER_DPAD_DOWN ||
			  nButtonCode == KEY_DOWN )
	{
		if (m_pServerPage && m_pServerPage->GetMapList())
		{
			int nItem = m_pServerPage->GetMapList()->GetSelectedItem(0) + 1;
			if ( nItem >= m_pServerPage->GetMapList()->GetItemCount() )
			{
				nItem = 0;
			}
			m_pServerPage->GetMapList()->SetSingleSelectedItem( nItem );
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void ExtraManagerPanel::OnClose() {
    BaseClass::OnClose();
    MarkForDeletion();
}

//-------------------------------------------------------------------------
// Purpose: ExtraListPage 消息处理
//-------------------------------------------------------------------------
void ExtraListPage::OnTextChanged(Panel *panel)
{
    if (panel == m_pGameTypeCombo)
    {
        UpdateGameModeList();
        RefreshList();
    }
    else if (panel == m_pGameModeCombo)
    {
        RefreshList();
    }
}

void ExtraListPage::OnCheckButtonChecked(Panel *panel)
{
    if (panel == m_pAllMapsCheck)
    {
        RefreshList();
    }
}
