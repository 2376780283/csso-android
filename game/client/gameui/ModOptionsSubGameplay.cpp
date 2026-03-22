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

	// Create all controls
	m_pCloseOnBuy = new CCvarToggleCheckButton(this, "CloseOnBuyCheckbox", "#GameUI_Gameplay_CloseOnBuy", "closeonbuy");
	m_pUseOpensBuyMenu = new CCvarToggleCheckButton(this, "UseOpensBuyMenuCheckbox", "#GameUI_Gameplay_UseOpensBuyMenu", "cl_use_opens_buy_menu");
	m_pAddBotPrefix = new CCvarToggleCheckButton(this, "AddBotPrefix", "#GameUI_Gameplay_AddBotPrefix", "cl_add_bot_prefix");
	m_pDrawTracers = new CCvarToggleCheckButton(this, "DrawTracers", "#GameUI_Gameplay_DrawTracers", "r_drawtracers");
	m_pSpecInterpCamera = new CCvarToggleCheckButton(this, "SpecInterpCamera", "#GameUI_Gameplay_SpecInterpCamera", "cl_obs_interp_enable");
	m_pDisableShootingEffects = new CCvarToggleCheckButton(this, "DisableShootingEffects", "#GameUI_Gameplay_DisableShootingEffects", "cl_disable_shooting_effects");

	m_pViewmodelOffsetX = new CCvarSlider(this, "ViewmodelOffsetXSlider", "", -2.0f, 2.5f, "viewmodel_offset_x");
	m_pViewmodelOffsetXLabel = new Label(this, "ViewmodelOffsetXLabel", "0.0");
	m_pViewmodelOffsetY = new CCvarSlider(this, "ViewmodelOffsetYSlider", "", -2.0f, 2.0f, "viewmodel_offset_y");
	m_pViewmodelOffsetYLabel = new Label(this, "ViewmodelOffsetYLabel", "0.0");
	m_pViewmodelOffsetZ = new CCvarSlider(this, "ViewmodelOffsetZSlider", "", -2.0f, 2.0f, "viewmodel_offset_z");
	m_pViewmodelOffsetZLabel = new Label(this, "ViewmodelOffsetZLabel", "0.0");
	m_pViewmodelOffsetPreset = new CLabeledCommandComboBox(this, "ViewmodelOffsetPreset");
	m_pViewmodelFOV = new CCvarSlider(this, "ViewmodelFOVSlider", "", 54.0f, 68.0f, "viewmodel_fov");
	m_pViewmodelFOVLabel = new Label(this, "ViewmodelFOVLabel", "60");
	m_pViewmodelRecoil = new CCvarSlider(this, "ViewmodelRecoilSlider", "", 0.0f, 1.0f, "viewmodel_recoil");
	m_pViewmodelRecoilLabel = new Label(this, "ViewmodelRecoilLabel", "0.0");
	m_pViewbobStyle = new CLabeledCommandComboBox(this, "ViewbobStyleComboBox");
	m_pWeaponPos = new CLabeledCommandComboBox(this, "WeaponPositionComboBox");

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

	// Calculate content width
	int contentWidth = pw - (margin * 2);
	int controlWidth = contentWidth - labelWidth - labelControlGap;

	int currentY = margin;

	// ================== SECTION 1: General Settings ==================
	// We'll draw section labels directly using SetPos in OnPaint or create them
	currentY += controlHeight + spacing;

	m_pCloseOnBuy->SetPos(margin, currentY);
	m_pCloseOnBuy->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pUseOpensBuyMenu->SetPos(margin, currentY);
	m_pUseOpensBuyMenu->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pAddBotPrefix->SetPos(margin, currentY);
	m_pAddBotPrefix->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pDrawTracers->SetPos(margin, currentY);
	m_pDrawTracers->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pSpecInterpCamera->SetPos(margin, currentY);
	m_pSpecInterpCamera->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	m_pDisableShootingEffects->SetPos(margin, currentY);
	m_pDisableShootingEffects->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 2: Viewmodel ==================
	currentY += sectionSpacing;

	// Viewmodel Preset
	m_pViewmodelOffsetPreset->SetPos(margin, currentY);
	m_pViewmodelOffsetPreset->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset X
	m_pViewmodelOffsetX->SetPos(margin, currentY);
	m_pViewmodelOffsetX->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetXLabel->SetPos(margin + controlWidth - sliderLabelWidth - spacing + spacing, currentY);
	m_pViewmodelOffsetXLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset Y
	m_pViewmodelOffsetY->SetPos(margin, currentY);
	m_pViewmodelOffsetY->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetYLabel->SetPos(margin + controlWidth - sliderLabelWidth - spacing + spacing, currentY);
	m_pViewmodelOffsetYLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Offset Z
	m_pViewmodelOffsetZ->SetPos(margin, currentY);
	m_pViewmodelOffsetZ->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelOffsetZLabel->SetPos(margin + controlWidth - sliderLabelWidth - spacing + spacing, currentY);
	m_pViewmodelOffsetZLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel FOV
	m_pViewmodelFOV->SetPos(margin, currentY);
	m_pViewmodelFOV->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelFOVLabel->SetPos(margin + controlWidth - sliderLabelWidth - spacing + spacing, currentY);
	m_pViewmodelFOVLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Viewmodel Recoil
	m_pViewmodelRecoil->SetPos(margin, currentY);
	m_pViewmodelRecoil->SetSize(controlWidth - sliderLabelWidth - spacing, controlHeight);
	m_pViewmodelRecoilLabel->SetPos(margin + controlWidth - sliderLabelWidth - spacing + spacing, currentY);
	m_pViewmodelRecoilLabel->SetSize(sliderLabelWidth, controlHeight);
	currentY += controlHeight + spacing;

	// ================== SECTION 3: Movement ==================
	currentY += sectionSpacing;

	// Viewbob Style
	m_pViewbobStyle->SetPos(margin, currentY);
	m_pViewbobStyle->SetSize(controlWidth, controlHeight);
	currentY += controlHeight + spacing;

	// Weapon Position
	m_pWeaponPos->SetPos(margin, currentY);
	m_pWeaponPos->SetSize(controlWidth, controlHeight);
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
