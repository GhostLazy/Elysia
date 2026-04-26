--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_LevelUp_C
local M = UnLua.Class()

function M:ShouldPauseForLevelUp()
    local World = self:GetWorld()
    return World and World:GetNetMode() == UE.ENetMode.NM_Standalone
end

function M:WidgetControllerSet()
    self.LevelUpWidgetController = UE.UElysiaLevelUpWidgetController.Cast(self.WidgetController, UE.UElysiaLevelUpWidgetController.StaticClass())
    self.LevelUpWidgetController.OnEquipmentChoicesChanged:Add(self, self.HandleEquipmentChoicesChange)
    self.LevelUpWidgetController.OnEquipmentInventoryChanged:Add(self, self.HandleEquipmentInventoryChange)

    self.Button_0.OnClicked:Add(self, self.OnButtonClicked_0)
    self.Button_1.OnClicked:Add(self, self.OnButtonClicked_1)
    self.Button_2.OnClicked:Add(self, self.OnButtonClicked_2)

    --显示初始装备
    self:HandleEquipmentChoicesChange()
end

function M:EnterLevelUpState()
    if self.bLevelUpStateActive then
        return
    end

    local PlayerController = self:GetOwningPlayer()
    if PlayerController then
        PlayerController:SetIgnoreMoveInput(true)
        UE.UWidgetBlueprintLibrary.SetInputMode_UIOnlyEx(PlayerController, self, UE.EMouseLockMode.DoNotLock, true)
    end

    self.bPausedForLevelUp = self:ShouldPauseForLevelUp()
    if self.bPausedForLevelUp then
        UE.UGameplayStatics.SetGamePaused(self, true)
    end
    self.bLevelUpStateActive = true
end

function M:ExitLevelUpState()
    if not self.bLevelUpStateActive then
        return
    end

    local PlayerController = self:GetOwningPlayer()
    if self.bPausedForLevelUp then
        UE.UGameplayStatics.SetGamePaused(self, false)
    end

    if PlayerController then
        PlayerController:ResetIgnoreMoveInput()
        UE.UWidgetBlueprintLibrary.SetInputMode_GameAndUIEx(PlayerController, self, UE.EMouseLockMode.DoNotLock, false, true)
    end

    self.bPausedForLevelUp = false
    self.bLevelUpStateActive = false
end

function M:UpdateChoiceText(TextBlock, ChoiceIndex)
    if not TextBlock then
        return
    end

    local ChoiceCount = self.LevelUpWidgetController.CurrentEquipmentChoices:Length()
    if ChoiceIndex > ChoiceCount then
        TextBlock:SetText("")
        return
    end

    local Choice = self.LevelUpWidgetController.CurrentEquipmentChoices:Get(ChoiceIndex)
    if Choice and Choice.Equipment then
        TextBlock:SetText(Choice.Equipment.DisplayName)
    else
        TextBlock:SetText("")
    end
end

function M:OnButtonClicked_0()
    self.LevelUpWidgetController:SelectEquipmentByIndex(0)
end

function M:OnButtonClicked_1()
    self.LevelUpWidgetController:SelectEquipmentByIndex(1)
end

function M:OnButtonClicked_2()
    self.LevelUpWidgetController:SelectEquipmentByIndex(2)
end

function M:HandleEquipmentChoicesChange()
    if self.LevelUpWidgetController:HasPendingEquipmentChoices() then
        self:EnterLevelUpState()
        self:UpdateChoiceText(self.TextBlock_0, 1)
        self:UpdateChoiceText(self.TextBlock_1, 2)
        self:UpdateChoiceText(self.TextBlock_2, 3)
        self:SetVisibility(UE.ESlateVisibility.Visible)
    else
        self:ExitLevelUpState()
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
