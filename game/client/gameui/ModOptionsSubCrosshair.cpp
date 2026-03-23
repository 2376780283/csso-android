//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubCrosshair.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/Cursor.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/FileOpenDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ScrollBar.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "cvarslider.h"
#include "LabeledCommandComboBox.h"
#include "filesystem.h"
#include "EngineInterface.h"
#include "BitmapImagePanel.h"
#include "tier1/utlbuffer.h"
#include "ModInfo.h"
#include "tier1/convar.h"
#include "tier0/icommandline.h"

#ifdef WIN32
#include <io.h>
#endif

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

struct ColorItem_t
{
	const char	*name;
	int			r, g, b;
};

static ColorItem_t s_crosshairColors[] = 
{
	{ "#Valve_Red",		250,	50,		50 },
	{ "#Valve_Green",	50,		250,	50 },
	{ "#Valve_Yellow",	250,	250,	50 },
	{ "#Valve_Blue",	50,		50,		250 },
	{ "#Valve_Ltblue",	50,		250,	250 },
	{ "#GameUI_Crosshair_Color_Custom", 0, 0, 0 }
};

static const char* crosshairBackgroundImages[] =
{
	"crosshair/de_dust2",
	"crosshair/de_mirage",
	"crosshair/de_aztec",
	"crosshair/cs_office",
};


//-----------------------------------------------------------------------------
class CrosshairImagePanelCS : public CrosshairImagePanelBase
{
	DECLARE_CLASS_SIMPLE( CrosshairImagePanelCS, CrosshairImagePanelBase );

public:
	CrosshairImagePanelCS( Panel *parent, const char *name, CModOptionsSubCrosshair* pOptionsPanel );
	virtual void ResetData();
	virtual void ApplyChanges();

	// Public access to controls for layout positioning
	CLabeledCommandComboBox	*m_pCrosshairStyle;
	CCvarSlider				*m_pCrosshairAlpha;
	CCvarToggleCheckButton	*m_pCrosshairUseAlpha;
	CCvarSlider				*m_pCrosshairGap;
	CCvarToggleCheckButton	*m_pCrosshairGapUseWeaponValue;
	CCvarSlider				*m_pCrosshairSize;
	CCvarSlider				*m_pCrosshairThickness;
	CCvarToggleCheckButton	*m_pCrosshairDot;
	CCvarSlider				*m_pCrosshairColorR;
	CCvarSlider				*m_pCrosshairColorG;
	CCvarSlider				*m_pCrosshairColorB;
	CCvarToggleCheckButton	*m_pCrosshairDrawOutline;
	CCvarSlider				*m_pCrosshairOutlineThickness;
	CCvarToggleCheckButton	*m_pCrosshairT;
	CLabeledCommandComboBox	*m_pCrosshairColor;

protected:
	MESSAGE_FUNC_PARAMS( OnSliderMoved, "SliderMoved", data );
	MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );

	virtual void Paint();
	void DrawCrosshairRect( int r, int g, int b, int a, int x0, int y0, int x1, int y1, bool bAdditive );
	void UpdateCrosshair();

private:
	CModOptionsSubCrosshair	*m_pOptionsPanel;
	int m_iCrosshairTextureID;
};

