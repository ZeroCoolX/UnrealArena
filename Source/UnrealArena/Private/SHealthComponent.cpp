// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	// subscribe ourselves to whichever component we belong to
	if (Owner) {
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}

	Health = DefaultHealth;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, 
	float Damage,
	const class UDamageType* DamageType, 
	class AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (Damage <= 0.f) {
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);
													// turns float into character array
	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

