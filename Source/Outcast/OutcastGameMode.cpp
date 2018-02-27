#include "OutcastGameMode.h"


AOutcastGameMode::AOutcastGameMode()
{
  DefaultPawnClass = AOutcastCharacter::StaticClass();//ASpectatorPawn::StaticClass();
}

void AOutcastGameMode::PostLogin(APlayerController* NewPlayer)
{
  Super::PostLogin(NewPlayer);

  PlayerList.Add(NewPlayer);

  //NewPlayer->GetPawn()->UnPossessed();

  //AOutcastCharacter* NewCharacter = GetWorld()->SpawnActor<AOutcastCharacter>(AOutcastCharacter::StaticClass(), FVector(0, 0, 120), FRotator::ZeroRotator);
  
  //NewPlayer->Possess(NewCharacter);
}

void AOutcastGameMode::Logout(AController* Player)
{
  Super::Logout(Player);

  PlayerList.Remove(Cast<APlayerController>(Player));
}

