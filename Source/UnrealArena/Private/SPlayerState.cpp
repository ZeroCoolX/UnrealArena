// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"




void ASPlayerState::AddScore(float ScoreDelta)
{
	// Utilizes the inherited Score member variable
	Score += ScoreDelta;
}
