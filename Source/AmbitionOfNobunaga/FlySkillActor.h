// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HeroBuff.h"
#include "FlySkillActor.generated.h"

class AHeroCharacter;

UCLASS()
class AMBITIONOFNOBUNAGA_API AFlySkillActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UFUNCTION()
	void OnBeginAttackOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

	UFUNCTION(Server, WithValidation, Reliable)
	void Injury();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(Category = "Equipment", VisibleAnywhere, BlueprintReadOnly)
	class UCapsuleComponent * CapsuleComponent;

	UPROPERTY(Category = "FlySkill", VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent * BulletParticle;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite, Replicated)
	int32 TeamId;

	UPROPERTY(Category = "FlySkill", VisibleAnywhere, BlueprintReadOnly)
	USceneComponent * Scene;

	UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadWrite)
	float DestroyDelay;

	UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadWrite)
	float DestoryCount;

	UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadWrite)
	uint32  PrepareDestory: 1;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly)
	uint32 IsFixdLength: 1;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly)
	float PhysicalDamage;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly)
	float MagicDamage;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly)
	uint32 UseTargetLocation: 1;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly)
	uint32 UseTargetActor: 1;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite, Replicated,
	          meta = (EditCondition = "UseTargetLocation"))
	FVector TargetLocation;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite, Replicated,
	          meta = (EditCondition = "UseTargetActor"))
	AActor * TargetActor;

	UPROPERTY(Category = "FlySkill", VisibleAnywhere, BlueprintReadOnly)
	TArray<AHeroCharacter*> AttackCollision;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> AlreadyDamageActor;

	UPROPERTY(Category = "RangeSkill", EditAnywhere, BlueprintReadWrite)
	TArray<UHeroBuff*> Buffs;

	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadWrite)
	bool IsReadyToStart;
};
