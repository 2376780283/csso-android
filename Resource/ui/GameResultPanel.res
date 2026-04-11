"GameResultPanel"
{
    "ControlName"    "Frame"
    "fieldName"    "GameResultPanel"
    "xpos"        "0"
    "ypos"        "0"
    "wide"        "f0"
    "tall"        "f0"
    "autoResize"    "0"
    "pinCorner"    "0"
    "visible"    "1"
    "enabled"    "1"
    "tabPosition"    "0"
    "setTitleBarVisible"    "1"
    "title"    ""
    "PaintBackgroundType"    "0"

    // 主背景 - 半透明黑色
    "BGBorder"
    {
        "ControlName"    "Label"
        "fieldName"    "BGBorder"
        "xpos"        "c-400"
        "ypos"        "c-300"
        "wide"        "800"
        "tall"        "600"
        "autoResize"    "0"
        "pinCorner"    "0"
        "visible"    "1"
        "enabled"    "1"
        "tabPosition"    "0"
        "labelText"    ""
        "PaintBackgroundType"    "2"
        "bgcolor"    "0 0 0 200"
    }

    // 标题区域 - 胜利/失败
    "ResultLabel"
    {
        "ControlName"    "Label"
        "fieldName"    "ResultLabel"
        "xpos"        "c-300"
        "ypos"        "c-250"
        "wide"        "600"
        "tall"        "60"
        "autoResize"    "0"
        "pinCorner"    "0"
        "visible"    "1"
        "enabled"    "1"
        "tabPosition"    "0"
        "labelText"    ""
        "textAlignment"    "center"
        "font"        "HudNumbers"
        "fgcolor"    "255 255 255 255"
    }

    // 玩家面板容器 - 5个玩家模型
    "PlayerPanelContainer"
    {
        "ControlName"    "EditablePanel"
        "fieldName"    "PlayerPanelContainer"
        "xpos"        "c-350"
        "ypos"        "c-150"
        "wide"        "700"
        "tall"        "300"
        "autoResize"    "0"
        "pinCorner"    "0"
        "visible"    "1"
        "enabled"    "1"
        "tabPosition"    "0"

        // 左侧第2个玩家
        "PlayerPanel0"
        {
            "ControlName"    "EditablePanel"
            "fieldName"    "PlayerPanel0"
            "xpos"        "0"
            "ypos"        "50"
            "wide"        "140"
            "tall"        "250"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
        }

        // 左侧第1个玩家
        "PlayerPanel1"
        {
            "ControlName"    "EditablePanel"
            "fieldName"    "PlayerPanel1"
            "xpos"        "140"
            "ypos"        "50"
            "wide"        "140"
            "tall"        "250"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
        }

        // 中间玩家 (本地玩家)
        "PlayerPanel2"
        {
            "ControlName"    "EditablePanel"
            "fieldName"    "PlayerPanel2"
            "xpos"        "280"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "300"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "border"    "TeamBorder"
        }

        // 右侧第1个玩家
        "PlayerPanel3"
        {
            "ControlName"    "EditablePanel"
            "fieldName"    "PlayerPanel3"
            "xpos"        "420"
            "ypos"        "50"
            "wide"        "140"
            "tall"        "250"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
        }

        // 右侧第2个玩家
        "PlayerPanel4"
        {
            "ControlName"    "EditablePanel"
            "fieldName"    "PlayerPanel4"
            "xpos"        "560"
            "ypos"        "50"
            "wide"        "140"
            "tall"        "250"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
        }
    }

    // 玩家名称标签
    "PlayerLabelContainer"
    {
        "ControlName"    "EditablePanel"
        "fieldName"    "PlayerLabelContainer"
        "xpos"        "c-350"
        "ypos"        "c150"
        "wide"        "700"
        "tall"        "30"
        "autoResize"    "0"
        "pinCorner"    "0"
        "visible"    "1"
        "enabled"    "1"
        "tabPosition"    "0"

        "PlayerLabel0"
        {
            "ControlName"    "Label"
            "fieldName"    "PlayerLabel0"
            "xpos"        "0"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "30"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "labelText"    ""
            "textAlignment"    "center"
            "font"        "Default"
            "fgcolor"    "255 255 255 255"
        }

        "PlayerLabel1"
        {
            "ControlName"    "Label"
            "fieldName"    "PlayerLabel1"
            "xpos"        "140"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "30"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "labelText"    ""
            "textAlignment"    "center"
            "font"        "Default"
            "fgcolor"    "255 255 255 255"
        }

        "PlayerLabel2"
        {
            "ControlName"    "Label"
            "fieldName"    "PlayerLabel2"
            "xpos"        "280"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "30"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "labelText"    ""
            "textAlignment"    "center"
            "font"        "Default"
            "fgcolor"    "255 255 255 255"
        }

        "PlayerLabel3"
        {
            "ControlName"    "Label"
            "fieldName"    "PlayerLabel3"
            "xpos"        "420"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "30"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "labelText"    ""
            "textAlignment"    "center"
            "font"        "Default"
            "fgcolor"    "255 255 255 255"
        }

        "PlayerLabel4"
        {
            "ControlName"    "Label"
            "fieldName"    "PlayerLabel4"
            "xpos"        "560"
            "ypos"        "0"
            "wide"        "140"
            "tall"        "30"
            "autoResize"    "0"
            "pinCorner"    "0"
            "visible"    "1"
            "enabled"    "1"
            "tabPosition"    "0"
            "labelText"    ""
            "textAlignment"    "center"
            "font"        "Default"
            "fgcolor"    "255 255 255 255"
        }
    }

    // 继续按钮
    "ContinueButton"
    {
        "ControlName"    "Button"
        "fieldName"    "ContinueButton"
        "xpos"        "c-100"
        "ypos"        "c220"
        "wide"        "200"
        "tall"        "40"
        "autoResize"    "0"
        "pinCorner"    "0"
        "visible"    "1"
        "enabled"    "1"
        "tabPosition"    "0"
        "labelText"    "#GameUI_Continue"
        "textAlignment"    "center"
        "font"        "MenuLarge"
        "command"    "continue"
        "sound_depressed"    "UI/buttonclick.wav"
        "sound_released"    "UI/buttonclickrelease.wav"
        
        "defaultBgColor_override"    "0 0 0 150"
        "armedBgColor_override"    "200 0 0 200"
        "depressedBgColor_override"    "100 0 0 200"
        "defaultFgColor_override"    "255 255 255 255"
        "armedFgColor_override"    "255 255 255 255"
        "depressedFgColor_override"    "255 255 255 255"
    }
}
