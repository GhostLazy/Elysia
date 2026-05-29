--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_Overlay_C
local M = UnLua.Class()

function M:WidgetControllerSet()
    self.WBP_XPBar:SetWidgetController(self.WidgetController)
    self.WBP_GameStateBar:SetWidgetController(self.WidgetController)
    
    local ElysiaOverWidgetController = UE.UElysiaOverlayWidgetController.Cast(self.WidgetController, UE.UElysiaOverlayWidgetController.StaticClass())
    if ElysiaOverWidgetController then
        ElysiaOverWidgetController.OnScoreTextChanged:Add(self, self.UpdateScoreText)
        ElysiaOverWidgetController.OnGameProgressPercentChanged:Add(self, self.UpdateProgressPercent)
    end
end

function M:UpdateScoreText(TotalScore)
    self.TotalScore:SetText(UE.UKismetTextLibrary.Conv_IntToText(TotalScore))
end

function M:UpdateProgressPercent(Percent)
    self.WBP_GameStateBar.ProgressBar:SetPercent(Percent)
end

return M