//-----------------------------------------------------------------------------
CrosshairImagePanelCS::CrosshairImagePanelCS( Panel *parent, const char *name, CModOptionsSubCrosshair* pOptionsPanel ) : CrosshairImagePanelBase( parent, name )
{
	m_pOptionsPanel = pOptionsPanel;

	// Get the scroll container from parent options panel
	Panel* pScrollContainer = pOptionsPanel->GetScrollContainer();

	// Create all controls inside the scroll container
	m_pCrosshairStyle = new CLabeledCommandComboBox( pScrollContainer, "CrosshairStyle" );
	m_pCrosshairAlpha = new CCvarSlider( pScrollContainer, "CrosshairAlpha", "#GameUI_Crosshair_Alpha", 0.0f, 255.0f, "cl_crosshairalpha" );
	m_pCrosshairUseAlpha = new CCvarToggleCheckButton( pScrollContainer, "CrosshairUseAlpha", "#GameUI_Crosshair_UseAlpha", "cl_crosshairusealpha" );
	m_pCrosshairGap = new CCvarSlider( pScrollContainer, "CrosshairGap", "#GameUI_Crosshair_Gap", -5.0f, 5.0f, "cl_crosshairgap" );
	m_pCrosshairGapUseWeaponValue = new CCvarToggleCheckButton( pScrollContainer, "CrosshairGapUseWeaponValue", "#GameUI_Crosshair_Gap_UseWeaponValue", "cl_crosshairgap_useweaponvalue" );
	m_pCrosshairSize = new CCvarSlider( pScrollContainer, "CrosshairSize", "#GameUI_Crosshair_Size", 0.0f, 10.0f, "cl_crosshairsize" );
	m_pCrosshairThickness = new CCvarSlider( pScrollContainer, "CrosshairThickness", "#GameUI_Crosshair_Thickness", 0.1f, 6.0f, "cl_crosshairthickness" );
	m_pCrosshairDot = new CCvarToggleCheckButton( pScrollContainer, "CrosshairDot", "#GameUI_CrosshairDot", "cl_crosshairdot" );
	m_pCrosshairColorR = new CCvarSlider( pScrollContainer, "CrosshairColorR", "#GameUI_Crosshair_Color_R", 0.0f, 255.0f, "cl_crosshaircolor_r" );
	m_pCrosshairColorG = new CCvarSlider( pScrollContainer, "CrosshairColorG", "#GameUI_Crosshair_Color_G", 0.0f, 255.0f, "cl_crosshaircolor_g" );
	m_pCrosshairColorB = new CCvarSlider( pScrollContainer, "CrosshairColorB", "#GameUI_Crosshair_Color_B", 0.0f, 255.0f, "cl_crosshaircolor_b" );
	m_pCrosshairDrawOutline = new CCvarToggleCheckButton( pScrollContainer, "CrosshairDrawOutline", "#GameUI_Crosshair_DrawOutline", "cl_crosshair_drawoutline" );
	m_pCrosshairOutlineThickness = new CCvarSlider( pScrollContainer, "CrosshairOutlineThickness", "#GameUI_Crosshair_OutlineThickness", 0.0f, 3.0f, "cl_crosshair_outlinethickness" );
	m_pCrosshairT = new CCvarToggleCheckButton( pScrollContainer, "CrosshairT", "#GameUI_Crosshair_T", "cl_crosshair_t" );
	m_pCrosshairColor = new CLabeledCommandComboBox( pScrollContainer, "CrosshairColor" );

	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_0", "cl_crosshairstyle 0" );
	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_1", "cl_crosshairstyle 1" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_2", "cl_crosshairstyle 2" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_3", "cl_crosshairstyle 3" );
	m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_4", "cl_crosshairstyle 4" );
	//m_pCrosshairStyle->AddItem( "#GameUI_Crosshair_Style_5", "cl_crosshairstyle 5" ); // deprecated in CSGO

	for ( int i = 0; i < ARRAYSIZE( s_crosshairColors ); i++ )
	{
		char command[64];
		Q_snprintf( command, sizeof( command ), "cl_crosshaircolor %d", i );
		m_pCrosshairColor->AddItem( s_crosshairColors[i].name, command );
	}

	m_pCrosshairStyle->AddActionSignalTarget( this );
	m_pCrosshairAlpha->AddActionSignalTarget( this );
	m_pCrosshairUseAlpha->AddActionSignalTarget( this );
	m_pCrosshairGap->AddActionSignalTarget( this );
	m_pCrosshairGapUseWeaponValue->AddActionSignalTarget( this );
	m_pCrosshairSize->AddActionSignalTarget( this );
	m_pCrosshairThickness->AddActionSignalTarget( this );
	m_pCrosshairDot->AddActionSignalTarget( this );
	m_pCrosshairColorR->AddActionSignalTarget( this );
	m_pCrosshairColorG->AddActionSignalTarget( this );
	m_pCrosshairColorB->AddActionSignalTarget( this );
	m_pCrosshairDrawOutline->AddActionSignalTarget( this );
	m_pCrosshairOutlineThickness->AddActionSignalTarget( this );
	m_pCrosshairT->AddActionSignalTarget( this );
	m_pCrosshairColor->AddActionSignalTarget( this );

	m_iCrosshairTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iCrosshairTextureID, "vgui/white_additive" , true, false);

	ResetData();
}

