//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubLoadout.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#include "GameUI_Interface.h"

#include "cs_shareddefs.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

extern ConVar loadout_music;

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::CModOptionsSubLoadout(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubLoadout")
{
	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create scroll container panel (holds all controls for scrolling)
	m_pScrollContainer = new vgui::Panel(this, "ScrollContainer");

	// Create vertical scroll bar
	m_pVScrollBar = new vgui::ScrollBar(this, "VScrollBar", true);
	m_pVScrollBar->AddActionSignalTarget(this);

	// Create all controls (inside scroll container)
	m_pLoadoutM4ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "M4ComboBox");
	m_pLoadoutM4ComboBox->AddItem("#Cstrike_WPNHUD_M4A4", "loadout_slot_m4_weapon 0");
	m_pLoadoutM4ComboBox->AddItem("#Cstrike_WPNHUD_M4A1", "loadout_slot_m4_weapon 1");
	m_pLoadoutM4ComboBox->AddActionSignalTarget(this);

	m_pLoadoutHKP2000ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "HKP2000ComboBox");
	m_pLoadoutHKP2000ComboBox->AddItem("#Cstrike_WPNHUD_HKP2000", "loadout_slot_hkp2000_weapon 0");
	m_pLoadoutHKP2000ComboBox->AddItem("#Cstrike_WPNHUD_USP45", "loadout_slot_hkp2000_weapon 1");
	m_pLoadoutHKP2000ComboBox->AddActionSignalTarget(this);

	m_pLoadoutFiveSevenComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "FiveSevenComboBox");
	m_pLoadoutFiveSevenComboBox->AddItem("#Cstrike_WPNHUD_FiveSeven", "loadout_slot_fiveseven_weapon 0");
	m_pLoadoutFiveSevenComboBox->AddItem("#Cstrike_WPNHUD_CZ75", "loadout_slot_fiveseven_weapon 1");
	m_pLoadoutFiveSevenComboBox->AddActionSignalTarget(this);

	m_pLoadoutMP7CTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "MP7CTComboBox");
	m_pLoadoutMP7CTComboBox->AddItem("#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_ct 0");
	m_pLoadoutMP7CTComboBox->AddItem("#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_ct 1");
	m_pLoadoutMP7CTComboBox->AddActionSignalTarget(this);

	m_pLoadoutDeagleCTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "DeagleCTComboBox");
	m_pLoadoutDeagleCTComboBox->AddItem("#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_ct 0");
	m_pLoadoutDeagleCTComboBox->AddItem("#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_ct 1");
	m_pLoadoutDeagleCTComboBox->AddActionSignalTarget(this);

	m_pLoadoutTec9ComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "Tec9ComboBox");
	m_pLoadoutTec9ComboBox->AddItem("#Cstrike_WPNHUD_Tec9", "loadout_slot_tec9_weapon 0");
	m_pLoadoutTec9ComboBox->AddItem("#Cstrike_WPNHUD_CZ75", "loadout_slot_tec9_weapon 1");
	m_pLoadoutTec9ComboBox->AddActionSignalTarget(this);

	m_pLoadoutMP7TComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "MP7TComboBox");
	m_pLoadoutMP7TComboBox->AddItem("#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_t 0");
	m_pLoadoutMP7TComboBox->AddItem("#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_t 1");
	m_pLoadoutMP7TComboBox->AddActionSignalTarget(this);

	m_pLoadoutDeagleTComboBox = new CLabeledCommandComboBox(m_pScrollContainer, "DeagleTComboBox");
	m_pLoadoutDeagleTComboBox->AddItem("#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_t 0");
	m_pLoadoutDeagleTComboBox->AddItem("#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_t 1");
	m_pLoadoutDeagleTComboBox->AddActionSignalTarget(this);

	m_pStatTrak = new CCvarToggleCheckButton(m_pScrollContainer, "EnableStatTrak", "#GameUI_Loadout_StatTrak", "loadout_stattrak");
	m_pStatTrak->AddActionSignalTarget(this);

	m_pMusicSelection = new CLabeledCommandComboBox(m_pScrollContainer, "MusicSelectionComboBox");
	for (int i = 0; i < MAX_MUSIC; i++)
	{
		char command[128];
		char string[128];
		Q_snprintf(command, sizeof(command), "loadout_music %d", i);
		Q_snprintf(string, sizeof(string), "#GameUI_Gameplay_MusicKit_%s", g_szMusicKits[i]);
		m_pMusicSelection->AddItem(string, command);
	}
	m_pMusicSelection->AddActionSignalTarget(this);

