// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;

UCLASS()
class UNREALARENA_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual FVector GetPawnViewLocation() const override;

protected:
	virtual void BeginPlay() override;

	/******** Input /********/
	// Movement
	void MoveForward(float Direction);
	void MoveRight(float Direction);
	// Crouch
	void BeginCrouch();
	void EndCrouch();
	// ADS
	void BeginZoom();
	void EndZoom();

	bool bZooming;
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;
	
	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1f, ClampMax = 100.f))
	float ZoomInterpSpeed;

	float DefaultFOV;

	/******** Weapon /********/
	void Fire();
	void StartFire();
	void StopFire();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		FName WeaponAttachSocketName;

	/******** Camera Control /********/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
};