//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::DrawCrosshairRect( int r, int g, int b, int a, int x0, int y0, int x1, int y1, bool bAdditive )
{
	if ( m_pCrosshairDrawOutline->IsSelected() )
	{
		float flThick = m_pCrosshairOutlineThickness->GetSliderValue();
		vgui::surface()->DrawSetColor( 0, 0, 0, a );
		vgui::surface()->DrawFilledRect( x0-flThick, y0-flThick, x1+flThick, y1+flThick );
	}

	vgui::surface()->DrawSetColor( r, g, b, a );

	if ( bAdditive )
	{
		vgui::surface()->DrawTexturedRect( x0, y0, x1, y1 );
	}
	else
	{
		// Alpha-blended crosshair
		vgui::surface()->DrawFilledRect( x0, y0, x1, y1 );
	}
}

//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::Paint()
{
	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );;

	BaseClass::Paint();

	int wide, tall;
	GetSize( wide, tall );

	bool bAdditive = !m_pCrosshairUseAlpha->IsSelected();

	int a = 200;
	if ( !bAdditive )
		a = m_pCrosshairAlpha->GetSliderValue();

	int r, g, b;
	switch ( m_pCrosshairColor->GetActiveItem() )
	{
		case 0:	r = 250;	g = 50;		b = 50;		break;
		case 1:	r = 50;		g = 250;	b = 50;		break;
		case 2:	r = 250;	g = 250;	b = 50;		break;
		case 3:	r = 50;		g = 50;		b = 250;	break;
		case 4:	r = 50;		g = 250;	b = 250;	break;
		case 5:
			r = m_pCrosshairColorR->GetSliderValue();
			g = m_pCrosshairColorG->GetSliderValue();
			b = m_pCrosshairColorB->GetSliderValue();
			break;
		default:	r = 50;		g = 250;	b = 50;		break;
	}

	vgui::surface()->DrawSetColor( r, g, b, a );

	if ( bAdditive )
	{
		vgui::surface()->DrawSetTexture( m_iCrosshairTextureID );
	}

	int centerX = wide / 2;
	int centerY = tall / 2;

	int iBarSize = RoundFloatToInt(m_pCrosshairSize->GetSliderValue() * screenTall / 480.0f);
	int iBarThickness = max(1, RoundFloatToInt(m_pCrosshairThickness->GetSliderValue() * (float)screenTall / 480.0f));

	int iBarGap = m_pCrosshairGap->GetSliderValue() + 4;

	// draw horizontal crosshair lines
	int iInnerLeft = centerX - iBarGap - iBarThickness / 2;
	int iInnerRight = iInnerLeft + 2 * iBarGap + iBarThickness;
	int iOuterLeft = iInnerLeft - iBarSize;
	int iOuterRight = iInnerRight + iBarSize;
	int y0 = centerY - iBarThickness / 2;
	int y1 = y0 + iBarThickness;
	DrawCrosshairRect( r, g, b, a, iOuterLeft, y0, iInnerLeft, y1, bAdditive );
	DrawCrosshairRect( r, g, b, a, iInnerRight, y0, iOuterRight, y1, bAdditive );

	// draw vertical crosshair lines
	int iInnerTop = centerY - iBarGap - iBarThickness / 2;
	int iInnerBottom = iInnerTop + 2 * iBarGap + iBarThickness;
	int iOuterTop = iInnerTop - iBarSize;
	int iOuterBottom = iInnerBottom + iBarSize;
	int x0 = centerX - iBarThickness / 2;
	int x1 = x0 + iBarThickness;
	if ( !m_pCrosshairT->IsSelected() )
		DrawCrosshairRect( r, g, b, a, x0, iOuterTop, x1, iInnerTop, bAdditive );
	DrawCrosshairRect( r, g, b, a, x0, iInnerBottom, x1, iOuterBottom, bAdditive );

	// draw dot
	if ( m_pCrosshairDot->IsSelected() )
	{
		x0 = centerX - iBarThickness / 2;
		x1 = x0 + iBarThickness;
		y0 = centerY - iBarThickness / 2;
		y1 = y0 + iBarThickness;
		DrawCrosshairRect( r, g, b, a, x0, y0, x1, y1, bAdditive );
	}
}


