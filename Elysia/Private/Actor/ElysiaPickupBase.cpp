// Copyright GhostLazy


#include "Actor/ElysiaPickupBase.h"

#include "Character/ElysiaCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Elysia/Elysia.h"

AElysiaPickupBase::AElysiaPickupBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
}

void AElysiaPickupBase::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaPickupBase::OnSphereOverlap);
}

void AElysiaPickupBase::HandlePickedBy(AElysiaCharacter* Character)
{
}

bool AElysiaPickupBase::CanBePickedBy(const AElysiaCharacter* Character, const UPrimitiveComponent* OtherComp) const
{
	return IsValid(Character) && OtherComp == Character->GetCapsuleComponent();
}

void AElysiaPickupBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(OtherActor))
	{
		if (!CanBePickedBy(ElysiaCharacter, OtherComp))
		{
			return;
		}

		HandlePickedBy(ElysiaCharacter);
		Destroy();
	}
}
