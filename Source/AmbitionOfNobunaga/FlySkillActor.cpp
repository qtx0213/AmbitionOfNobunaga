// Fill out your copyright notice in the Description page of Project Settings.

#include "AmbitionOfNobunaga.h"
#include "FlySkillActor.h"
#include "UnrealNetwork.h"
#include "AONGameState.h"
#include "HeroCharacter.h"
// for GEngine
#include "Engine.h"

AFlySkillActor::AFlySkillActor(const FObjectInitializer& ObjectInitializer)
	: Super(FObjectInitializer::Get())
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("root0"));
	BulletParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("BulletParticle0"));
	RootComponent = Scene;
	BulletParticle->AttachParent = RootComponent;
	CapsuleComponent = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule0"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->AttachParent = RootComponent;
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->bShouldUpdatePhysicsVolume = true;
	CapsuleComponent->bCheckAsyncSceneOnMove = false;
	CapsuleComponent->bCanEverAffectNavigation = false;
	CapsuleComponent->bDynamicObstacle = true;
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
	DestroyDelay = 2;
	UseTargetLocation = true;
	IsReadyToStart = false;
}

#if WITH_EDITOR
void AFlySkillActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AFlySkillActor, UseTargetLocation)))
	{
		if (UseTargetLocation)
		{
			UseTargetActor = false;
		}
		else
		{
			UseTargetActor = true;
		}
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AFlySkillActor, UseTargetActor)))
	{
		if (UseTargetActor)
		{
			UseTargetLocation = false;
		}
		else
		{
			UseTargetLocation = true;
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AFlySkillActor::OnBeginAttackOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
	AHeroCharacter* hero = Cast<AHeroCharacter>(OtherActor);
	if (hero)
	{
		AttackCollision.Add(hero);
	}
}

bool AFlySkillActor::Injury_Validate()
{
	return true;
}

void AFlySkillActor::Injury_Implementation()
{
	AAONGameState* ags = Cast<AAONGameState>(UGameplayStatics::GetGameState(GetWorld()));
	for (AHeroCharacter* hero : AttackCollision)
	{
		// 如果不同隊才造成傷害
		if (hero && hero->TeamId != this->TeamId)
		{
			// 物傷
			if (PhysicalDamage > 0)
			{
				float Injury = ags->ArmorConvertToInjuryPersent(hero->CurrentArmor);
				float Damage = PhysicalDamage * Injury;
				hero->CurrentHP -= Damage;

				// 顯示傷害文字
				ADamageEffect* TempDamageText = GetWorld()->SpawnActor<ADamageEffect>(AHeroCharacter::ShowDamageEffect);
				if (TempDamageText)
				{
					FVector pos = hero->GetActorLocation();
					pos.X += 10;
					TempDamageText->OriginPosition = pos;
					TempDamageText->SetString(FString::FromInt((int32)Damage));
					FVector scaleSize(TempDamageText->ScaleSize, TempDamageText->ScaleSize, TempDamageText->ScaleSize);
					TempDamageText->SetActorScale3D(scaleSize);
					FVector dir = hero->GetActorLocation() - GetActorLocation();
					dir.Normalize();
					TempDamageText->FlyDirection = dir;
				}
			}
			// 法傷
			if (MagicDamage > 0)
			{
				float Damage = MagicDamage * (1 - hero->CurrentMagicInjured);
				hero->CurrentHP -= Damage;

				// 顯示傷害文字
				ADamageEffect* TempDamageText = GetWorld()->SpawnActor<ADamageEffect>(AHeroCharacter::ShowDamageEffect);
				if (TempDamageText)
				{
					FVector pos = hero->GetActorLocation();
					pos.X += 10;
					TempDamageText->OriginPosition = pos;
					TempDamageText->SetString(FString::FromInt((int32)Damage));
					FVector scaleSize(TempDamageText->ScaleSize, TempDamageText->ScaleSize, TempDamageText->ScaleSize);
					TempDamageText->SetActorScale3D(scaleSize);
					FVector dir = hero->GetActorLocation() - GetActorLocation();
					dir.Normalize();
					TempDamageText->FlyDirection = dir;
				}
			}
			hero->BuffQueue.Append(Buffs);
		}
	}
	AttackCollision.Empty();
}

// Called when the game starts or when spawned
void AFlySkillActor::BeginPlay()
{
	Super::BeginPlay();
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AFlySkillActor::OnBeginAttackOverlap);
	PrepareDestory = false;
}

// Called every frame
void AFlySkillActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (!IsReadyToStart)
	{
		return;
	}
	if (AttackCollision.Num() > 0)
	{
		Injury();
	}
	float move = DeltaTime * MoveSpeed;
	FVector ourpos = GetActorLocation();
	FVector dstpos;
	if (UseTargetLocation)
	{
		dstpos = TargetLocation;
	}
	else
	{
		dstpos = TargetActor->GetActorLocation();;
	}
	float dis = FVector::Dist(ourpos, dstpos);
	if (move >= dis)
	{
		SetActorLocation(dstpos);
	}
	else
	{
		FVector dir = dstpos - ourpos;
		dir.Normalize();
		dir *= move;
		SetActorLocation(ourpos + dir);
	}
	if (dis < 1 && !PrepareDestory)
	{
		BulletParticle->SetActive(false);
		PrepareDestory = true;
		DestoryCount = 0;
	}
	if (PrepareDestory)
	{
		DestoryCount += DeltaTime;
		if (DestoryCount >= DestroyDelay)
		{
			this->Destroy();
		}
	}
}

void AFlySkillActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFlySkillActor, TargetLocation);
	DOREPLIFETIME(AFlySkillActor, TargetActor);
	DOREPLIFETIME(AFlySkillActor, TeamId);
}
