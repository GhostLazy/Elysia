// Copyright GhostLazy


#include "Equipment/ElysiaEquipmentComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Algo/RandomShuffle.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

UElysiaEquipmentComponent::UElysiaEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UElysiaEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UElysiaEquipmentComponent, OwnedEquipments);
	DOREPLIFETIME(UElysiaEquipmentComponent, PendingChoices);
	DOREPLIFETIME(UElysiaEquipmentComponent, PendingSelectionCount);
}

int32 UElysiaEquipmentComponent::GetEquipmentLevelById(FName EquipmentId) const
{
	if (const FElysiaEquipmentEntry* EquipmentEntry = FindOwnedEquipmentById(EquipmentId))
	{
		return EquipmentEntry->Level;
	}

	return 0;
}

bool UElysiaEquipmentComponent::IsEquipmentEvolvedById(FName EquipmentId) const
{
	if (const FElysiaEquipmentEntry* EquipmentEntry = FindOwnedEquipmentById(EquipmentId))
	{
		return EquipmentEntry->bEvolved;
	}

	return false;
}

int32 UElysiaEquipmentComponent::GetEquipmentLevelByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (const FElysiaEquipmentEntry* EquipmentEntry = FindOwnedEquipmentByAbilityClass(AbilityClass))
	{
		return EquipmentEntry->Level;
	}

	return 0;
}

bool UElysiaEquipmentComponent::IsEquipmentEvolvedByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (const FElysiaEquipmentEntry* EquipmentEntry = FindOwnedEquipmentByAbilityClass(AbilityClass))
	{
		return EquipmentEntry->bEvolved;
	}

	return false;
}

void UElysiaEquipmentComponent::QueueLevelUpSelections(int32 NumSelections)
{
	if (NumSelections <= 0 || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	PendingSelectionCount += NumSelections;
	if (!HasPendingChoices())
	{
		RollNextChoices();
	}
}

void UElysiaEquipmentComponent::GrantStartupEquipmentsOnce(const TArray<FName>& StartupEquipmentIds)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bStartupEquipmentsGranted)
	{
		return;
	}

	// 开局装备属于权威状态，只允许服务端成功初始化一次
	bStartupEquipmentsGranted = true;

	if (!EquipmentPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to grant startup equipments because EquipmentPool is null."), *GetName());
		return;
	}

	for (const FName EquipmentId : StartupEquipmentIds)
	{
		if (EquipmentId.IsNone())
		{
			continue;
		}

		// 所有开局装备都必须来自统一的装备池，避免定义分叉
		const FElysiaEquipmentDefinition* EquipmentDefinition = EquipmentPool->FindEquipmentById(EquipmentId);
		if (!EquipmentDefinition)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s failed to find startup equipment definition for '%s'."), *GetName(), *EquipmentId.ToString());
			continue;
		}

		GrantEquipment(*EquipmentDefinition);
	}
}

void UElysiaEquipmentComponent::SelectChoiceByIndex(int32 ChoiceIndex)
{
	if (!HasPendingChoices())
	{
		return;
	}

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		ServerSelectChoiceByIndex(ChoiceIndex);
		return;
	}

	if (!PendingChoices.IsValidIndex(ChoiceIndex))
	{
		return;
	}

	GrantEquipment(PendingChoices[ChoiceIndex].Equipment);

	PendingChoices.Empty();
	OnRep_PendingChoices();

	PendingSelectionCount = FMath::Max(0, PendingSelectionCount - 1);
	RollNextChoices();
}

void UElysiaEquipmentComponent::OnRep_OwnedEquipments()
{
	OnOwnedEquipmentsChanged.Broadcast();
}

void UElysiaEquipmentComponent::OnRep_PendingChoices()
{
	OnPendingChoicesChanged.Broadcast();
}

void UElysiaEquipmentComponent::ServerSelectChoiceByIndex_Implementation(int32 ChoiceIndex)
{
	SelectChoiceByIndex(ChoiceIndex);
}

