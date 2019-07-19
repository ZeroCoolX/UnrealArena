// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "../Public/SHealthComponent.h"
#include "../Public/SGameState.h"


ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.f;

	GameStateClass = ASGameState::StaticClass();

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
	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
	++WaveLevel;

	NrOfBotsToSpawn = 2 * WaveLevel;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);
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

	SetWaveState(EWaveState::WaitingToStart);
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

	SetWaveState(EWaveState::WaveComplete);

	// Only prepare if we got here meaning that no bot is alive
	PrepareForNextWave();
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn()) {
			APawn* Pwn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(Pwn->GetComponentByClass(USHealthComponent::StaticClass()));
			// If this check fails it will breakpoint here
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.f) {
				// someone is still kickin'
				return;
			}
		}
	}

	GameOver();
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::GameOver() {
	EndWave();

	SetWaveState(EWaveState::GameOver);

	// Finish up the match and present stats to players
	UE_LOG(LogTemp, Log, TEXT("Game Over - All players are dead"));
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS)) {
		GS->SetWaveState(NewState);
	}
}
