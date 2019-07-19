// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

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
	
protected:

	FTimerHandle TimerHandle_BotSpawner;
	
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
};
