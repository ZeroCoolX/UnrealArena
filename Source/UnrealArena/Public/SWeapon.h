// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

// contains info of a single hitscan weapon line trace
USTRUCT()
struct FHitscanTrace {
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class UNREALARENA_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	// Push this request to the hosting server, guaranteed to get there eventually, required when specifying push to server
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	void StartFire();
	void StopFire();

protected:
	virtual void BeginPlay() override;

	FVector Shoot(AActor* own);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	/* Bullet spread in degrees */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.f))
	float BulletSpread;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFiredTime;

	// RPM - Rounds per minute fired
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// Derived from RPM
	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing=OnRep_HitscanTrace)
	FHitscanTrace HitscanTrace;

	UFUNCTION()
	void OnRep_HitscanTrace();

private:
	// Weapon effects
	void PlayImpactEffect(EPhysicalSurface surfaceType, FVector impactPoint);
	void PlayShotEffects(FVector targetPoint);
	void PlayMuzzleFlashEffect();
	void PlaySmokeTrailEffect(FVector targetPoint);
	void ShakeCamera();
};
