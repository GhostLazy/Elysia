--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_Overlay_C
local M = UnLua.Class()

--工具函数Start
local function SetWidgetVisible(Widget, bVisible)
    if not Widget then
        return
    end

    Widget:SetVisibility(bVisible and UE.ESlateVisibility.Visible or UE.ESlateVisibility.Collapsed)
end

local function ToText(String)
    return UE.UKismetTextLibrary.Conv_StringToText(String or "")
end

local function FormatRemainingTime(Remaining)
    local TotalSeconds = math.max(0, math.ceil(Remaining or 0))
    local Minutes = math.floor(TotalSeconds / 60)
    local Seconds = TotalSeconds % 60
    return string.format("%02d:%02d", Minutes, Seconds)
end

local function GetElysiaPlayerController(Widget)
    local PlayerController = Widget and Widget:GetOwningPlayer() or nil
    return UE.AElysiaPlayerController.Cast(
        PlayerController,
        UE.AElysiaPlayerController.StaticClass()
    )
end

local function GetTrialInteractionComponent(Widget)
    local PlayerController = GetElysiaPlayerController(Widget)
    return PlayerController and PlayerController:GetTrialInteractionComponent() or nil
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

local function Atan2(Y, X)
    if math.atan2 then
        return math.atan2(Y, X)
    end

	return math.atan(Y, X)
end

local function SubtractVector(A, B)
	return {
		X = A.X - B.X,
		Y = A.Y - B.Y,
		Z = A.Z - B.Z,
	}
end

local function GetOwningPawn(Widget)
    if Widget and Widget.GetOwningPlayerPawn then
        return Widget:GetOwningPlayerPawn()
    end

    return UE.UGameplayStatics.GetPlayerPawn(Widget, 0)
end
--工具函数End

function M:WidgetControllerSet()
    self.WBP_XPBar:SetWidgetController(self.WidgetController)
    self.WBP_GameStateBar:SetWidgetController(self.WidgetController)
    self.WBP_SkillGlobe:SetWidgetController(self.WidgetController)
    self:SetBossHealthBarVisible(false)
    self:SetTrialDirectionIndicatorsVisible(false)
    self:SetInteractPromptVisible(false)
    self:SetTrialStatusVisible(false)
    
    local ElysiaOverWidgetController = UE.UElysiaOverlayWidgetController.Cast(self.WidgetController, UE.UElysiaOverlayWidgetController.StaticClass())
    if ElysiaOverWidgetController then
        ElysiaOverWidgetController.OnScoreTextChanged:Add(self, self.UpdateScoreText)
        ElysiaOverWidgetController.OnGameProgressPercentChanged:Add(self, self.UpdateProgressPercent)
        ElysiaOverWidgetController.OnBossHealthPercentChanged:Add(self, self.UpdateBossHealthPercent)
        ElysiaOverWidgetController.OnBossHealthVisibilityChanged:Add(self, self.SetBossHealthBarVisible)
        -- 所有 UI 委托完成绑定后，再请求当前客户端快照，避免错过先到达的 OnRep。
        ElysiaOverWidgetController:BroadcastInitialValues()
    end
end

function M:Construct()
    self:SetTrialDirectionIndicatorsVisible(false)
    self:SetInteractPromptVisible(false)
    self:SetTrialStatusVisible(false)
end

function M:Tick(MyGeometry, InDeltaTime)
    self:UpdateTrialDirectionIndicator()
    self:UpdateInteractPrompt()
    self:UpdateActiveTrialStatus()
end

--更新基本UI
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

--试炼位置指示器
function M:SetTrialDirectionIndicatorsVisible(bVisible)
    SetWidgetVisible(self.WBP_TrialDirectionIndicator_Left, bVisible)
    SetWidgetVisible(self.WBP_TrialDirectionIndicator_Right, bVisible)
end

function M:GetCurrentTrialOfferActor()
    local TrialInteractionComponent = GetTrialInteractionComponent(self)
    return TrialInteractionComponent and TrialInteractionComponent:GetCurrentTrialOfferActor() or nil
end

function M:GetCurrentInteractableTrialOfferActor()
    local TrialInteractionComponent = GetTrialInteractionComponent(self)
    return TrialInteractionComponent and TrialInteractionComponent:GetCurrentInteractableTrialOfferActor() or nil
end

function M:GetCurrentTrialEvent()
    local TrialInteractionComponent = GetTrialInteractionComponent(self)
    return TrialInteractionComponent and TrialInteractionComponent:GetCurrentTrialEvent() or nil
