// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_NebulaGameMode.h"
#include "Project_NebulaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_NebulaGameMode::AProject_NebulaGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
