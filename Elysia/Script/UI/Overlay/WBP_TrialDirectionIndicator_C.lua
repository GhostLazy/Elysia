--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_TrialDirectionIndicator_C
local M = UnLua.Class()

local function ToText(String)
    return UE.UKismetTextLibrary.Conv_StringToText(String or "")
end

local function FormatRemainingTime(Remaining)
    local TotalSeconds = math.max(0, math.ceil(Remaining or 0))
    local Minutes = math.floor(TotalSeconds / 60)
    local Seconds = TotalSeconds % 60
    return string.format("%02d:%02d", Minutes, Seconds)
end

local function GetPointerOrbit(Widget)
    if Widget.Image_pointer and Widget.Image_pointer.GetParent then
        return Widget.Image_pointer:GetParent()
    end

    return Widget.CanvasPanel_PointerOrbit
        or Widget.CanvasPanel_Orbit
        or Widget.PointerOrbit
        or Widget.CanvasPanel
end

local function SetWidgetVisible(Widget, bVisible)
    if Widget then
        Widget:SetVisibility(bVisible and UE.ESlateVisibility.SelfHitTestInvisible or UE.ESlateVisibility.Collapsed)
    end
end

function M:Construct()
    self.TrialOfferActor = nil
    self.PointerAngle = 0
    self:SetVisibility(UE.ESlateVisibility.Collapsed)
end

function M:WidgetControllerSet()
	self.TrialOfferActor = UE.AElysiaTrialInteractableActor.Cast(
		self.WidgetController,
		UE.AElysiaTrialInteractableActor.StaticClass()
	)

    self:RefreshDisplay()
end

function M:SetPointerAngle(Angle)
    self.PointerAngle = Angle or 0

    local PointerOrbit = GetPointerOrbit(self)
    if PointerOrbit then
        PointerOrbit:SetRenderTransformAngle(self.PointerAngle)
    end
end

function M:RefreshDisplay()
	if not self.TrialOfferActor then
		self:SetVisibility(UE.ESlateVisibility.Collapsed)
		return
	end

	local Remaining = self.TrialOfferActor:GetRemainingOfferTime()
	self:SetVisibility(UE.ESlateVisibility.SelfHitTestInvisible)

	if self.TextBlock_Time then
		self.TextBlock_Time:SetText(ToText(FormatRemainingTime(Remaining)))
	end
end

function M:SetIndicatorActive(TrialOfferActor, PointerAngle)
    if TrialOfferActor and self.WidgetController ~= TrialOfferActor then
		if self.SetWidgetController then
			self:SetWidgetController(TrialOfferActor)
		else
			self.WidgetController = TrialOfferActor
			self:WidgetControllerSet()
		end
	else
		self.TrialOfferActor = TrialOfferActor
	end

    self:SetPointerAngle(PointerAngle)
    self:RefreshDisplay()
end

function M:SetIndicatorInactive()
    self.TrialOfferActor = nil
    SetWidgetVisible(self, false)
end

function M:Tick(MyGeometry, InDeltaTime)
    self:RefreshDisplay()
end

return M
