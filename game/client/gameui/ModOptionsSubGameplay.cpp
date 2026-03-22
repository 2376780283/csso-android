//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubGameplay.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ScrollBar.h>
#include "vgui_controls/QueryBox.h"

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifndef PROPVAL
#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout
//-----------------------------------------------------------------------------
CModOptionsSubGameplay::CModOptionsSubGameplay( vgui::Panel *parent ): vgui::PropertyPage( parent, "ModOptionsSubGameplay" )
{
	// Create controls - positions will be set in PerformLayout
#ifndef PROPVAL
	#define PROPVAL(x) (IsProportional() ? scheme()->GetProportionalScaledValueEx(GetScheme(), (x)) : (x))
#endif

	// Initialize with minimum size - will be resized in PerformLayout
	SetSize(100, 100);

	// Create scroll container panel (holds all controls for scrolling)
	m_pScrollContainer = new vgui::Panel(this, "ScrollContainer");

	// Create vertical scroll bar
	m_pVScrollBar = new vgui::ScrollBar(this, "VScrollBar", true);
	m_pVScrollBar->AddActionSignalTarget(this);

	// Create Section Labels (inside scroll container)
	m_pGeneralLabel = new Label(m_pScrollContainer, "GeneralLabel", "#GameUI_Gameplay");
	m_pViewmodelLabel = new Label(m_pScrollContainer, "ViewmodelLabel", "#GameUI_Gameplay_Viewmodel");
	m_pMovementLabel = new Label(m_pScrollContainer, "MovementLabel", "#GameUI_Gameplay_Viewmodel"); // missing language
	m_pPresetLabel = new Label(m_pScrollContainer, "PresetLabel", "#GameUI_Gameplay_ViewmodelPreset");
	m_pOffsetXLabel = new Label(m_pScrollContainer, "OffsetXLabel", "Viewmodel Offset X"); // missing language
	m_pOffsetYLabel = new Label(m_pScrollContainer, "OffsetYLabel", "Viewmodel Offset Y");
	m_pOffsetZLabel = new Label(m_pScrollContainer, "OffsetZLabel", "Viewmodel Offset Z");
	m_pFOVLabel = new Label(m_pScrollContainer, "FOVLabel", "#GameUI_Gameplay_ViewmodelFOV");
	m_pRecoilLabel = new Label(m_pScrollContainer, "RecoilLabel", "#GameUI_Gameplay_ViewmodelRecoil");
	m_pViewbobLabel = new Label(m_pScrollContainer, "ViewbobLabel", "#GameUI_Gameplay_ViewbobStyle");
	m_pWeaponPosLabel = new Label(m_pScrollContainer, "WeaponPosLabel", "#GameUI_Gameplay_WeaponPos");

	// Create all controls (inside scroll container)
	m_pCloseOnBuy = new CCvarToggleCheckButton(m_pScrollContainer, "CloseOnBuyCheckbox", "#GameUI_Gameplay_CloseOnBuy", "closeonbuy");
	m_pUseOpensBuyMenu = new CCvarToggleCheckButton(m_pScrollContainer, "UseOpensBuyMenuCheckbox", "#GameUI_Gameplay_UseOpensBuyMenu", "cl_use_opens_buy_menu");
	m_pAddBotPrefix = new CCvarToggleCheckButton(m_pScrollContainer, "AddBotPrefix", "#GameUI_Gameplay_AddBotPrefix", "cl_add_bot_prefix");
	m_pDrawTracers = new CCvarToggleCheckButton(m_pScrollContainer, "DrawTracers", "#GameUI_Gameplay_DrawTracers", "r_drawtracers");
	m_pSpecInterpCamera = new CCvarToggleCheckButton(m_pScrollContainer, "SpecInterpCamera", "#GameUI_Gameplay_SpecInterpCamera", "cl_obs_interp_enable");
	m_pDisableShootingEffects = new CCvarToggleCheckButton(m_pScrollContainer, "DisableShootingEffects", "#GameUI_Gameplay_DisableShootingEffects", "cl_disable_shooting_effects");

	m_pViewmodelOffsetX = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetXSlider", "", -2.0f, 2.5f, "viewmodel_offset_x");
	m_pViewmodelOffsetXLabel = new Label(m_pScrollContainer, "ViewmodelOffsetXLabel", "0.0");
	m_pViewmodelOffsetY = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetYSlider", "", -2.0f, 2.0f, "viewmodel_offset_y");
	m_pViewmodelOffsetYLabel = new Label(m_pScrollContainer, "ViewmodelOffsetYLabel", "0.0");
	m_pViewmodelOffsetZ = new CCvarSlider(m_pScrollContainer, "ViewmodelOffsetZSlider", "", -2.0f, 2.0f, "viewmodel_offset_z");
	m_pViewmodelOffsetZLabel = new Label(m_pScrollContainer, "ViewmodelOffsetZLabel", "0.0");
	m_pViewmodelOffsetPreset = new CLabeledCommandComboBox(m_pScrollContainer, "ViewmodelOffsetPreset");
	m_pViewmodelFOV = new CCvarSlider(m_pScrollContainer, "ViewmodelFOVSlider", "", 54.0f, 68.0f, "viewmodel_fov");
	m_pViewmodelFOVLabel = new Label(m_pScrollContainer, "ViewmodelFOVLabel", "60");
	m_pViewmodelRecoil = new CCvarSlider(m_pScrollContainer, "ViewmodelRecoilSlider", "", 0.0f, 1.0f, "viewmodel_recoil");
	m_pViewmodelRecoilLabel = new Label(m_pScrollContainer, "ViewmodelRecoilLabel", "0.0");
	m_pViewbobStyle = new CLabeledCommandComboBox(m_pScrollContainer, "ViewbobStyleComboBox");
	m_pWeaponPos = new CLabeledCommandComboBox(m_pScrollContainer, "WeaponPositionComboBox");

	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_1", "viewmodel_presetpos 1");
	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_2", "viewmodel_presetpos 2");
	m_pViewmodelOffsetPreset->AddItem("#GameUI_Gameplay_Viewmodel_Preset_3", "viewmodel_presetpos 3");

	m_pViewbobStyle->AddItem("#GameUI_Gameplay_Viewbob_CSS", "cl_use_new_headbob 0");
	m_pViewbobStyle->AddItem("#GameUI_Gameplay_Viewbob_CSGO", "cl_use_new_headbob 1");

	m_pWeaponPos->AddItem("#GameUI_Gameplay_Hand_Left", "cl_righthand 0");
	m_pWeaponPos->AddItem("#GameUI_Gameplay_Hand_Right", "cl_righthand 1");

	m_pCloseOnBuy->AddActionSignalTarget(this);
	m_pUseOpensBuyMenu->AddActionSignalTarget(this);
	m_pAddBotPrefix->AddActionSignalTarget(this);
	m_pDrawTracers->AddActionSignalTarget(this);
	m_pSpecInterpCamera->AddActionSignalTarget(this);
	m_pDisableShootingEffects->AddActionSignalTarget(this);
	m_pViewmodelOffsetX->AddActionSignalTarget(this);
	m_pViewmodelOffsetY->AddActionSignalTarget(this);
	m_pViewmodelOffsetZ->AddActionSignalTarget(this);
	m_pViewmodelFOV->AddActionSignalTarget(this);
	m_pViewmodelRecoil->AddActionSignalTarget(this);
	m_pViewmodelOffsetPreset->AddActionSignalTarget(this);
	m_pViewbobStyle->AddActionSignalTarget(this);
	m_pWeaponPos->AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - called when size changes
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size from parent
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
	int sliderLabelWidth = PROPVAL(40);
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

	// ================== SECTION 1: General Settings ==================
	totalContentHeight += controlHeight + spacing; // GeneralLabel
	totalContentHeight += (controlHeight + spacing) * 6; // 6 checkboxes

	// ================== SECTION 2: Viewmodel ==================
	totalContentHeight += sectionSpacing;
	totalContentHeight += controlHeight + spacing; // ViewmodelLabel
	totalContentHeight += controlHeight + spacing; // Preset
	totalContentHeight += (controlHeight + spacing) * 5; // 5 offset/FOV/recoil sliders

	// ================== SECTION 3: Movement ==================
	totalContentHeight += sectionSpacing;
	totalContentHeight += controlHeight + spacing; // MovementLabel
	totalContentHeight += (controlHeight + spacing) * 2; // Viewbob + Weapon Position

	// Add bottom margin
	totalContentHeight += margin;

	// Set scroll bar range
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(ph - (margin * 2));

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();

	// Now position all controls inside the scroll container (offset by scroll position)
	int currentY = -scrollOffset;

	// ================== SECTION 1: General Settings ==================
	m_pGeneralLabel->SetPos(0, currentY);
	m_pGeneralLabel->SetSize(contentWidth, controlHeight);
	m_pGeneralLabel->SetContentAlignment(Label::a_west);
	currentY += controlHeight + spacing;

	m_pCloseOnBuy->SetPos(0, currentY);
	m_pCloseOnBuy->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pUseOpensBuyMenu->SetPos(0, currentY);
	m_pUseOpensBuyMenu->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pAddBotPrefix->SetPos(0, currentY);
	m_pAddBotPrefix->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pDrawTracers->SetPos(0, currentY);
	m_pDrawTracers->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pSpecInterpCamera->SetPos(0, currentY);
	m_pSpecInterpCamera->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pDisableShootingEffects->SetPos(0, currentY);
	m_pDisableShootingEffects->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 2: Viewmodel ==================
	currentY += sectionSpacing;

	m_pViewmodelLabel->SetPos(0, currentY);
	m_pViewmodelLabel->SetSize(contentWidth, controlHeight);
	m_pViewmodelLabel->SetContentAlignment(Label::a_west);
	currentY += controlHeight + spacing;

	// Viewmodel Preset
	m_pPresetLabel->SetPos(0, currentY);
	m_pPresetLabel->SetSize(labelWidth, controlHeight);
	m_pPresetLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelOffsetPreset->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelOffsetPreset->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset X
	m_pOffsetXLabel->SetPos(0, currentY);
	m_pOffsetXLabel->SetSize(labelWidth, controlHeight);
	m_pOffsetXLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelOffsetX->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelOffsetX->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetXLabel->SetPos(labelWidth + labelControlGap + controlWidth - sliderLabelWidth - spacing, currentY);
	m_pViewmodelOffsetXLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset Y
	m_pOffsetYLabel->SetPos(0, currentY);
	m_pOffsetYLabel->SetSize(labelWidth, controlHeight);
	m_pOffsetYLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelOffsetY->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelOffsetY->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetYLabel->SetPos(labelWidth + labelControlGap + controlWidth - sliderLabelWidth - spacing, currentY);
	m_pViewmodelOffsetYLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset Z
	m_pOffsetZLabel->SetPos(0, currentY);
	m_pOffsetZLabel->SetSize(labelWidth, controlHeight);
	m_pOffsetZLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelOffsetZ->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelOffsetZ->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetZLabel->SetPos(labelWidth + labelControlGap + controlWidth - sliderLabelWidth - spacing, currentY);
	m_pViewmodelOffsetZLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel FOV
	m_pFOVLabel->SetPos(0, currentY);
	m_pFOVLabel->SetSize(labelWidth, controlHeight);
	m_pFOVLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelFOV->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelFOV->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelFOVLabel->SetPos(labelWidth + labelControlGap + controlWidth - sliderLabelWidth - spacing, currentY);
	m_pViewmodelFOVLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Recoil
	m_pRecoilLabel->SetPos(0, currentY);
	m_pRecoilLabel->SetSize(labelWidth, controlHeight);
	m_pRecoilLabel->SetContentAlignment(Label::a_west);
	m_pViewmodelRecoil->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewmodelRecoil->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelRecoilLabel->SetPos(labelWidth + labelControlGap + controlWidth - sliderLabelWidth - spacing, currentY);
	m_pViewmodelRecoilLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 3: Movement ==================
	currentY += sectionSpacing;

	m_pMovementLabel->SetPos(0, currentY);
	m_pMovementLabel->SetSize(contentWidth, controlHeight);
	m_pMovementLabel->SetContentAlignment(Label::a_west);
	currentY += controlHeight + spacing;

	// Viewbob Style
	m_pViewbobLabel->SetPos(0, currentY);
	m_pViewbobLabel->SetSize(labelWidth, controlHeight);
	m_pViewbobLabel->SetContentAlignment(Label::a_west);
	m_pViewbobStyle->SetPos(labelWidth + labelControlGap, currentY);
	m_pViewbobStyle->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Weapon Position
	m_pWeaponPosLabel->SetPos(0, currentY);
	m_pWeaponPosLabel->SetSize(labelWidth, controlHeight);
	m_pWeaponPosLabel->SetContentAlignment(Label::a_west);
	m_pWeaponPos->SetPos(labelWidth + labelControlGap, currentY);
	m_pWeaponPos->SetSize(controlWidth, controlHeight);

	// Done - all controls are positioned inside scroll container
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubGameplay::~CModOptionsSubGameplay()
{
}

void CModOptionsSubGameplay::UpdateViewmodelSliderLabels()
{
	char strValue[8];
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetX->GetSliderValue() );
	m_pViewmodelOffsetXLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetY->GetSliderValue() );
	m_pViewmodelOffsetYLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelOffsetZ->GetSliderValue() );
	m_pViewmodelOffsetZLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelFOV->GetSliderValue() );
	m_pViewmodelFOVLabel->SetText( strValue );
	Q_snprintf( strValue, sizeof( strValue ), "%2.1f", m_pViewmodelRecoil->GetSliderValue() );
	m_pViewmodelRecoilLabel->SetText( strValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnControlModified()
{
	PostMessage( GetParent(), new KeyValues( "ApplyButtonEnable" ) );
	InvalidateLayout();
}

void CModOptionsSubGameplay::OnTextChanged( vgui::Panel *panel )
{
	if ( panel == m_pViewmodelOffsetPreset )
	{
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 0 )
		{
			m_pViewmodelOffsetX->SetSliderValue(1);
			m_pViewmodelOffsetY->SetSliderValue(1);
			m_pViewmodelOffsetZ->SetSliderValue(-1);
			m_pViewmodelFOV->SetSliderValue(60);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 1 )
		{
			m_pViewmodelOffsetX->SetSliderValue(0);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(0);
			m_pViewmodelFOV->SetSliderValue(54);
			UpdateViewmodelSliderLabels();
		}
		if ( m_pViewmodelOffsetPreset->GetActiveItem() == 2 )
		{
			m_pViewmodelOffsetX->SetSliderValue(2.5f);
			m_pViewmodelOffsetY->SetSliderValue(0);
			m_pViewmodelOffsetZ->SetSliderValue(-1.5f);
			m_pViewmodelFOV->SetSliderValue(68);
			UpdateViewmodelSliderLabels();
		}
	}
}

