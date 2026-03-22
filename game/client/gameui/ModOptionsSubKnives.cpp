//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubKnives.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"

#include "LabeledCommandComboBox.h"
#include "tier1/convar.h"
#include "BitmapImagePanel.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

struct Knives
{
	const char*		m_szUIName;
	const char*		m_szImage;
};

static Knives knifeNames[] =
{
	{ "#GameUI_Loadout_Knife_Default",		NULL							},
	{ "#GameUI_Loadout_Knife_CSS",			"weapon_knife_css"				},
	{ "#GameUI_Loadout_Knife_Karambit",		"weapon_knife_karambit"			},
	{ "#GameUI_Loadout_Knife_Flip",			"weapon_knife_flip"				},
	{ "#GameUI_Loadout_Knife_Bayonet",		"weapon_knife_bayonet"			},
	{ "#GameUI_Loadout_Knife_M9_Bayonet",	"weapon_knife_m9_bayonet"		},
	{ "#GameUI_Loadout_Knife_Butterfly",	"weapon_knife_butterfly"		},
	{ "#GameUI_Loadout_Knife_Gut",			"weapon_knife_gut"				},
	{ "#GameUI_Loadout_Knife_Tactical",		"weapon_knife_tactical"			},
	{ "#GameUI_Loadout_Knife_Falchion",		"weapon_knife_falchion"			},
	{ "#GameUI_Loadout_Knife_Bowie",		"weapon_knife_survival_bowie"	},
	{ "#GameUI_Loadout_Knife_Canis",		"weapon_knife_canis"			},
	{ "#GameUI_Loadout_Knife_Cord",			"weapon_knife_cord"				},
	{ "#GameUI_Loadout_Knife_Gypsy",		"weapon_knife_gypsy_jackknife"	},
	{ "#GameUI_Loadout_Knife_Outdoor",		"weapon_knife_outdoor"			},
	{ "#GameUI_Loadout_Knife_Skeleton",		"weapon_knife_skeleton"			},
	{ "#GameUI_Loadout_Knife_Stiletto",		"weapon_knife_stiletto"			},
	{ "#GameUI_Loadout_Knife_Ursus",		"weapon_knife_ursus"			},
	{ "#GameUI_Loadout_Knife_Widowmaker",	"weapon_knife_widowmaker"		},
	{ "#GameUI_Loadout_Knife_Push",			"weapon_knife_push"				},
};

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubKnives::CModOptionsSubKnives(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubKnives")
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create Section Labels
	m_pCTLabel = new Label(this, "CTLabel", "#GameUI_Loadout_Knife_CT");
	m_pTLabel = new Label(this, "TLabel", "#GameUI_Loadout_Knife_T");

	// Create controls
	m_pKnifeImageCT = new CBitmapImagePanel(this, "KnifeImageCT", NULL);
	m_pKnifeImageCT->AddActionSignalTarget(this);

	m_pLoadoutKnifeCTComboBox = new CLabeledCommandComboBox(this, "KnifeCTComboBox");
	char command[64];
	for (int i = 0; i < ARRAYSIZE(knifeNames); i++)
	{
		Q_snprintf(command, sizeof(command), "loadout_slot_knife_weapon_ct %d", i);
		m_pLoadoutKnifeCTComboBox->AddItem(knifeNames[i].m_szUIName, command);
	}
	m_pLoadoutKnifeCTComboBox->AddActionSignalTarget(this);

	m_pKnifeImageT = new CBitmapImagePanel(this, "KnifeImageT", NULL);
	m_pKnifeImageT->AddActionSignalTarget(this);

	m_pLoadoutKnifeTComboBox = new CLabeledCommandComboBox(this, "KnifeTComboBox");
	for (int i = 0; i < ARRAYSIZE(knifeNames); i++)
	{
		Q_snprintf(command, sizeof(command), "loadout_slot_knife_weapon_t %d", i);
		m_pLoadoutKnifeTComboBox->AddItem(knifeNames[i].m_szUIName, command);
	}
	m_pLoadoutKnifeTComboBox->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Padding and spacing values - 更紧凑的布局
	int margin = PROPVAL(24);
	int spacing = PROPVAL(12);
	int controlHeight = PROPVAL(26);
	int labelWidth = PROPVAL(120);

	// 使用较小的预览图尺寸
	int previewSize = PROPVAL(140);

	// 计算左右两边的宽度
	int sideWidth = (pw - margin * 2 - spacing) / 2;

	int currentY = margin;

	// ================== SECTION 1: CT Knife ==================
	m_pCTLabel->SetPos(margin, currentY);
	m_pCTLabel->SetSize(sideWidth, controlHeight);
	m_pCTLabel->SetContentAlignment(Label::a_west);
	currentY += controlHeight + spacing;

	// Knife Preview CT - 使用较小的预览图
	m_pKnifeImageCT->SetBounds(margin, currentY, previewSize, previewSize);

	// Knife Selection CT - 放在预览图右侧
	m_pLoadoutKnifeCTComboBox->SetBounds(margin + previewSize + spacing, currentY, sideWidth - previewSize - spacing, controlHeight);
	currentY = margin;

	// ================== SECTION 2: T Knife ==================
	int tx = margin + sideWidth + spacing;

	currentY = margin;

	m_pTLabel->SetPos(tx, currentY);
	m_pTLabel->SetSize(sideWidth, controlHeight);
	m_pTLabel->SetContentAlignment(Label::a_west);
	currentY += controlHeight + spacing;

	// Knife Preview T - 使用较小的预览图
	m_pKnifeImageT->SetBounds(tx, currentY, previewSize, previewSize);

	// Knife Selection T - 放在预览图右侧
	m_pLoadoutKnifeTComboBox->SetBounds(tx + previewSize + spacing, currentY, sideWidth - previewSize - spacing, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubKnives::~CModOptionsSubKnives()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::RemapKnivesImage()
{
	const char *pImageNameCT = knifeNames[m_pLoadoutKnifeCTComboBox->GetActiveItem()].m_szImage;
	const char *pImageNameT = knifeNames[m_pLoadoutKnifeTComboBox->GetActiveItem()].m_szImage;

	char texture[256];
	if ( pImageNameCT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/knives/%s", pImageNameCT );
		m_pKnifeImageCT->setTexture( texture );
	}
	else
	{
		m_pKnifeImageCT->setTexture( "vgui/knives/weapon_knife" );
	}

	if ( pImageNameT != NULL )
	{
		Q_snprintf( texture, sizeof( texture ), "vgui/knives/%s", pImageNameT );
		m_pKnifeImageT->setTexture( texture );
	}
	else
	{
		m_pKnifeImageT->setTexture( "vgui/knives/weapon_knife_t" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnTextChanged( vgui::Panel *panel )
{
	RemapKnivesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnResetData()
{
	ConVarRef loadout_slot_knife_weapon_ct( "loadout_slot_knife_weapon_ct" );
	if ( loadout_slot_knife_weapon_ct.IsValid() )
		m_pLoadoutKnifeCTComboBox->SetInitialItem( loadout_slot_knife_weapon_ct.GetInt() );

	ConVarRef loadout_slot_knife_weapon_t( "loadout_slot_knife_weapon_t" );
	if ( loadout_slot_knife_weapon_t.IsValid() )
		m_pLoadoutKnifeTComboBox->SetInitialItem( loadout_slot_knife_weapon_t.GetInt() );

	RemapKnivesImage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubKnives::OnApplyChanges()
{
	m_pLoadoutKnifeCTComboBox->ApplyChanges();
	m_pLoadoutKnifeTComboBox->ApplyChanges();
}
