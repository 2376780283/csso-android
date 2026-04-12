#include "CardPanel.h"
#include "ExtraManagerPanel.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "tier1/checksum_crc.h"
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

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