//-----------------------------------------------------------------------------
// Purpose: takes the settings from the crosshair settings combo boxes and sliders
//          and apply it to the crosshair illustrations.
//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::UpdateCrosshair()
{
}


void CrosshairImagePanelCS::OnSliderMoved(KeyValues *data)
{
	m_pOptionsPanel->OnControlModified();

	UpdateCrosshair();
}


//-----------------------------------------------------------------------------
// Purpose: Called whenever color combo changes
//-----------------------------------------------------------------------------
void CrosshairImagePanelCS::OnTextChanged(vgui::Panel *panel)
{
	m_pCrosshairColorR->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pCrosshairColorG->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pCrosshairColorB->SetEnabled( m_pCrosshairColor->GetActiveItem() == 5 );
	m_pOptionsPanel->OnControlModified();
	UpdateCrosshair();
}

void CrosshairImagePanelCS::OnCheckButtonChecked()
{
	m_pCrosshairAlpha->SetEnabled(m_pCrosshairUseAlpha->IsSelected());
	m_pCrosshairOutlineThickness->SetEnabled(m_pCrosshairDrawOutline->IsSelected());
	m_pOptionsPanel->OnControlModified();
	UpdateCrosshair();
}

void CrosshairImagePanelCS::ResetData()
{
	ConVarRef cl_crosshairstyle( "cl_crosshairstyle" );
	m_pCrosshairStyle->SetInitialItem( Clamp( cl_crosshairstyle.GetInt() - 2, 0, 2 ) ); // remove Clamp if unlocking other styles

	m_pCrosshairAlpha->Reset();
	m_pCrosshairUseAlpha->Reset();
	m_pCrosshairGap->Reset();
	m_pCrosshairGapUseWeaponValue->Reset();
	m_pCrosshairSize->Reset();
	m_pCrosshairThickness->Reset();
	m_pCrosshairDot->Reset();
	m_pCrosshairColorR->Reset();
	m_pCrosshairColorG->Reset();
	m_pCrosshairColorB->Reset();
	m_pCrosshairDrawOutline->Reset();
	m_pCrosshairOutlineThickness->Reset();
	m_pCrosshairT->Reset();

	ConVarRef cl_crosshaircolor( "cl_crosshaircolor" );
	m_pCrosshairColor->SetInitialItem( cl_crosshaircolor.GetInt() );

	SetImage( crosshairBackgroundImages[RandomInt(0, ARRAYSIZE(crosshairBackgroundImages) - 1)] ); // bruh

	UpdateCrosshair();
}

