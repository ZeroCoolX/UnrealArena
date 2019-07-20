// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "../Public/SGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;
	bIsDead = false;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook in in we are the server
	if (GetOwnerRole() == ROLE_Authority) {
		AActor* Owner = GetOwner();
		// subscribe ourselves to whichever component we belong to
		if (Owner) {
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = OldHealth - Health;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, 
	float Damage,
	const class UDamageType* DamageType, 
	class AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (Damage <= 0.f || bIsDead) {
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);
													// turns float into character array
	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));

	bIsDead = Health <= 0.f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead) {
		// Should only be done on the server
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM) {
			GM->OnActorKilled.Broadcast(GetOwner(),		// Victim
				DamageCauser,							// Killer
				InstigatedBy);							// Killer Owner
		}
	}
}

void USHealthComponent::Heal(float HealAmount)
{
	// Do not heal dead thing or with 0% healing
	if (HealAmount <= 0.f || Health <= 0.f) { return; }

	Health = FMath::Clamp(Health + HealAmount, 0.f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	// -HealAmount
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// replicate to any relevant client thats connected to the server
	DOREPLIFETIME(USHealthComponent, Health);
}

