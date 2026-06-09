--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_LevelUp_C
local M = UnLua.Class()

function M:TrySetLevelUpPaused(bPaused)
    local PlayerController = self:GetOwningPlayer()
    if PlayerController and PlayerController.TrySetLevelUpPaused then
        return PlayerController:TrySetLevelUpPaused(bPaused)
    end

    return UE.UGameplayStatics.SetGamePaused(self, bPaused)
end

function M:WidgetControllerSet()
    self.LevelUpWidgetController = UE.UElysiaLevelUpWidgetController.Cast(self.WidgetController, UE.UElysiaLevelUpWidgetController.StaticClass())
    self.LevelUpWidgetController.OnEquipmentChoicesChanged:Add(self, self.HandleEquipmentChoicesChange)
    self.LevelUpWidgetController.OnEquipmentInventoryChanged:Add(self, self.HandleEquipmentInventoryChange)

    self:SetupChoiceCards()

    -- 显示初始装备
    self:HandleEquipmentChoicesChange()
end

function M:EnterLevelUpState()
    if self.bLevelUpStateActive then
        return
    end

    self.bPausedForLevelUp = false
    self.bUsingExclusiveLevelUpInput = false

    self.bPausedForLevelUp = self:TrySetLevelUpPaused(true)

    local PlayerController = self:GetOwningPlayer()
    if PlayerController and self.bPausedForLevelUp then
        PlayerController:SetIgnoreMoveInput(true)
        UE.UWidgetBlueprintLibrary.SetInputMode_UIOnlyEx(PlayerController, self, UE.EMouseLockMode.DoNotLock, true)
        self.bUsingExclusiveLevelUpInput = true
    end

    self.bLevelUpStateActive = true
end

function M:ExitLevelUpState()
    if not self.bLevelUpStateActive then
        return
    end

    local PlayerController = self:GetOwningPlayer()
    if self.bPausedForLevelUp then
        self:TrySetLevelUpPaused(false)
    end

    if PlayerController and self.bUsingExclusiveLevelUpInput then
        PlayerController:ResetIgnoreMoveInput()
        UE.UWidgetBlueprintLibrary.SetInputMode_GameAndUIEx(PlayerController, self, UE.EMouseLockMode.DoNotLock, false, true)
    end

    self.bPausedForLevelUp = false
    self.bUsingExclusiveLevelUpInput = false
    self.bLevelUpStateActive = false
end

function M:SetupChoiceCards()
    self.ChoiceCards = {
        self.WBP_EquipmentChoiceCard_1,
        self.WBP_EquipmentChoiceCard_2,
        self.WBP_EquipmentChoiceCard_3,
    }

    for _, Card in ipairs(self.ChoiceCards) do
        if Card and Card.SetOwnerLevelUpWidget then
            Card:SetOwnerLevelUpWidget(self)
        end
    end
end

function M:GetChoiceDisplayData(ChoicePosition)
    if not self.LevelUpWidgetController or not self.LevelUpWidgetController.CurrentEquipmentChoiceDisplays then
        return nil
    end

    local ChoiceDisplays = self.LevelUpWidgetController.CurrentEquipmentChoiceDisplays
    if ChoicePosition > ChoiceDisplays:Length() then
        return nil
    end

    return ChoiceDisplays:Get(ChoicePosition)
end

function M:UpdateChoiceCards()
    if not self.ChoiceCards then
        self:SetupChoiceCards()
    end

    for ChoicePosition, Card in ipairs(self.ChoiceCards) do
        if Card and Card.SetChoiceData then
            Card:SetChoiceData(self:GetChoiceDisplayData(ChoicePosition))
        end
    end
end

function M:ClearChoiceCards()
    if not self.ChoiceCards then
        return
    end

    for _, Card in ipairs(self.ChoiceCards) do
        if Card and Card.ClearChoiceData then
            Card:ClearChoiceData()
        end
    end
end

function M:HandleChoiceCardSelected(ChoiceIndex)
    if self.bChoiceSelectionSubmitted then
        return
    end

    if self.LevelUpWidgetController and ChoiceIndex and ChoiceIndex >= 0 then
        self.bChoiceSelectionSubmitted = true
        self.LevelUpWidgetController:SelectEquipmentByIndex(ChoiceIndex)
        if self.bChoiceSelectionSubmitted then
            self:ExitLevelUpState()
        end
    end
end

function M:HandleEquipmentChoicesChange()
    if self.LevelUpWidgetController:HasPendingEquipmentChoices() then
        self.bChoiceSelectionSubmitted = false
        self:EnterLevelUpState()
        self:UpdateChoiceCards()
        self:SetVisibility(UE.ESlateVisibility.Visible)
    else
        self:ExitLevelUpState()
        self:ClearChoiceCards()
        self.bChoiceSelectionSubmitted = false
        self:SetVisibility(UE.ESlateVisibility.Collapsed)
    end
end

function M:HandleEquipmentInventoryChange()
    local OwnedWeapons = UE.TArray(UE.FElysiaEquipmentDefinition)
    local OwnedPassives = UE.TArray(UE.FElysiaEquipmentDefinition)
    local OwnedEquipments = self.LevelUpWidgetController.OwnedEquipments

    for i = 1, OwnedEquipments:Length() do
        local OwnedEquipment = OwnedEquipments:Get(i).Equipment
        if OwnedEquipment.EquipmentType == UE.EElysiaEquipmentType.Weapon then
            OwnedWeapons:Add(OwnedEquipment)
        else
            OwnedPassives:Add(OwnedEquipment)
        end
    end

    local WeaponNum = OwnedWeapons:Length()
    if WeaponNum >= 1 then
        self.Image_Weapon_1:SetBrushFromTexture(OwnedWeapons:Get(1).Icon) end
    if WeaponNum >= 2 then
        self.Image_Weapon_2:SetBrushFromTexture(OwnedWeapons:Get(2).Icon) end
    if WeaponNum >= 3 then
        self.Image_Weapon_3:SetBrushFromTexture(OwnedWeapons:Get(3).Icon) end
    if WeaponNum >= 4 then
        self.Image_Weapon_4:SetBrushFromTexture(OwnedWeapons:Get(4).Icon) end

    local PassiveNum = OwnedPassives:Length()
    if PassiveNum >= 1 then
        self.Image_Passive_1:SetBrushFromTexture(OwnedPassives:Get(1).Icon) end
    if PassiveNum >= 2 then
        self.Image_Passive_2:SetBrushFromTexture(OwnedPassives:Get(2).Icon) end
    if PassiveNum >= 3 then
        self.Image_Passive_3:SetBrushFromTexture(OwnedPassives:Get(3).Icon) end
    if PassiveNum >= 4 then
        self.Image_Passive_4:SetBrushFromTexture(OwnedPassives:Get(4).Icon) end
end

return M
