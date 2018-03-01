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

  //NewPlayer->GetPawn()->UnPossessed();

  //AOutcastCharacter* NewCharacter = GetWorld()->SpawnActor<AOutcastCharacter>(AOutcastCharacter::StaticClass(), FVector(0, 0, 120), FRotator::ZeroRotator);
  
  //NewPlayer->Possess(NewCharacter);
}

void AOutcastGameMode::Logout(AController* Player)
{
  Super::Logout(Player);

  PlayerList.Remove(Cast<APlayerController>(Player));
}

void AOutcastGameMode::Respawn(APlayerController* Player)
{
  APawn* OldPlayer = Player->GetPawn();
  OldPlayer->UnPossessed();
  OldPlayer->Destroy();

  AActor* PlayerStart = ChoosePlayerStart(Player);

  AOutcastCharacter* NewCharacter = GetWorld()->SpawnActor<AOutcastCharacter>(AOutcastCharacter::StaticClass(), 
    PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());

  Player->Possess(NewCharacter);
  NewCharacter->SetMyPlayerController(Player);
}