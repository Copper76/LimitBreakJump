// Copyright Epic Games, Inc. All Rights Reserved.

#include "LimitBreakProjectGameMode.h"
#include "LimitBreakProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALimitBreakProjectGameMode::ALimitBreakProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
