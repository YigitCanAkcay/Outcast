#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Animation/AnimBlueprint.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

#include "OutcastAnimInstance.h"

#include "OutcastCharacter.generated.h"

UENUM()
enum class EJump
{
  NONE,
  Upwards,
  Downwards,
  BunnyHop
};

UENUM()
enum class EAttack
{
  NONE    = 0,
  Left    = 1,
  Right   = 2,
  Forward = 3
};

UENUM()
enum class EKey
{
  W,
  A,
  S,
  D,
  Space
};

UENUM()
enum class EMouse
{
  Left,
  Right
};

USTRUCT()
struct FReplicatedData
{
  GENERATED_BODY()

  UPROPERTY()
  EJump Jumping;
  UPROPERTY()
  int Health;
  UPROPERTY()
  FVector CharacterRotation;
  UPROPERTY()
  FVector CharacterLocation;
};

USTRUCT()
struct FReplicatedAnimData
{
  GENERATED_BODY()

  UPROPERTY()
  float Speed;
  UPROPERTY()
  float WalkPlayrate;
  UPROPERTY()
  FVector LegsRotation;
  UPROPERTY()
  FVector TorsoRotation;
  UPROPERTY()
  EAttack Attacking;
};

USTRUCT()
struct FState
{
  GENERATED_BODY()

  UPROPERTY()
  FReplicatedData Data;

  UPROPERTY()
  FReplicatedAnimData AnimData;

  UPROPERTY()
  float Time;
};

UCLASS()
class OUTCAST_API AOutcastCharacter : public ACharacter
{
	GENERATED_BODY()

  //******** COMPONENTS ********
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Body;
  UPROPERTY(VisibleAnywhere, Category = StaticMesh)
    UStaticMeshComponent* Weapon;
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Eye_R;
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Eye_L;
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Armor;

  UCapsuleComponent* Capsule;

  UPROPERTY(VisibleAnywhere, Category = Camera)
    UCameraComponent* Camera;

  UCharacterMovementComponent* Movement;
  //******** COMPONENTS ********

  APlayerController* MyPlayerController;

  //******** ANIMATION ********
  UOutcastAnimInstance* Anim;
  //******** ANIMATION ********

  //******** REPLICATION ********
  UPROPERTY(ReplicatedUsing = OnRep_ApplyState)
  FState CurrentState;
  void ExtractState();
  UFUNCTION()
  void OnRep_ApplyState();
  void CreateState();
  //******** REPLICATION ********

  //******** HELPERS ********
  FVector ReturnFVector(const FRotator& Rotator);
  FRotator ReturnFRotator(const FVector& Vector);
  //******** HELPERS ********

  //******** BASIC MOVEMENT ********
  float Speed;
  const float MaxSpeed = 100.0f;
  const float MinSpeed = 0.0f;
  FVector Direction;
  float WalkPlayrate;
  FRotator LegsRotation;
  FRotator TorsoRotation;
  FRotator CharacterRotation;
  FVector CharacterLocation;

  EJump Jumping;
  float JumpHeight;
  float JumpStartLocZ;
  const float JumpHeightLimit = 1000.0f;
  const float MinJumpHeight   = 75.0f;

  float BunnyHopSpeedRatio;
  const float DefaultJumpSpeedRatio     = 120.0f;
  const float BunnyHopMaxHeight         = 100.0f;
  const float BunnyHopFastestSpeedRatio = 20.0f;
  //******** BASIC MOVEMENT ********

  //******** ATTACKING ********
  UFUNCTION(BlueprintCallable)
  int GetHealth();

  TMap<AOutcastCharacter*, float> DamageTakenBy;

  EAttack Attacking;
  int Health;

  UFUNCTION()
  void BodyOverlapBegin(
    UPrimitiveComponent* OverlappedComp, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult);
  UFUNCTION()
  void BodyOverlapEnd(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex);
  //******** ATTACKING ********

  //******** PLAYER INPUT ********
  TMap<EKey, bool> KeyMap;
  void WPressed();
  void WReleased();

  void APressed();
  void AReleased();

  void SPressed();
  void SReleased();

  void DPressed();
  void DReleased();

  void SpacePressed();
  void SpaceReleased();

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetKey(const EKey Key, const bool bIsPressed);

  FVector2D MouseInput;
  TMap<EMouse, bool> MouseMap;
  void MouseLeftPressed();
  void MouseRightPressed();
  void MouseLeftReleased();
  void MouseRightReleased();
  void MouseUpDown(const float AxisValue);
  void MouseRightLeft(const float AxisValue);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetMouse(const EMouse Key, const bool bIsPressed);
  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetMouseInput(const FVector2D NewMouseInput);
  //******** PLAYER INPUT ********

  //******** COLLISION ********
  UFUNCTION()
  void OnHit(
    UPrimitiveComponent* HitComp,
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, 
    const FHitResult& Hit);
  //******** COLLISION ********

  //******** TICK FUNCTIONS ********
  void LookAround();
  void MoveAround();
  void Jump();
  void Attack(const float DeltaTime);
  void Alive(const float DeltaTime);
  //******** TICK FUNCTIONS ********

public:
	AOutcastCharacter();

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void SetMyPlayerController(APlayerController* const NewPlayerController);

  EAttack GetAttack();

	virtual void SetupPlayerInputComponent(
    class UInputComponent* PlayerInputComponent) override;
};
