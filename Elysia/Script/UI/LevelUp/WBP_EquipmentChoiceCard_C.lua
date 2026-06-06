--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_EquipmentChoiceCard_C
local M = UnLua.Class()

local function TextToString(Text)
    if not Text then
        return ""
    end

    if UE.UKismetTextLibrary.Conv_TextToString then
        return UE.UKismetTextLibrary.Conv_TextToString(Text)
    end

    return tostring(Text)
end

local function StringToText(String)
    if UE.UKismetTextLibrary.Conv_StringToText then
        return UE.UKismetTextLibrary.Conv_StringToText(String or "")
    end

    return String or ""
end

local function IsDisplayType(DisplayData, DisplayType)
    return DisplayData and DisplayData.DisplayType == DisplayType
end

local function GetCardStyleIndex(CardStyle)
    if CardStyle == UE.EElysiaEquipmentChoiceCardStyle.Weapon then
        return 0
    end

    if CardStyle == UE.EElysiaEquipmentChoiceCardStyle.Evolution then
        return 2
    end

    return 1
end

function M:Construct()
    self.ChoiceIndex = -1
    self.bIsHovered = false

    if self.Button_Select then
        self.Button_Select.OnClicked:Add(self, self.HandleClicked)
        self.Button_Select.OnHovered:Add(self, self.HandleHovered)
        self.Button_Select.OnUnhovered:Add(self, self.HandleUnhovered)
        self.Button_Select.OnPressed:Add(self, self.HandlePressed)
        self.Button_Select.OnReleased:Add(self, self.HandleReleased)
    end

    self:ClearChoiceData()
end

function M:SetOwnerLevelUpWidget(OwnerLevelUpWidget)
    self.OwnerLevelUpWidget = OwnerLevelUpWidget
end

function M:SetChoiceData(DisplayData)
    if not DisplayData then
        self:ClearChoiceData()
        return
    end

    self.DisplayData = DisplayData
    self.ChoiceIndex = DisplayData.ChoiceIndex
    self.bIsHovered = false
    self:SetVisibility(UE.ESlateVisibility.Visible)
    self:ApplyCardStyle(DisplayData.CardStyle)
    self:ApplyChoiceContent(DisplayData)
    self:HideInteractionLayers()
end

function M:ClearChoiceData()
    self.DisplayData = nil
    self.ChoiceIndex = -1
    self.bIsHovered = false
    self:HideInteractionLayers()
    self:SetVisibility(UE.ESlateVisibility.Collapsed)
end

function M:ApplyCardStyle(CardStyle)
    local CardStyleIndex = GetCardStyleIndex(CardStyle)

    if self.WidgetSwitcher_Background then
        self.WidgetSwitcher_Background:SetActiveWidgetIndex(CardStyleIndex)
    end

    if self.WidgetSwitcher_Hovered then
        self.WidgetSwitcher_Hovered:SetActiveWidgetIndex(CardStyleIndex)
    end

    if self.WidgetSwitcher_Pressed then
        self.WidgetSwitcher_Pressed:SetActiveWidgetIndex(CardStyleIndex)
    end
end

function M:ApplyChoiceContent(DisplayData)
    if self.Image_Icon and DisplayData.Icon then
        self.Image_Icon:SetBrushFromTexture(DisplayData.Icon)
    end

    if self.TextBlock_NameLevel then
        self.TextBlock_NameLevel:SetText(StringToText(self:BuildNameLevelText(DisplayData)))
    end

    if self.TextBlock_Description then
        self.TextBlock_Description:SetText(StringToText(self:BuildDescriptionText(DisplayData)))
    end
end

function M:BuildNameLevelText(DisplayData)
    if IsDisplayType(DisplayData, UE.EElysiaEquipmentChoiceDisplayType.Recovery) then
        return TextToString(DisplayData.TypeDisplayName)
    end

    local DisplayName = TextToString(DisplayData.DisplayName)
    if DisplayData.bShowLevelText then
        return string.format("%s:%d级", DisplayName, DisplayData.NextLevel)
    end

    return DisplayName
end

function M:BuildDescriptionText(DisplayData)
    if IsDisplayType(DisplayData, UE.EElysiaEquipmentChoiceDisplayType.Recovery) then
        return string.format("恢复%.0f点生命值", DisplayData.RecoveryHealth)
    end

    local Lines = {}
    local Description = TextToString(DisplayData.Description)
    if Description ~= "" then
        table.insert(Lines, Description)
    end

    if DisplayData.bHasEvolutionRequirement then
        local RequiredName = TextToString(DisplayData.EvolutionRequiredDisplayName)
        if RequiredName ~= "" then
            local RequirementState = DisplayData.bEvolutionRequirementMet and "已拥有" or "未拥有"
            table.insert(Lines, string.format("进化组合：%s（%s）", RequiredName, RequirementState))
        end
    end

    return table.concat(Lines, "\n")
end

function M:HideInteractionLayers()
    if self.WidgetSwitcher_Hovered then
        self.WidgetSwitcher_Hovered:SetVisibility(UE.ESlateVisibility.Hidden)
    end

    if self.WidgetSwitcher_Pressed then
        self.WidgetSwitcher_Pressed:SetVisibility(UE.ESlateVisibility.Hidden)
    end
end

function M:ShowHoveredLayer()
    if self.WidgetSwitcher_Hovered then
        self.WidgetSwitcher_Hovered:SetVisibility(UE.ESlateVisibility.Visible)
    end
end

function M:ShowPressedLayer()
    if self.WidgetSwitcher_Pressed then
        self.WidgetSwitcher_Pressed:SetVisibility(UE.ESlateVisibility.Visible)
    end
end

function M:HandleClicked()
    if self.OwnerLevelUpWidget and self.ChoiceIndex and self.ChoiceIndex >= 0 then
        self.OwnerLevelUpWidget:HandleChoiceCardSelected(self.ChoiceIndex)
    end
end

function M:HandleHovered()
    self.bIsHovered = true
    self:ShowHoveredLayer()
end

function M:HandleUnhovered()
    self.bIsHovered = false
    self:HideInteractionLayers()
end

function M:HandlePressed()
    self:ShowPressedLayer()
end

function M:HandleReleased()
    if self.WidgetSwitcher_Pressed then
        self.WidgetSwitcher_Pressed:SetVisibility(UE.ESlateVisibility.Hidden)
    end

    if self.bIsHovered then
        self:ShowHoveredLayer()
    end
end

return M