void UElysiaEquipmentComponent::GrantEquipment(const FElysiaEquipmentDefinition& EquipmentDefinition)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || EquipmentDefinition.EquipmentId.IsNone())
	{
		return;
	}

	FElysiaEquipmentEntry* OwnedEntry = FindOwnedEquipment(EquipmentDefinition.EquipmentId);
	if (OwnedEntry == nullptr)
	{
		// 首次获得装备时，新建条目并从1级开始
		FElysiaEquipmentEntry NewEntry;
		NewEntry.Equipment = EquipmentDefinition;
		NewEntry.Level = 1;
		OwnedEquipments.Add(NewEntry);
		OwnedEntry = &OwnedEquipments.Last();
	}
	else
	{
		// 再次获得同装备时，按定义中的 MaxLevel 做升级
		OwnedEntry->Level = FMath::Clamp(OwnedEntry->Level + 1, 1, FMath::Max(1, EquipmentDefinition.MaxLevel));
	}

	// 装备获得后统一走效果、能力、进化状态刷新
	ApplyEquipmentEffects(*OwnedEntry);
	EnsureWeaponAbilityGranted(EquipmentDefinition);
	UpdateWeaponEvolutionStates();
	OnRep_OwnedEquipments();
}

void UElysiaEquipmentComponent::RollNextChoices()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (PendingSelectionCount <= 0)
	{
		PendingChoices.Empty();
		OnRep_PendingChoices();
		return;
	}

	TArray<FElysiaEquipmentDefinition> Candidates;
	TSet<FName> UniqueDefinitions;
	const TArray<FElysiaEquipmentDefinition>& EquipmentDefinitions = EquipmentPool ? EquipmentPool->Equipments : TArray<FElysiaEquipmentDefinition>();
	for (const FElysiaEquipmentDefinition& EquipmentDefinition : EquipmentDefinitions)
	{
		if (!EquipmentDefinition.EquipmentId.IsNone() && !UniqueDefinitions.Contains(EquipmentDefinition.EquipmentId) && CanOfferEquipment(EquipmentDefinition))
		{
			UniqueDefinitions.Add(EquipmentDefinition.EquipmentId);
			Candidates.Add(EquipmentDefinition);
		}
	}

	if (Candidates.IsEmpty())
	{
		PendingSelectionCount = 0;
		PendingChoices.Empty();
		OnRep_PendingChoices();
		return;
	}

	Algo::RandomShuffle(Candidates);

	const int32 NumChoices = FMath::Min(ChoiceCountPerLevel, Candidates.Num());
	PendingChoices.Reset(NumChoices);
	for (int32 Index = 0; Index < NumChoices; ++Index)
	{
		const FElysiaEquipmentDefinition& EquipmentDefinition = Candidates[Index];
		const int32 OwnedIndex = FindOwnedEquipmentIndex(EquipmentDefinition.EquipmentId);
		const int32 CurrentLevel = OwnedIndex == INDEX_NONE ? 0 : OwnedEquipments[OwnedIndex].Level;
		const int32 MaxLevel = FMath::Max(1, EquipmentDefinition.MaxLevel);

		FElysiaEquipmentChoice Choice;
		Choice.Equipment = EquipmentDefinition;
		Choice.EquipmentType = EquipmentDefinition.EquipmentType;
		Choice.CurrentLevel = CurrentLevel;
		Choice.NextLevel = FMath::Min(CurrentLevel + 1, MaxLevel);
		Choice.MaxLevel = MaxLevel;
		Choice.bAlreadyOwned = OwnedIndex != INDEX_NONE;
		Choice.bWillEvolve = EquipmentDefinition.EquipmentType == EElysiaEquipmentType::Weapon
			&& Choice.NextLevel >= MaxLevel
			&& !EquipmentDefinition.RequiredPassiveEquipmentId.IsNone()
			&& GetEquipmentLevelById(EquipmentDefinition.RequiredPassiveEquipmentId) > 0;

		PendingChoices.Add(Choice);
	}

	OnRep_PendingChoices();
}

