// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STickerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class UNREALARENA_API ASTickerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTickerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category="Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp,
		float Health,
		float DeltaHealth,
		const class UDamageType* DamageType,
		class AController* InstigatedBy,
		AActor* DamageCauser);

	FVector GetNextPathPoint();

	// Next point in the navigation pathfinding
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float TargetDistanceThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	bool bUseVelocityChange;

	// Dynamic material to pulse
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	UParticleSystem* ExplosionEffect;

	bool bExploded;

	bool bSelfDestructInitiated;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float ExplosionRadius;
	
	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

	// Audio assets
	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	USoundCue* ExplodeSound;

	void OnCheckNearbyBots();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
