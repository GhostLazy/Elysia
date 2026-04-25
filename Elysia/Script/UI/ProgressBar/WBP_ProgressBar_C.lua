--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_ProgressBar_C
local M = UnLua.Class()

function M:WidgetControllerSet()
    local Character = UE.AElysiaCharacterBase.Cast(self.WidgetController, UE.AElysiaCharacterBase.StaticClass())
    self.Health = 0
    self.MaxHealth = 0

    Character.OnHealthChanged:Add(self, self.UpdateHealthBarByHealth)
    Character.OnMaxHealthChanged:Add(self, self.UpdateHealthBarByMaxHealth)
end

function M:UpdateHealthBarByHealth(NewHealth)
    self.Health = NewHealth
    self:SetBarPercent(self.Health, self.MaxHealth)
end

function M:UpdateHealthBarByMaxHealth(NewMaxHealth)
    self.MaxHealth = NewMaxHealth
    self:SetBarPercent(self.Health, self.MaxHealth)
end

function M:SetBarPercent(Health, MaxHealth)
    if MaxHealth > 0 then
        self.ProgressBar:SetPercent(Health / MaxHealth)
    end
end

return M
