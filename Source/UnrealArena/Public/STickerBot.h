// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STickerBot.generated.h"

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

	FVector GetNextPathPoint();

	// Next point in the navigation pathfinding
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	float TargetDistanceThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "TickerBot")
	bool bUseVelocityChange;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