#if !INSTANT_MUSIC_CHANGE
	m_bNeedToWarnAboutMusic = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Padding and spacing values - scaled for different resolutions
	int margin = PROPVAL(24);
	int spacing = PROPVAL(12);
	int labelWidth = PROPVAL(200);
	int controlHeight = PROPVAL(26);
	int sectionSpacing = PROPVAL(28);
	int labelControlGap = PROPVAL(10);
	int scrollBarWidth = PROPVAL(20);

	// Calculate content width (accounting for scroll bar)
	int contentWidth = pw - (margin * 2) - scrollBarWidth;
	int controlWidth = contentWidth - labelWidth - labelControlGap;

	// Position scroll bar on the right side
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, margin);
	m_pVScrollBar->SetSize(scrollBarWidth, ph - (margin * 2));

	// Position scroll container
	m_pScrollContainer->SetPos(margin, margin);
	m_pScrollContainer->SetSize(contentWidth, ph - (margin * 2));

	// Calculate total content height first
	int totalContentHeight = 0;

	// ================== SECTION 1: CT Weapons ==================
	totalContentHeight += controlHeight + spacing; // M4
	totalContentHeight += controlHeight + spacing; // HKP2000
	totalContentHeight += controlHeight + spacing; // FiveSeven
	totalContentHeight += controlHeight + spacing; // MP7 CT
	totalContentHeight += controlHeight + spacing; // Deagle CT

	// ================== SECTION 2: T Weapons ==================
	totalContentHeight += sectionSpacing;
	totalContentHeight += controlHeight + spacing; // Tec9
	totalContentHeight += controlHeight + spacing; // MP7 T
	totalContentHeight += controlHeight + spacing; // Deagle T

	// ================== SECTION 3: Options ==================
	totalContentHeight += sectionSpacing;
	totalContentHeight += controlHeight + spacing; // StatTrak
	totalContentHeight += controlHeight + spacing; // Music Selection

	// Add bottom margin
	totalContentHeight += margin;

	// Set scroll bar range
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(ph - (margin * 2));

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();

	// Now position all controls inside the scroll container (offset by scroll position)
	int currentY = -scrollOffset;

	// ================== SECTION 1: CT Weapons ==================
	// M4
	m_pLoadoutM4ComboBox->SetPos(0, currentY);
	m_pLoadoutM4ComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// HKP2000
	m_pLoadoutHKP2000ComboBox->SetPos(0, currentY);
	m_pLoadoutHKP2000ComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// FiveSeven
	m_pLoadoutFiveSevenComboBox->SetPos(0, currentY);
	m_pLoadoutFiveSevenComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// MP7 CT
	m_pLoadoutMP7CTComboBox->SetPos(0, currentY);
	m_pLoadoutMP7CTComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Deagle CT
	m_pLoadoutDeagleCTComboBox->SetPos(0, currentY);
	m_pLoadoutDeagleCTComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 2: T Weapons ==================
	currentY += sectionSpacing;

	// Tec9
	m_pLoadoutTec9ComboBox->SetPos(0, currentY);
	m_pLoadoutTec9ComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// MP7 T
	m_pLoadoutMP7TComboBox->SetPos(0, currentY);
	m_pLoadoutMP7TComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Deagle T
	m_pLoadoutDeagleTComboBox->SetPos(0, currentY);
	m_pLoadoutDeagleTComboBox->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 3: Options ==================
	currentY += sectionSpacing;

	// StatTrak
	m_pStatTrak->SetPos(0, currentY);
	m_pStatTrak->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Music Selection
	m_pMusicSelection->SetPos(0, currentY);
	m_pMusicSelection->SetSize(controlWidth, controlHeight);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::~CModOptionsSubLoadout()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnScrollBarSliderMoved( KeyValues *data )
{
	int position = data->GetInt( "position", 0 );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle slider movement
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnSliderMoved( KeyValues *data )
{
	vgui::Panel* pPanel = static_cast<vgui::Panel*>(data->GetPtr( "panel" ));

	if ( pPanel == m_pVScrollBar )
	{
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnResetData()
{
	ConVarRef loadout_slot_m4_weapon( "loadout_slot_m4_weapon" );
	if ( loadout_slot_m4_weapon.IsValid() )
		m_pLoadoutM4ComboBox->SetInitialItem( loadout_slot_m4_weapon.GetInt() );

	ConVarRef loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon" );
	if ( loadout_slot_hkp2000_weapon.IsValid() )
		m_pLoadoutHKP2000ComboBox->SetInitialItem( loadout_slot_hkp2000_weapon.GetInt() );

	ConVarRef loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon" );
	if ( loadout_slot_fiveseven_weapon.IsValid() )
		m_pLoadoutFiveSevenComboBox->SetInitialItem( loadout_slot_fiveseven_weapon.GetInt() );

	ConVarRef loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon" );
	if ( loadout_slot_tec9_weapon.IsValid() )
		m_pLoadoutTec9ComboBox->SetInitialItem( loadout_slot_tec9_weapon.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct" );
	if ( loadout_slot_mp7_weapon_ct.IsValid() )
		m_pLoadoutMP7CTComboBox->SetInitialItem( loadout_slot_mp7_weapon_ct.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t" );
	if ( loadout_slot_mp7_weapon_t.IsValid() )
		m_pLoadoutMP7TComboBox->SetInitialItem( loadout_slot_mp7_weapon_t.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct" );
	if ( loadout_slot_deagle_weapon_ct.IsValid() )
		m_pLoadoutDeagleCTComboBox->SetInitialItem( loadout_slot_deagle_weapon_ct.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t" );
	if ( loadout_slot_deagle_weapon_t.IsValid() )
		m_pLoadoutDeagleTComboBox->SetInitialItem( loadout_slot_deagle_weapon_t.GetInt() );

	m_pStatTrak->Reset();

	m_pMusicSelection->SetInitialItem( Clamp( loadout_music.GetInt(), 0, MAX_MUSIC - 1 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnApplyChanges()
{
	m_pLoadoutM4ComboBox->ApplyChanges();
	m_pLoadoutHKP2000ComboBox->ApplyChanges();
	m_pLoadoutFiveSevenComboBox->ApplyChanges();
	m_pLoadoutTec9ComboBox->ApplyChanges();
	m_pLoadoutMP7CTComboBox->ApplyChanges();
	m_pLoadoutMP7TComboBox->ApplyChanges();
	m_pLoadoutDeagleCTComboBox->ApplyChanges();
	m_pLoadoutDeagleTComboBox->ApplyChanges();
	m_pStatTrak->ApplyChanges();

#if INSTANT_MUSIC_CHANGE
	if ( loadout_music.GetInt() != m_pMusicSelection->GetActiveItem() )
#else
	if ( m_bNeedToWarnAboutMusic && loadout_music.GetInt() != m_pMusicSelection->GetActiveItem() )
#endif
	{
		// Bring up the confirmation dialog
#if INSTANT_MUSIC_CHANGE
		m_pMusicSelection->ApplyChanges();
		GameUI().ReleaseBackgroundMusic();
#else
		MessageBox *box = new MessageBox( "#GameUI_OptionsRestartRequired_Title", "#GameUI_Gameplay_MusicRestartHint", this );
		box->MoveToFront();
		box->DoModal();
		m_bNeedToWarnAboutMusic = false;
#endif
	}
}
