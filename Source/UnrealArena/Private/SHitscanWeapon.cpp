// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SHitscanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

// Sets default values
ASHitscanWeapon::ASHitscanWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleFlashSocket";
}

// Called when the game starts or when spawned
void ASHitscanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASHitscanWeapon::Fire() {
	// TRace the world from pawn eyes to crosshair location

	AActor* Owner = GetOwner();
	if (Owner) {
		// Setup basic line trace
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000); // arbitrary long length for hitscan

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		// More expensive, but also tells us exactly where the collision occurred
		// Useful for spawning effects on the hit
		QueryParams.bTraceComplex = true;

		FHitResult Hit;
		bool blockingHit = GetWorld()->LineTraceSingleByChannel(
			Hit,				// Struct to store the hit data in
			EyeLocation,		// Start location
			TraceEnd,			// End location
			ECC_Visibility,		// Similar to layer
			QueryParams);

		if (blockingHit) {
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(
				HitActor,							// Actor that got hit
				20.f,								// damage to apply
				ShotDirection,						// direction shot came from
				Hit,								// struct containing all hit data
				Owner->GetInstigatorController(),	// who triggered the damage event
				this,								// who is applying the damage to the AActor
				DamageType);						// damage type (using unreal defaults) - can be extended for specific use

			// Spawn impact effect
			if (ImpactEffect) {
				//UE_LOG(LogTemp, Log, TEXT("playing impact effect"));
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
		}

		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.f);

		// Play flash effect
		if (MuzzleFlashEffect) {
			// Ensures the effect follows the parent location
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, MeshComp, MuzzleSocketName);
		}
	}
}

// Called every frame
void ASHitscanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