void UElysiaEquipmentComponent::ApplyEquipmentEffects(const FElysiaEquipmentEntry& EquipmentEntry)
{
	// 当且仅当该装备为属性加成（被动）类时，该函数执行
	if (EquipmentEntry.Equipment.EquipmentId.IsNone() || EquipmentEntry.Equipment.EquipmentType != EElysiaEquipmentType::Passive || !EquipmentEntry.Equipment.GrantedEffectClass)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		// 删除当前被动原有GE，以待升级
		if (FActiveGameplayEffectHandle* ExistingEffectHandle = ActiveEffectHandles.Find(EquipmentEntry.Equipment.EquipmentId))
		{
			if (ExistingEffectHandle->IsValid())
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(*ExistingEffectHandle);
			}
		}

		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			EquipmentEntry.Equipment.GrantedEffectClass,
			static_cast<float>(EquipmentEntry.Level),
			EffectContext);
		// 新增当前被动升级后的GE
		if (EffectSpecHandle.IsValid())
		{
			ActiveEffectHandles.FindOrAdd(EquipmentEntry.Equipment.EquipmentId) = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

void UElysiaEquipmentComponent::EnsureWeaponAbilityGranted(const FElysiaEquipmentDefinition& EquipmentDefinition)
{
	// 当且仅当该装备为武器时，该函数执行
	if (EquipmentDefinition.EquipmentId.IsNone() || EquipmentDefinition.EquipmentType != EElysiaEquipmentType::Weapon || !EquipmentDefinition.GrantedAbilityClass)
	{
		return;
	}

	const FElysiaEquipmentEntry* OwnedEntry = FindOwnedEquipmentById(EquipmentDefinition.EquipmentId);
	const int32 AbilityLevel = OwnedEntry ? FMath::Max(1, OwnedEntry->Level) : 1;

	if (UElysiaAbilitySystemComponent* ElysiaASC = Cast<UElysiaAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ElysiaASC->GrantOrUpdateAbilityLevel(EquipmentDefinition.GrantedAbilityClass, AbilityLevel);
	}
}

void UElysiaEquipmentComponent::UpdateWeaponEvolutionStates()
{
	// 该变量用于判断是否触发OwnedEquipments变化广播
	bool bAnyEvolutionChanged = false;
	for (FElysiaEquipmentEntry& EquipmentEntry : OwnedEquipments)
	{
		if (!EquipmentEntry.bEvolved && CanEvolve(EquipmentEntry))
		{
			EquipmentEntry.bEvolved = true;
			bAnyEvolutionChanged = true;
		}
	}

	if (bAnyEvolutionChanged)
	{
		OnRep_OwnedEquipments();
	}
}

bool UElysiaEquipmentComponent::CanEvolve(const FElysiaEquipmentEntry& EquipmentEntry) const
{
	if (EquipmentEntry.Equipment.EquipmentId.IsNone() || EquipmentEntry.Equipment.EquipmentType != EElysiaEquipmentType::Weapon)
	{
		return false;
	}

	if (EquipmentEntry.Level < FMath::Max(1, EquipmentEntry.Equipment.MaxLevel))
	{
		return false;
	}

	if (EquipmentEntry.Equipment.RequiredPassiveEquipmentId.IsNone())
	{
		return false;
	}

	return GetEquipmentLevelById(EquipmentEntry.Equipment.RequiredPassiveEquipmentId) > 0;
}

FElysiaEquipmentEntry* UElysiaEquipmentComponent::FindOwnedEquipment(FName EquipmentId)
{
	const int32 OwnedIndex = FindOwnedEquipmentIndex(EquipmentId);
	return OwnedEquipments.IsValidIndex(OwnedIndex) ? &OwnedEquipments[OwnedIndex] : nullptr;
}

const FElysiaEquipmentEntry* UElysiaEquipmentComponent::FindOwnedEquipmentById(FName EquipmentId) const
{
	return OwnedEquipments.FindByPredicate([EquipmentId](const FElysiaEquipmentEntry& Entry)
	{
		return Entry.Equipment.EquipmentId == EquipmentId;
	});
}

const FElysiaEquipmentEntry* UElysiaEquipmentComponent::FindOwnedEquipmentByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!AbilityClass)
	{
		return nullptr;
	}

	return OwnedEquipments.FindByPredicate([&AbilityClass](const FElysiaEquipmentEntry& Entry)
	{
		return Entry.Equipment.GrantedAbilityClass == AbilityClass;
	});
}

int32 UElysiaEquipmentComponent::FindOwnedEquipmentIndex(FName EquipmentId) const
{
	return OwnedEquipments.IndexOfByPredicate([EquipmentId](const FElysiaEquipmentEntry& Entry)
	{
		return Entry.Equipment.EquipmentId == EquipmentId;
	});
}

bool UElysiaEquipmentComponent::CanOfferEquipment(const FElysiaEquipmentDefinition& EquipmentDefinition) const
{
	if (EquipmentDefinition.EquipmentId.IsNone())
	{
		return false;
	}

	const int32 OwnedIndex = FindOwnedEquipmentIndex(EquipmentDefinition.EquipmentId);
	const int32 CurrentLevel = OwnedIndex == INDEX_NONE ? 0 : OwnedEquipments[OwnedIndex].Level;
	return CurrentLevel < FMath::Max(1, EquipmentDefinition.MaxLevel);
}

UAbilitySystemComponent* UElysiaEquipmentComponent::GetAbilitySystemComponent() const
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	return nullptr;
}
