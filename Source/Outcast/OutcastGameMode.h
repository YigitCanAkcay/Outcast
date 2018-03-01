#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Runtime/Engine/Classes/GameFramework/SpectatorPawn.h"

#include "OutcastCharacter.h"

#include "OutcastGameMode.generated.h"

UCLASS()
class OUTCAST_API AOutcastGameMode : public AGameModeBase
{
  GENERATED_BODY()

public:
  AOutcastGameMode();

  void PostLogin(APlayerController* NewPlayer) override;
  void Logout(AController* Player) override;

  void Respawn(APlayerController* Player);

private:
  TArray<APlayerController*> PlayerList;
	
	
};