end

function M:SetTrialStatusVisible(bVisible)
    SetWidgetVisible(self.TextBlock_TrialStatus, bVisible)
end

function M:SetTrialStatusText(StatusText)
    local TextBlock = self.TextBlock_TrialStatus
    if TextBlock then
        TextBlock:SetText(ToText(StatusText))
    end
end

function M:UpdateActiveTrialStatus()
    local TrialEvent = self:GetCurrentTrialEvent()
    if not TrialEvent then
        self:SetTrialStatusVisible(false)
        return
    end

    if TrialEvent:HasExpired() then
        self:SetTrialStatusText("试炼失败")
        self:SetTrialStatusVisible(true)
        return
    end

    if TrialEvent:HasBeenTriggered() then
        self:SetTrialStatusText(FormatRemainingTime(TrialEvent:GetRemainingTrialTime()))
        self:SetTrialStatusVisible(true)
        return
    end

    self:SetTrialStatusVisible(false)
end

function M:UpdateTrialDirectionIndicator()
    local TrialInteractionComponent = GetTrialInteractionComponent(self)
    local IndicatorTarget = nil
    local bShouldShowIndicator = false
    if TrialInteractionComponent and TrialInteractionComponent:ShouldShowTrialDestinationIndicator() then
        IndicatorTarget = self:GetCurrentTrialEvent()
        bShouldShowIndicator = true
    elseif TrialInteractionComponent and TrialInteractionComponent:ShouldShowTrialDirectionIndicator() then
        IndicatorTarget = self:GetCurrentTrialOfferActor()
        bShouldShowIndicator = true
    end
    local Pawn = GetOwningPawn(self)

	if not bShouldShowIndicator then
		if self.WBP_TrialDirectionIndicator_Left and self.WBP_TrialDirectionIndicator_Left.SetIndicatorInactive then
			self.WBP_TrialDirectionIndicator_Left:SetIndicatorInactive()
		end
        if self.WBP_TrialDirectionIndicator_Right and self.WBP_TrialDirectionIndicator_Right.SetIndicatorInactive then
            self.WBP_TrialDirectionIndicator_Right:SetIndicatorInactive()
        end
        return
	end

	if not IndicatorTarget or not Pawn then
		if self.WBP_TrialDirectionIndicator_Left and self.WBP_TrialDirectionIndicator_Left.SetIndicatorInactive then
			self.WBP_TrialDirectionIndicator_Left:SetIndicatorInactive()
		end
        if self.WBP_TrialDirectionIndicator_Right and self.WBP_TrialDirectionIndicator_Right.SetIndicatorInactive then
            self.WBP_TrialDirectionIndicator_Right:SetIndicatorInactive()
        end
        return
	end

	local TargetLocation = IndicatorTarget:GetIndicatorTargetLocation()
	local PawnLocation = Pawn:K2_GetActorLocation()
	local DirectionToTarget = SubtractVector(TargetLocation, PawnLocation)
	DirectionToTarget.Z = 0

    local PointerAngle = math.deg(Atan2(-DirectionToTarget.X, DirectionToTarget.Y))
    local bTargetOnRight = DirectionToTarget.Y >= 0

    local ActiveIndicator = bTargetOnRight and self.WBP_TrialDirectionIndicator_Right or self.WBP_TrialDirectionIndicator_Left
    local InactiveIndicator = bTargetOnRight and self.WBP_TrialDirectionIndicator_Left or self.WBP_TrialDirectionIndicator_Right

	if ActiveIndicator then
		SetWidgetVisible(ActiveIndicator, true)
		if ActiveIndicator.SetIndicatorActive then
			ActiveIndicator:SetIndicatorActive(IndicatorTarget, PointerAngle)
		end
	end

	if InactiveIndicator then
		if InactiveIndicator.SetIndicatorInactive then
			InactiveIndicator:SetIndicatorInactive()
		else
			SetWidgetVisible(InactiveIndicator, false)
		end
	end
end

--试炼交互键
function M:SetInteractPromptVisible(bVisible)
    SetWidgetVisible(self.Image_Interact, bVisible)
end

function M:UpdateInteractPrompt()
    local TrialInteractionComponent = GetTrialInteractionComponent(self)
    self:SetInteractPromptVisible(TrialInteractionComponent and TrialInteractionComponent:ShouldShowTrialInteractPrompt())
end


return M