void CrosshairImagePanelCS::ApplyChanges()
{
	m_pCrosshairStyle->ApplyChanges();
	m_pCrosshairAlpha->ApplyChanges();
	m_pCrosshairUseAlpha->ApplyChanges();
	m_pCrosshairGap->ApplyChanges();
	m_pCrosshairGapUseWeaponValue->ApplyChanges();
	m_pCrosshairSize->ApplyChanges();
	m_pCrosshairThickness->ApplyChanges();
	m_pCrosshairDot->ApplyChanges();
	m_pCrosshairColorR->ApplyChanges();
	m_pCrosshairColorG->ApplyChanges();
	m_pCrosshairColorB->ApplyChanges();
	m_pCrosshairDrawOutline->ApplyChanges();
	m_pCrosshairOutlineThickness->ApplyChanges();
	m_pCrosshairT->ApplyChanges();
	m_pCrosshairColor->ApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor - Fullscreen Style with C++ Layout and Scroll
//-----------------------------------------------------------------------------
CModOptionsSubCrosshair::CModOptionsSubCrosshair(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubCrosshair")
{
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

	// Create the crosshair preview image - in scroll container (right column)
	m_pCrosshairImage = new CrosshairImagePanelCS(m_pScrollContainer, "CrosshairImage", this);

	// Create labels for controls inside scroll container
	// Description label (from RES file)
	new vgui::Label(m_pScrollContainer, "CrosshairLabel", "#GameUI_CrosshairDescription");

	// Style section
	new vgui::Label(m_pScrollContainer, "StyleLabel", "#GameUI_Crosshair_Style");

	// Size section
	new vgui::Label(m_pScrollContainer, "SizeLabel", "#GameUI_Crosshair_Size");
	new vgui::Label(m_pScrollContainer, "ThicknessLabel", "#GameUI_Crosshair_Thickness");
	new vgui::Label(m_pScrollContainer, "GapLabel", "#GameUI_Crosshair_Gap");

	// Color section
	new vgui::Label(m_pScrollContainer, "CrosshairColorLabel", "#GameUI_Crosshair_Color");
	new vgui::Label(m_pScrollContainer, "RedLabel", "#GameUI_Crosshair_Color_R");
	new vgui::Label(m_pScrollContainer, "GreenLabel", "#GameUI_Crosshair_Color_G");
	new vgui::Label(m_pScrollContainer, "BlueLabel", "#GameUI_Crosshair_Color_B");

	// Alpha section
	new vgui::Label(m_pScrollContainer, "AlphaLabel", "#GameUI_Crosshair_Alpha");

	// Outline section
	new vgui::Label(m_pScrollContainer, "OutlineLabel", "#GameUI_Crosshair_OutlineThickness");
}

//-----------------------------------------------------------------------------
// Purpose: Perform layout - refined two-column scroll layout
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::PerformLayout()
{
	BaseClass::PerformLayout();

	// Get available size
	int pw = GetWide();
	int ph = GetTall();

	if (pw < 100 || ph < 100)
		return;

	// Layout parameters - increased sizes for better visibility
	int margin = PROPVAL(24);
	int spacing = PROPVAL(8);
	int controlHeight = PROPVAL(26);
	int sliderHeight = PROPVAL(30);
	int labelWidth = PROPVAL(110);
	int sliderLabelWidth = PROPVAL(16);
	int scrollBarWidth = PROPVAL(8);
	int columnGap = PROPVAL(20);

	// Two-column layout calculation
	int contentWidth = pw - (margin * 2) - scrollBarWidth;
	int columnWidth = (contentWidth - columnGap) / 2;
	int leftColumnX = 0;
	int rightColumnX = columnWidth + columnGap;

	// Crosshair preview area - in right column, at top of scroll area
	int previewSize = PROPVAL(120);

	// Scroll area starts from top
	int scrollAreaY = margin;
	int scrollAreaHeight = ph - margin * 2;

	// Position scroll bar on the right side
	m_pVScrollBar->SetPos(pw - margin - scrollBarWidth, scrollAreaY);
	m_pVScrollBar->SetSize(scrollBarWidth, scrollAreaHeight);

	// Position scroll container
	m_pScrollContainer->SetPos(margin, scrollAreaY);
	m_pScrollContainer->SetSize(contentWidth, scrollAreaHeight);

	// Calculate content heights for both columns
	int leftColumnHeight = 0;
	int rightColumnHeight = 0;

	// Right column starts with preview
	rightColumnHeight += previewSize + spacing * 3; // Preview + spacing

	// Left column items
	leftColumnHeight += controlHeight + spacing; // Style
	leftColumnHeight += sliderHeight + spacing; // Size
	leftColumnHeight += sliderHeight + spacing; // Thickness
	leftColumnHeight += sliderHeight + spacing; // Gap
	leftColumnHeight += controlHeight + spacing; // Gap Use Weapon

	// Right column items
	rightColumnHeight += controlHeight + spacing; // Color Combo
	rightColumnHeight += sliderHeight + spacing; // Color R
	rightColumnHeight += sliderHeight + spacing; // Color G
	rightColumnHeight += sliderHeight + spacing; // Color B
	rightColumnHeight += controlHeight + spacing; // Use Alpha
	rightColumnHeight += sliderHeight + spacing; // Alpha
	rightColumnHeight += controlHeight + spacing; // Dot
	rightColumnHeight += controlHeight + spacing; // T
	rightColumnHeight += controlHeight + spacing; // Draw Outline
	rightColumnHeight += sliderHeight + spacing; // Outline Thickness

	// Use the larger height for scroll
	int totalContentHeight = MAX(leftColumnHeight, rightColumnHeight) + margin;

	// Set scroll bar range
	m_pVScrollBar->SetRange(0, totalContentHeight);
	m_pVScrollBar->SetRangeWindow(scrollAreaHeight);

	// Get scroll offset
	int scrollOffset = m_pVScrollBar->GetValue();

	// ================== LEFT COLUMN ==================
	int currentY = -scrollOffset + spacing * 2;

	// Crosshair Description Label (from RES file: x=16, y=4)
	Panel* pCrosshairLabel = m_pScrollContainer->FindChildByName("CrosshairLabel");
	if (pCrosshairLabel)
	{
		pCrosshairLabel->SetVisible(true);
		pCrosshairLabel->SetPos(leftColumnX, currentY - spacing * 2 + PROPVAL(4));
		pCrosshairLabel->SetSize(PROPVAL(128), controlHeight);
	}

	// Style (Combo)
	Panel* pStyleLabel = m_pScrollContainer->FindChildByName("StyleLabel");
	if (pStyleLabel)
	{
		pStyleLabel->SetVisible(true);
		pStyleLabel->SetPos(leftColumnX, currentY);
		pStyleLabel->SetSize(labelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairStyle->SetPos(leftColumnX + labelWidth + spacing, currentY);
	m_pCrosshairImage->m_pCrosshairStyle->SetSize(columnWidth - labelWidth - spacing, controlHeight);
	currentY += controlHeight + spacing;

	// Size
	Panel* pSizeLabel = m_pScrollContainer->FindChildByName("SizeLabel");
	if (pSizeLabel)
	{
		pSizeLabel->SetVisible(true);
		pSizeLabel->SetPos(leftColumnX, currentY);
		pSizeLabel->SetSize(labelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairSize->SetPos(leftColumnX + labelWidth + spacing, currentY);
	m_pCrosshairImage->m_pCrosshairSize->SetSize(columnWidth - labelWidth - spacing, sliderHeight);
	currentY += sliderHeight + spacing;

	// Thickness
	Panel* pThicknessLabel = m_pScrollContainer->FindChildByName("ThicknessLabel");
	if (pThicknessLabel)
	{
		pThicknessLabel->SetVisible(true);
		pThicknessLabel->SetPos(leftColumnX, currentY);
		pThicknessLabel->SetSize(labelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairThickness->SetPos(leftColumnX + labelWidth + spacing, currentY);
	m_pCrosshairImage->m_pCrosshairThickness->SetSize(columnWidth - labelWidth - spacing, sliderHeight);
	currentY += sliderHeight + spacing;

	// Gap
	Panel* pGapLabel = m_pScrollContainer->FindChildByName("GapLabel");
	if (pGapLabel)
	{
		pGapLabel->SetVisible(true);
		pGapLabel->SetPos(leftColumnX, currentY);
		pGapLabel->SetSize(labelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairGap->SetPos(leftColumnX + labelWidth + spacing, currentY);
	m_pCrosshairImage->m_pCrosshairGap->SetSize(columnWidth - labelWidth - spacing, sliderHeight);
	currentY += sliderHeight + spacing;

	// Gap Use Weapon Value
	m_pCrosshairImage->m_pCrosshairGapUseWeaponValue->SetPos(leftColumnX + labelWidth + spacing, currentY);
	m_pCrosshairImage->m_pCrosshairGapUseWeaponValue->SetSize(columnWidth - labelWidth - spacing, controlHeight);

	// ================== RIGHT COLUMN ==================
	int rightY = -scrollOffset + spacing * 2;

	// Crosshair Preview (centered in right column)
	m_pCrosshairImage->SetBounds(rightColumnX + (columnWidth - previewSize) / 2, rightY, previewSize, previewSize);
	rightY += previewSize + spacing * 3;

	// Color Label
	Panel* pColorLabel = m_pScrollContainer->FindChildByName("CrosshairColorLabel");
	if (pColorLabel)
	{
		pColorLabel->SetVisible(true);
		pColorLabel->SetPos(rightColumnX, rightY);
		pColorLabel->SetSize(labelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairColor->SetPos(rightColumnX + labelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairColor->SetSize(columnWidth - labelWidth - spacing, controlHeight);
	rightY += controlHeight + spacing;

	// Color R
	Panel* pRedLabel = m_pScrollContainer->FindChildByName("RedLabel");
	if (pRedLabel)
	{
		pRedLabel->SetVisible(true);
		pRedLabel->SetPos(rightColumnX, rightY);
		pRedLabel->SetSize(sliderLabelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorR->SetPos(rightColumnX + sliderLabelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairColorR->SetSize(columnWidth - sliderLabelWidth - spacing, sliderHeight);
	rightY += sliderHeight + spacing;

	// Color G
	Panel* pGreenLabel = m_pScrollContainer->FindChildByName("GreenLabel");
	if (pGreenLabel)
	{
		pGreenLabel->SetVisible(true);
		pGreenLabel->SetPos(rightColumnX, rightY);
		pGreenLabel->SetSize(sliderLabelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorG->SetPos(rightColumnX + sliderLabelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairColorG->SetSize(columnWidth - sliderLabelWidth - spacing, sliderHeight);
	rightY += sliderHeight + spacing;

	// Color B
	Panel* pBlueLabel = m_pScrollContainer->FindChildByName("BlueLabel");
	if (pBlueLabel)
	{
		pBlueLabel->SetVisible(true);
		pBlueLabel->SetPos(rightColumnX, rightY);
		pBlueLabel->SetSize(sliderLabelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairColorB->SetPos(rightColumnX + sliderLabelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairColorB->SetSize(columnWidth - sliderLabelWidth - spacing, sliderHeight);
	rightY += sliderHeight + spacing;

	// Use Alpha
	m_pCrosshairImage->m_pCrosshairUseAlpha->SetPos(rightColumnX, rightY);
	m_pCrosshairImage->m_pCrosshairUseAlpha->SetSize(columnWidth, controlHeight);
	rightY += controlHeight + spacing;

	// Alpha
	Panel* pAlphaLabel = m_pScrollContainer->FindChildByName("AlphaLabel");
	if (pAlphaLabel)
	{
		pAlphaLabel->SetVisible(true);
		pAlphaLabel->SetPos(rightColumnX, rightY);
		pAlphaLabel->SetSize(sliderLabelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairAlpha->SetPos(rightColumnX + sliderLabelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairAlpha->SetSize(columnWidth - sliderLabelWidth - spacing, sliderHeight);
	rightY += sliderHeight + spacing;

	// Dot
	m_pCrosshairImage->m_pCrosshairDot->SetPos(rightColumnX, rightY);
	m_pCrosshairImage->m_pCrosshairDot->SetSize(columnWidth, controlHeight);
	rightY += controlHeight + spacing;

	// T (crosshair center)
	m_pCrosshairImage->m_pCrosshairT->SetPos(rightColumnX, rightY);
	m_pCrosshairImage->m_pCrosshairT->SetSize(columnWidth, controlHeight);
	rightY += controlHeight + spacing;

	// Draw Outline
	m_pCrosshairImage->m_pCrosshairDrawOutline->SetPos(rightColumnX, rightY);
	m_pCrosshairImage->m_pCrosshairDrawOutline->SetSize(columnWidth, controlHeight);
	rightY += controlHeight + spacing;

	// Outline Thickness
	Panel* pOutlineLabel = m_pScrollContainer->FindChildByName("OutlineLabel");
	if (pOutlineLabel)
	{
		pOutlineLabel->SetVisible(true);
		pOutlineLabel->SetPos(rightColumnX, rightY);
		pOutlineLabel->SetSize(sliderLabelWidth, controlHeight);
	}
	m_pCrosshairImage->m_pCrosshairOutlineThickness->SetPos(rightColumnX + sliderLabelWidth + spacing, rightY);
	m_pCrosshairImage->m_pCrosshairOutlineThickness->SetSize(columnWidth - sliderLabelWidth - spacing, sliderHeight);

	// Update visibility
	if (m_pCrosshairImage)
		m_pCrosshairImage->UpdateVisibility();
}

//-----------------------------------------------------------------------------
// Purpose: Handle scroll bar movement
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnScrollBarSliderMoved(KeyValues *data)
{
	int position = data->GetInt("position", 0);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubCrosshair::~CModOptionsSubCrosshair()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define SUIT_HUE_START 192
#define SUIT_HUE_END 223
#define PLATE_HUE_START 160
#define PLATE_HUE_END 191

#ifdef POSIX 
typedef struct tagRGBQUAD { 
  uint8 rgbBlue;
  uint8 rgbGreen;
  uint8 rgbRed;
  uint8 rgbReserved;
} RGBQUAD;
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void PaletteHueReplace( RGBQUAD *palSrc, int newHue, int Start, int end )
{
	int i;
	float r, b, g;
	float maxcol, mincol;
	float hue, val, sat;

	hue = (float)(newHue * (360.0 / 255));

	for (i = Start; i <= end; i++)
	{
		b = palSrc[ i ].rgbBlue;
		g = palSrc[ i ].rgbGreen;
		r = palSrc[ i ].rgbRed;
		
		maxcol = max( max( r, g ), b ) / 255.0f;
		mincol = min( min( r, g ), b ) / 255.0f;
		
		val = maxcol;
		sat = (maxcol - mincol) / maxcol;

		mincol = val * (1.0f - sat);

		if (hue <= 120)
		{
			b = mincol;
			if (hue < 60)
			{
				r = val;
				g = mincol + hue * (val - mincol)/(120 - hue);
			}
			else
			{
				g = val;
				r = mincol + (120 - hue)*(val-mincol)/hue;
			}
		}
		else if (hue <= 240)
		{
			r = mincol;
			if (hue < 180)
			{
				g = val;
				b = mincol + (hue - 120)*(val-mincol)/(240 - hue);
			}
			else
			{
				b = val;
				g = mincol + (240 - hue)*(val-mincol)/(hue - 120);
			}
		}
		else
		{
			g = mincol;
			if (hue < 300)
			{
				b = val;
				r = mincol + (hue - 240)*(val-mincol)/(360 - hue);
			}
			else
			{
				r = val;
				b = mincol + (360 - hue)*(val-mincol)/(hue - 240);
			}
		}

		palSrc[ i ].rgbBlue = (unsigned char)(b * 255);
		palSrc[ i ].rgbGreen = (unsigned char)(g * 255);
		palSrc[ i ].rgbRed = (unsigned char)(r * 255);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnResetData()
{
	if ( m_pCrosshairImage )
		m_pCrosshairImage->ResetData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubCrosshair::OnApplyChanges()
{
	if ( m_pCrosshairImage != NULL )
		m_pCrosshairImage->ApplyChanges();
}
