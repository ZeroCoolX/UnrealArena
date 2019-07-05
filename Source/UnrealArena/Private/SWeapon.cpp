// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../UnrealArena.h"

static int32 DrawDebugWeapon = 0;
FAutoConsoleVariableRef CVARDrawDebugWeapon (
	TEXT("ARENA.DebugWeapons"), 
	DrawDebugWeapon,
	TEXT("Draw Debug Lines for weapons"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	TracerTargetName = "BeamEnd";
	MuzzleSocketName = "MuzzleFlashSocket";
}


void ASWeapon::Fire() {
	AActor* Owner = GetOwner();
	if (Owner) {
		FVector targetPoint = Shoot(Owner);
		PlayShotEffects(targetPoint);
	}
}

FVector ASWeapon::Shoot(AActor* own) {
	// Setup basic line trace
	FVector EyeLocation;
	FRotator EyeRotation;
	own->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();

	FVector TraceEnd = EyeLocation + (ShotDirection * 10000); // arbitrary long length for hit scan
	// Particle "Target" parameter
	FVector TracerEndPoint = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(own);
	QueryParams.AddIgnoredActor(this);
	// More expensive, but also tells us exactly where the collision occurred
	// Useful for spawning effects on the hit
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;

	bool blockingHit = GetWorld()->LineTraceSingleByChannel(
		Hit,					// Struct to store the hit data in
		EyeLocation,			// Start location
		TraceEnd,				// End location
		COLLISION_WEAPON,		// Similar to layer
		QueryParams);

	if (blockingHit) {
		AActor* HitActor = Hit.GetActor();

		UGameplayStatics::ApplyPointDamage(
			HitActor,							// Actor that got hit
			20.f,								// damage to apply
			ShotDirection,						// direction shot came from
			Hit,								// struct containing all hit data
			own->GetInstigatorController(),	// who triggered the damage event
			this,								// who is applying the damage to the AActor
			DamageType);						// damage type (using unreal defaults) - can be extended for specific use

		PlayImpactEffect(&Hit);

		if (DrawDebugWeapon) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
		}

		TracerEndPoint = Hit.ImpactPoint;
	}

	return TracerEndPoint;
}

void ASWeapon::PlayShotEffects(FVector targetPoint)
{
	PlayMuzzleFlashEffect();
	PlaySmokeTrailEffect(targetPoint);
	ShakeCamera();
}

void ASWeapon::PlayImpactEffect(FHitResult* hit) {
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(hit->PhysMaterial.Get());

	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType) {
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	// Spawn impact effect
	if (SelectedEffect) {
		//UE_LOG(LogTemp, Log, TEXT("playing impact effect"));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, hit->ImpactPoint, hit->ImpactNormal.Rotation());
	}
}

void ASWeapon::PlayMuzzleFlashEffect() {
	// Play flash effect
	if (MuzzleFlashEffect) {
		// Ensures the effect follows the parent location
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, MeshComp, MuzzleSocketName);
	}
}

void ASWeapon::PlaySmokeTrailEffect(FVector targetPoint) {
	if (TracerEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerEffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerEffectComp) {
			TracerEffectComp->SetVectorParameter(TracerTargetName, targetPoint);
		}
	}
}

void ASWeapon::ShakeCamera() {
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner) {
		APlayerController* playerController = Cast<APlayerController>(Owner->GetController());
		if (playerController) {
			playerController->ClientPlayCameraShake(FireCamShake);
		}
	}
}

