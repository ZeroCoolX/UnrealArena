// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

/**
 * 
 */
UCLASS()
class UNREALARENA_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
protected:

	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;
	
	int32 NrOfBotsToSpawn;
	int32 WaveLevel;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();
	
	void StartWave();

	void EndWave();

	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);
};
