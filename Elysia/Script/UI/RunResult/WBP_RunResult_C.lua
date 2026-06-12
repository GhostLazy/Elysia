---@type WBP_RunResult_C
local M = UnLua.Class()

local function ToText(Value)
    return UE.UKismetTextLibrary.Conv_StringToText(tostring(Value or ""))
end

function M:Construct()
    if self.Button_Confirm then
        self.Button_Confirm.OnClicked:Add(self, self.HandleConfirmClicked)
    end

    local PlayerController = self:GetOwningPlayer()
    if PlayerController then
        PlayerController:SetIgnoreMoveInput(true)
        PlayerController:SetIgnoreLookInput(true)
        PlayerController.bShowMouseCursor = true
        UE.UWidgetBlueprintLibrary.SetInputMode_UIOnlyEx(
            PlayerController,
            self,
            UE.EMouseLockMode.DoNotLock,
            true)
    end

    self:RefreshResult()
end

function M:WidgetControllerSet()
    self.GameState = UE.AElysiaGameState.Cast(
        self.WidgetController,
        UE.AElysiaGameState.StaticClass())
    self:RefreshResult()
end

function M:RefreshResult()
    if not self.GameState or not self.TextBlock_FinalScore then
        return
    end

    self.TextBlock_FinalScore:SetText(ToText(
        string.format("最终分数：%d", self.GameState:GetTotalScore())))
end

function M:HandleConfirmClicked()
    local PlayerController = self:GetOwningPlayer()
    if PlayerController then
        UE.UKismetSystemLibrary.QuitGame(
            self,
            PlayerController,
            UE.EQuitPreference.Quit,
            false)
    end
end

return M
