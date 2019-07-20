// Fill out your copyright notice in the Description page of Project Settings.

#include "STickerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "../Public/SHealthComponent.h"
#include "../Public/SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTickerBot::ASTickerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTickerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	TargetDistanceThreshold = 100;
	ExplosionDamage = 40;
	ExplosionRadius = 200;
	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ASTickerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority) {
		// find initial move to point
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby tickers
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTickerBot::OnCheckNearbyTickers, 1.f, true);
	}
}

// Called every frame
void ASTickerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded) {
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= TargetDistanceThreshold) {
			// find the next path point
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else {
			// Keep moving towards the current target point
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Green, false, 0.f, 0, 1.f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Green, false, 0.f, 1.f);
	}
}

void ASTickerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float DeltaHealth, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	// stops duplication
	if (MatInst == nullptr) {
		// Create the pulse effect
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0)); // Dynamic must be used so only the effected trackerbot is modified and not all instances in the world
	}

	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	// Need char* for the logging macro - not just primitive string
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	// Explode on hitpoints == 0
	if (Health <= 0.f) {
		SelfDestruct();
	}
}

FVector ASTickerBot::GetNextPathPoint() {
	// Hack to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerPawn)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

		// First path point is current location, want next one
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in path
			return NavPath->PathPoints[1];
		}
	}
	// Failed to find path
	return GetActorLocation();
}

void ASTickerBot::SelfDestruct()
{
	if (bExploded) {
		return;
	}
	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false,	// self 
		true);						// children
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Only run on server
	if (Role == ROLE_Authority) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.f, 0.f, 1.f);

		// Delete actor immediately
		SetLifeSpan(2.f);
	}
}

void ASTickerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bSelfDestructInitiated || bExploded) { return; }

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn) {
		// We overlapped with a player!

		if (Role == ROLE_Authority) {
			// Start self destruction sequence
			// Sequence is dependent on how much health it has
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTickerBot::DamageSelf, SelfDamageInterval, true, 0.f);
		}

		bSelfDestructInitiated = true;

		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
	}
}

void ASTickerBot::DamageSelf() {
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTickerBot::OnCheckNearbyTickers()
{
	// distance to check for nearby tickers
	const float Radius = 600;

	// create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	//Only find Pawns (eg. players and AI)
	FCollisionObjectQueryParams QueryParams;
	// Our ticker bot's mesh component is set to Physics Body in blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.f);

	// Collect the nr of tickers in the area
	int32 NrOfTickers = 0;
	for (FOverlapResult OverHit : Overlaps) {
		// check if we overlapped with another ticker (ignoring players and other bot types)
		ASTickerBot* Ticker = Cast<ASTickerBot>(OverHit.GetActor());
		if (Ticker && Ticker != this) {
			++NrOfTickers;
		}
	}

	const int32 MaxPowerLevel = 4;

	// Clamp between min=0 and max=4
	PowerLevel = FMath::Clamp(NrOfTickers, 0, MaxPowerLevel);

	// Update the material color
	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MeshComp) {
		// Convert to a float between 0 and 1 like an Alpha value of a texture. Now the material can be set up without having to know the max power level
		// which can be tweaked many times by gameplay decisions
		float PowerLevelAlpha = PowerLevel / (float)MaxPowerLevel;

		MatInst->SetScalarParameterValue("PowerLevelAlpha", PowerLevelAlpha);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.f, true);
}

