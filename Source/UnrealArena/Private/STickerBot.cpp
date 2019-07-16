// Fill out your copyright notice in the Description page of Project Settings.

#include "STickerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "../Public/SHealthComponent.h"

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

	bUseVelocityChange = false;
	MovementForce = 1000;
	TargetDistanceThreshold = 100;
}

// Called when the game starts or when spawned
void ASTickerBot::BeginPlay()
{
	Super::BeginPlay();
	
	// find initial move to point
	NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ASTickerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

void ASTickerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float DeltaHealth, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// Explode on hitpoints == 0

	// @TODO: Pulse the material on hit

	// Need char* for the logging macro - not just primitive string
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

FVector ASTickerBot::GetNextPathPoint() {
	// Hack to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerPawn)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

		// First path point is current location, want next one
		if (NavPath->PathPoints.Num() > 1)
		{
			// Return next point in path
			return NavPath->PathPoints[1];
		}
	}
	// Failed to find path
	return GetActorLocation();
}


