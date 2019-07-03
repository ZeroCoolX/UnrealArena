// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/SHitscanWeapon.h"


// Sets default values
ASHitscanWeapon::ASHitscanWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void ASHitscanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASHitscanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