void CModOptionsSubGameplay::OnSliderMoved( KeyValues *data )
{
	vgui::Panel* pPanel = static_cast<vgui::Panel*>(data->GetPtr( "panel" ));

	if ( pPanel == m_pViewmodelOffsetX || pPanel == m_pViewmodelOffsetY || pPanel == m_pViewmodelOffsetZ || pPanel == m_pViewmodelFOV || pPanel == m_pViewmodelRecoil )
	{
		UpdateViewmodelSliderLabels();
	}
	else if ( pPanel == m_pVScrollBar )
	{
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnScrollBarSliderMoved( KeyValues *data )
{
	int position = data->GetInt( "position", 0 );
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnResetData()
{
	m_pCloseOnBuy->Reset();
	m_pUseOpensBuyMenu->Reset();
	m_pAddBotPrefix->Reset();
	m_pDrawTracers->Reset();
	m_pSpecInterpCamera->Reset();
	m_pDisableShootingEffects->Reset();
	m_pViewmodelOffsetX->Reset();
	m_pViewmodelOffsetY->Reset();
	m_pViewmodelOffsetZ->Reset();
	m_pViewmodelFOV->Reset();
	m_pViewmodelRecoil->Reset();
	UpdateViewmodelSliderLabels();
	
	ConVarRef viewmodel_presetpos( "viewmodel_presetpos" );
	if ( viewmodel_presetpos.IsValid() )
		m_pViewmodelOffsetPreset->SetInitialItem( viewmodel_presetpos.GetInt() - 1 );

	ConVarRef cl_use_new_headbob( "cl_use_new_headbob" );
	if ( cl_use_new_headbob.IsValid() )
		m_pViewbobStyle->SetInitialItem( cl_use_new_headbob.GetInt() );

	ConVarRef cl_righthand( "cl_righthand" );
	if ( cl_righthand.IsValid() )
		m_pWeaponPos->SetInitialItem( cl_righthand.GetInt() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubGameplay::OnApplyChanges()
{
	m_pCloseOnBuy->ApplyChanges();
	m_pUseOpensBuyMenu->ApplyChanges();
	m_pAddBotPrefix->ApplyChanges();
	m_pDrawTracers->ApplyChanges();
	m_pSpecInterpCamera->ApplyChanges();
	m_pDisableShootingEffects->ApplyChanges();
	m_pViewmodelOffsetPreset->ApplyChanges();
	m_pViewmodelOffsetX->ApplyChanges();
	m_pViewmodelOffsetY->ApplyChanges();
	m_pViewmodelOffsetZ->ApplyChanges();
	m_pViewmodelFOV->ApplyChanges();
	m_pViewmodelRecoil->ApplyChanges();
	m_pViewbobStyle->ApplyChanges();
	m_pWeaponPos->ApplyChanges();
}
