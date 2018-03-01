#include "OutcastGameMode.h"


AOutcastGameMode::AOutcastGameMode()
{
  DefaultPawnClass = AOutcastCharacter::StaticClass();
  //DefaultPawnClass = ASpectatorPawn::StaticClass();
}

void AOutcastGameMode::PostLogin(APlayerController* NewPlayer)
{
  Super::PostLogin(NewPlayer);

  PlayerList.Add(NewPlayer);

  Cast<AOutcastCharacter>(NewPlayer->GetPawn())->SetMyPlayerController(NewPlayer);
}

void AOutcastGameMode::Logout(AController* Player)
{
  Super::Logout(Player);

  PlayerList.Remove(Cast<APlayerController>(Player));
}

void AOutcastGameMode::Respawn(APlayerController* Player)
{
  APawn* OldPlayer = Player->GetPawn();
  if (OldPlayer)
  {
    OldPlayer->UnPossessed();
    OldPlayer->Destroy();
  }

  AActor* PlayerStart = ChoosePlayerStart(Player);

  AOutcastCharacter* NewCharacter = GetWorld()->SpawnActor<AOutcastCharacter>(AOutcastCharacter::StaticClass(), 
    PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());

  Player->Possess(NewCharacter);
  NewCharacter->SetMyPlayerController(Player);
}