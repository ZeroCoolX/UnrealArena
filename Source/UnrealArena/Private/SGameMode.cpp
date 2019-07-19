// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "../Public/SHealthComponent.h"



ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f; // once a second
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}

void ASGameMode::StartWave()
{
	++WaveLevel;

	NrOfBotsToSpawn = 2 * WaveLevel;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	--NrOfBotsToSpawn;

	if (NrOfBotsToSpawn <= 0) {
		EndWave();
	}
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	bool bBotsLeftToSpawn = NrOfBotsToSpawn > 0;

	// don't check the rest because there are still bots to spawn
	if (bBotsLeftToSpawn || bIsPreparingForWave) {
		return;
	}

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {
		APawn* Pwn = It->Get();
		if (Pwn == nullptr || Pwn->IsPlayerControlled()) {
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(Pwn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.f) {
			return;
		}
	}

	// Only prepare if we got here meaning that no bot is alive
	PrepareForNextWave();
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}