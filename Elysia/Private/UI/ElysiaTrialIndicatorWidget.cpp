// Copyright GhostLazy


#include "UI/ElysiaTrialIndicatorWidget.h"

#include "Actor/ElysiaTrialEventBase.h"
#include "Actor/ElysiaTrialInteractableActor.h"

void UElysiaTrialIndicatorWidget::SetTrialOfferActor(AElysiaTrialInteractableActor* InTrialOfferActor)
{
	TrialOfferActor = InTrialOfferActor;
	TrialOfferActorSet();
}

AElysiaTrialEventBase* UElysiaTrialIndicatorWidget::GetTrialEvent() const
{
	return TrialOfferActor ? TrialOfferActor->GetTrialEvent() : nullptr;
}

float UElysiaTrialIndicatorWidget::GetRemainingOfferTime() const
{
	return TrialOfferActor ? TrialOfferActor->GetRemainingOfferTime() : 0.f;
}

float UElysiaTrialIndicatorWidget::GetTotalOfferTime() const
{
	return TrialOfferActor ? TrialOfferActor->GetTotalOfferTime() : 0.f;
}

FText UElysiaTrialIndicatorWidget::GetRemainingOfferTimeText() const
{
	const int32 RemainingSeconds = FMath::CeilToInt(FMath::Max(0.f, GetRemainingOfferTime()));
	const int32 Minutes = RemainingSeconds / 60;
	const int32 Seconds = RemainingSeconds % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}

FVector UElysiaTrialIndicatorWidget::GetTargetWorldLocation() const
{
	return TrialOfferActor ? TrialOfferActor->GetIndicatorTargetLocation() : FVector::ZeroVector;
}

void UElysiaTrialIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TrialOfferActor)
	{
		TrialIndicatorDataUpdated(GetRemainingOfferTime(), GetTotalOfferTime());
	}
}
