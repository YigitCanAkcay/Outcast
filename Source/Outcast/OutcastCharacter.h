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

USTRUCT()
struct FReplicatedData
{
  GENERATED_BODY()

  UPROPERTY()
  float Speed;
  UPROPERTY()
  float WalkPlayrate;
  UPROPERTY()
  FRotator LegsRotation;
  UPROPERTY()
  FRotator TorsoRotation;
  UPROPERTY()
  FRotator CharacterRotation;
  UPROPERTY()
  EJump Jumping;
  UPROPERTY()
  EAttack Attacking;
  UPROPERTY()
  FVector CharacterLocation;
  UPROPERTY()
  int Health;
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
  UPROPERTY(Replicated)
  FReplicatedData ReplicatedData;
  void ExtractReplicatedData();
  void FillReplicatedData();

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetReplicatedData(const FReplicatedData NewReplicatedData);
  void SetReplicatedData(const FReplicatedData NewReplicatedData);
  //******** REPLICATION ********

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

  void SetSpeed(const float NewSpeed);
  void SetWalkPlayrate(const float NewWalkPlayrate);
  void SetLegsRotation(const FRotator NewLegsRotation);
  void SetTorsoRotation(const FRotator NewTorsoRotation);
  void SetCharacterRotation(const FRotator NewCharacterRotation);

  EJump Jumping;
  float JumpHeight;
  float JumpStartLocZ;
  const float JumpHeightLimit = 1000.0f;
  const float MinJumpHeight   = 75.0f;

  float BunnyHopSpeedRatio;
  const float DefaultJumpSpeedRatio     = 120.0f;
  const float BunnyHopMaxHeight         = 100.0f;
  const float BunnyHopFastestSpeedRatio = 20.0f;

  void SetJumping(const EJump NewJumping);
  //******** BASIC MOVEMENT ********

  //******** ATTACKING ********
  float LeftMouseTimer;

  UFUNCTION(BlueprintCallable)
  int GetHealth();

  TMap<AOutcastCharacter*, float> DamageTakenBy;

  EAttack Attacking;
  int Health;

  void SetAttack(const EAttack NewAttack);
  void SetHealth(const int NewHealth);

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
  enum class EKeys
  {
    W,
    A,
    S,
    D,
    Space
  };
  enum class EMouse
  {
    Left,
    Right
  };

  TMap<EKeys, bool> KeyMap;
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

  FVector2D MouseInput;
  TMap<EMouse, bool> MouseMap;
  void MouseLeftPressed();
  void MouseRightPressed();
  void MouseLeftReleased();
  void MouseRightReleased();
  void MouseUpDown(const float AxisValue);
  void MouseRightLeft(const float AxisValue);
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
