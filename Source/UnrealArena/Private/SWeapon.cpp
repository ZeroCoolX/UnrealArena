// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../UnrealArena.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

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

	BaseDamage = 25.f;
	RateOfFire = 600;
	BulletSpread = 2.f;

	// Allows this object to be spawned on server when its spawned in clients
	SetReplicates(true);

	// Network update throttle time (fps)
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60.f / RateOfFire; //Rounds per minute
}


void ASWeapon::Fire() {
	// Client only needs to tell the server as well
	if (Role < ROLE_Authority) {
		ServerFire();
	}

	AActor* Owner = GetOwner();
	if (Owner) {
		FVector targetPoint = Shoot(Owner);
		PlayShotEffects(targetPoint);
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}
// Intended for anti hax
bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.f);

	GetWorldTimerManager().SetTimer(
		TimerHandle_TimeBetweenShots,	// timer instance
		this,							// parent
		&ASWeapon::Fire,				// function delegate
		TimeBetweenShots,				// interval
		true,							// looping
		FirstDelay);					// first delay - 0 = immediately on click
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

FVector ASWeapon::Shoot(AActor* own) {
	// Setup basic line trace
	FVector EyeLocation;
	FRotator EyeRotation;
	own->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();

	// Ad some randomness if this is an AI shooting
	float HalfRad = FMath::DegreesToRadians(BulletSpread);
	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

	FVector TraceEnd = EyeLocation + (ShotDirection * 10000); // arbitrary long length for hit scan
	// Particle "Target" parameter
	FVector TracerEndPoint = TraceEnd;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(own);
	QueryParams.AddIgnoredActor(this);
	// More expensive, but also tells us exactly where the collision occurred
	// Useful for spawning effects on the hit
	QueryParams.bTraceComplex = true;
	// Necessary for us to determine which surface type was hit
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	bool blockingHit = GetWorld()->LineTraceSingleByChannel(
		Hit,					// Struct to store the hit data in
		EyeLocation,			// Start location
		TraceEnd,				// End location
		COLLISION_WEAPON,		// Similar to layer
		QueryParams);

	if (blockingHit) {
		AActor* HitActor = Hit.GetActor();

		// Head shot
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		float ActualDamage = BaseDamage;
		// check for head shot
		if (SurfaceType == SURFACE_FLESHVULNERABLE) {
			ActualDamage *= 4.f;
		}

		UGameplayStatics::ApplyPointDamage(
			HitActor,							// Actor that got hit
			ActualDamage,								// damage to apply
			ShotDirection,						// direction shot came from
			Hit,								// struct containing all hit data
			own->GetInstigatorController(),	// who triggered the damage event
			own,								// who is applying the damage to the AActor
			DamageType);						// damage type (using unreal defaults) - can be extended for specific use

		PlayImpactEffect(SurfaceType, Hit.ImpactPoint);

		TracerEndPoint = Hit.ImpactPoint;
	}

	if (Role == ROLE_Authority) {
		HitscanTrace.TraceTo = TracerEndPoint;
		HitscanTrace.SurfaceType = SurfaceType;
	}

	LastFiredTime = GetWorld()->TimeSeconds;

	return TracerEndPoint;
}

void ASWeapon::OnRep_HitscanTrace()
{
	// Play cosmetic effects
	PlayShotEffects(HitscanTrace.TraceTo);

	PlayImpactEffect(HitscanTrace.SurfaceType, HitscanTrace.TraceTo);
}

void ASWeapon::PlayShotEffects(FVector targetPoint)
{
	PlayMuzzleFlashEffect();
	PlaySmokeTrailEffect(targetPoint);
	ShakeCamera();
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface surfaceType, FVector impactPoint) {
	//if (!surfaceType) { return; }

	UParticleSystem* SelectedEffect = nullptr;

	switch (surfaceType) {
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = impactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		
		//UE_LOG(LogTemp, Log, TEXT("playing impact effect"));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, impactPoint, ShotDirection.Rotation());
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

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// replicate to any relevant client thats connected to the server
	// replicate the HitscanTrace struct to all clients - but NOT if its the client who owns this weapon
	DOREPLIFETIME_CONDITION(ASWeapon, HitscanTrace, COND_SkipOwner);
}


