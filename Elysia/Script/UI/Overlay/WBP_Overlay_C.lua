--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_Overlay_C
local M = UnLua.Class()

local function SetWidgetVisible(Widget, bVisible)
    if not Widget then
        return
    end

    Widget:SetVisibility(bVisible and UE.ESlateVisibility.Visible or UE.ESlateVisibility.Collapsed)
end

local function SetEmbeddedProgressPercent(Widget, Percent)
    if not Widget then
        return
    end

    local ClampedPercent = math.max(0, math.min(Percent or 0, 1))
    if Widget.ProgressBar then
        Widget.ProgressBar:SetPercent(ClampedPercent)
    elseif Widget.SetBarPercent then
        Widget:SetBarPercent(ClampedPercent, 1)
    end
end

function M:WidgetControllerSet()
    self.WBP_XPBar:SetWidgetController(self.WidgetController)
    self.WBP_GameStateBar:SetWidgetController(self.WidgetController)
    self.WBP_SkillGlobe:SetWidgetController(self.WidgetController)
    self:SetBossHealthBarVisible(false)
    
    local ElysiaOverWidgetController = UE.UElysiaOverlayWidgetController.Cast(self.WidgetController, UE.UElysiaOverlayWidgetController.StaticClass())
    if ElysiaOverWidgetController then
        ElysiaOverWidgetController.OnScoreTextChanged:Add(self, self.UpdateScoreText)
        ElysiaOverWidgetController.OnGameProgressPercentChanged:Add(self, self.UpdateProgressPercent)
        ElysiaOverWidgetController.OnBossHealthPercentChanged:Add(self, self.UpdateBossHealthPercent)
        ElysiaOverWidgetController.OnBossHealthVisibilityChanged:Add(self, self.SetBossHealthBarVisible)
    end
end

function M:UpdateScoreText(TotalScore)
    self.TotalScore:SetText(UE.UKismetTextLibrary.Conv_IntToText(TotalScore))
end

function M:UpdateProgressPercent(Percent)
    SetEmbeddedProgressPercent(self.WBP_GameStateBar, Percent)
end

function M:UpdateBossHealthPercent(Percent)
    SetEmbeddedProgressPercent(self.WBP_BossHealthBar, Percent)
end

function M:SetBossHealthBarVisible(bVisible)
    SetWidgetVisible(self.WBP_BossHealthBar, bVisible)
    SetWidgetVisible(self.WBP_GameStateBar, not bVisible)
end

return M
