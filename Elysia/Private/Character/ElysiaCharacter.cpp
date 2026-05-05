// Copyright GhostLazy


#include "Character/ElysiaCharacter.h"
#include "Actor/ElysiaXPBall.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/ElysiaPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "Elysia/Elysia.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Player/ElysiaPlayerController.h"
#include "UI/ElysiaHUD.h"

AElysiaCharacter::AElysiaCharacter()
{
	// 设置武器附着与碰撞
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 设置弹簧臂角度
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeRotation(FRotator(-40.0f, 0.0f, 0.0f));
	// 设置弹簧臂长度，禁用碰撞、旋转、延迟
	SpringArm->TargetArmLength = 2200.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = false;
	// 设置相机视野
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(40.0f);
	
	// 角色面向移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
	// 角色运动限制在平面
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	// 保持角色朝向不受控制器朝向影响
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// 角色与敌人不发生物理碰撞
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Player);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Minion, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 启用每帧更新
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(FName("Player"));
}

void AElysiaCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitAbilityActorInfo();
}

void AElysiaCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilityActorInfo();
}

void AElysiaCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化经验球拾取定时器
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(XPBallAttractionScanTimerHandle, this, &AElysiaCharacter::ScanAndAttractNearbyXPBalls, XPBallAttractionScanInterval, true);
	}
	
	// 服务器初始化血条
	if (UElysiaAttributeSet* ElysiaAS = Cast<UElysiaAttributeSet>(AttributeSet))
	{
		OnHealthChanged.Broadcast(ElysiaAS->GetHealth());
		OnMaxHealthChanged.Broadcast(ElysiaAS->GetMaxHealth());
	}
}


void AElysiaCharacter::RotateToTarget(const AActor* TargetActor) const
{
	if (TargetActor == nullptr)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
	else if (AController* ElysiaController = GetController())
	{
		const FRotator TargetRot = (TargetActor->GetActorLocation() - GetActorLocation()).Rotation();
		ElysiaController->SetControlRotation(TargetRot);
	
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}


void AElysiaCharacter::InitAbilityActorInfo()
{
	AElysiaPlayerState* ElysiaPlayerState = Cast<AElysiaPlayerState>(GetPlayerState());
	if (ElysiaPlayerState)
	{
		ElysiaPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(ElysiaPlayerState, this);
		AbilitySystemComponent = ElysiaPlayerState->GetAbilitySystemComponent();
		AttributeSet = ElysiaPlayerState->GetAttributeSet();
	}

	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}
	
	InitDefaultAttributes();
	InitHealthBar();
	
	// 客户端初始化血条
	if (UElysiaAttributeSet* ElysiaAS = Cast<UElysiaAttributeSet>(AttributeSet))
	{
		OnHealthChanged.Broadcast(ElysiaAS->GetHealth());
		OnMaxHealthChanged.Broadcast(ElysiaAS->GetMaxHealth());
	}
	
	if (AElysiaPlayerController* ElysiaPlayerController = Cast<AElysiaPlayerController>(GetController()))
	{
		if (AElysiaHUD* ElysiaHUD = Cast<AElysiaHUD>(ElysiaPlayerController->GetHUD()))
		{
			ElysiaHUD->InitOverlay(ElysiaPlayerState, ElysiaPlayerController, AbilitySystemComponent, AttributeSet);
		}
	}
	InitCharacterAbilities();
	InitCharacterEquipments();
}

void AElysiaCharacter::InitCharacterEquipments()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AElysiaPlayerState* ElysiaPlayerState = Cast<AElysiaPlayerState>(GetPlayerState()))
	{
		UElysiaEquipmentComponent* EquipmentComponent = ElysiaPlayerState->GetEquipmentComponent();
		if (!EquipmentComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s failed to initialize startup equipments because EquipmentComponent is null."), *GetName());
			return;
		}

		EquipmentComponent->GrantStartupEquipmentsOnce(StartupEquipmentsId);
	}
}

void AElysiaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(XPBallAttractionScanTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AElysiaCharacter::ScanAndAttractNearbyXPBalls()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	// 仅扫描Projectile
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Projectile);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaXPBallAttractionScan), false);
	QueryParams.AddIgnoredActor(this);

	if (!GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(XPBallAttractionRadius),
		QueryParams))
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AElysiaXPBall* XPBall = Cast<AElysiaXPBall>(Overlap.GetActor()))
		{
			XPBall->BeginAttractionTo(this);
		}
	}
}
