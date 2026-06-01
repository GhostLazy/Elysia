--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_SkillGlobe_C
local M = UnLua.Class()

local CooldownRefreshInterval = 0.05

local function IsCooldownActive(CooldownInfo)
    if not CooldownInfo then
        return false
    end

    return CooldownInfo.bIsActive == true or CooldownInfo.IsActive == true
end

local function GetCooldownValue(CooldownInfo, PropertyName)
    if not CooldownInfo then
        return 0
    end

    return CooldownInfo[PropertyName] or 0
end

function M:Construct()
    self.CooldownRefreshAcc = 0
    self:HideCooldown()
end

function M:WidgetControllerSet()
    self.OverlayWidgetController = UE.UElysiaOverlayWidgetController.Cast(
        self.WidgetController,
        UE.UElysiaOverlayWidgetController.StaticClass()
    )

    self:UpdateSkillCooldown()
end

function M:Tick(MyGeometry, InDeltaTime)
    self.CooldownRefreshAcc = (self.CooldownRefreshAcc or 0) + InDeltaTime
    if self.CooldownRefreshAcc < CooldownRefreshInterval then
        return
    end

    self.CooldownRefreshAcc = 0
    self:UpdateSkillCooldown()
end

function M:UpdateSkillCooldown()
    if not self.OverlayWidgetController then
        self:HideCooldown()
        return
    end

    local CooldownInfo = self.OverlayWidgetController:GetSkillCooldownInfo()
    local Remaining = GetCooldownValue(CooldownInfo, "Remaining")

    if IsCooldownActive(CooldownInfo) and Remaining > 0 then
        self:ShowCooldown(Remaining)
    else
        self:HideCooldown()
    end
end

function M:ShowCooldown(Remaining)
    self.SkillMask:SetVisibility(UE.ESlateVisibility.Visible)
    self.Cooldown:SetVisibility(UE.ESlateVisibility.Visible)

    local SecondsText = string.format("%.1f", math.max(0, Remaining))
    self.Cooldown:SetText(UE.UKismetTextLibrary.Conv_StringToText(SecondsText))
end

function M:HideCooldown()
    self.SkillMask:SetVisibility(UE.ESlateVisibility.Hidden)
    self.Cooldown:SetVisibility(UE.ESlateVisibility.Hidden)
    self.Cooldown:SetText(UE.UKismetTextLibrary.Conv_IntToText(0))
end

return M
