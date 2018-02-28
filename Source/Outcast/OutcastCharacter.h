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

UCLASS()
class OUTCAST_API AOutcastCharacter : public ACharacter
{
	GENERATED_BODY()

  //******** COMPONENTS ********
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Body;
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* Weapon;
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

  //******** ANIMATION ********
  UOutcastAnimInstance* Anim;
  //******** ANIMATION ********

  //******** BASIC MOVEMENT ********
  UPROPERTY(Replicated)
  float Speed;
  const float MaxSpeed = 100.0f;
  const float MinSpeed = 0.0f;
  FVector Direction;
  UPROPERTY(Replicated)
  float WalkPlayrate;
  UPROPERTY(Replicated)
  FRotator LegsRotation;
  UPROPERTY(Replicated)
  FRotator TorsoRotation;
  UPROPERTY(Replicated)
  FRotator CharacterRotation;
  UPROPERTY(Replicated)
  FVector CharacterLocation;

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetSpeed(const float NewSpeed);
  void SetSpeed(const float NewSpeed);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetWalkPlayrate(const float NewWalkPlayrate);
  void SetWalkPlayrate(const float NewWalkPlayrate);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetLegsRotation(const FRotator NewLegsRotation);
  void SetLegsRotation(const FRotator NewLegsRotation);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetTorsoRotation(const FRotator NewTorsoRotation);
  void SetTorsoRotation(const FRotator NewTorsoRotation);

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetCharacterRotation(const FRotator NewCharacterRotation);
  void SetCharacterRotation(const FRotator NewCharacterRotation);

  UPROPERTY(Replicated)
  EJump Jumping;
  float JumpHeight;
  float JumpStartLocZ;
  const float JumpHeightLimit = 1000.0f;
  const float MinJumpHeight   = 75.0f;

  float BunnyHopSpeedRatio;
  const float DefaultJumpSpeedRatio     = 120.0f;
  const float BunnyHopMaxHeight         = 100.0f;
  const float BunnyHopFastestSpeedRatio = 20.0f;


  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetJumping(const EJump NewJumping);
  void SetJumping(const EJump NewJumping);
  //******** BASIC MOVEMENT ********

  //******** ATTACKING ********
  UPROPERTY(Replicated)
  EAttack Attacking;

  float LeftMouseTimer;

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SetAttack(const EAttack NewAttack);
  void SetAttack(const EAttack NewAttack);
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
  //******** TICK FUNCTIONS ********

public:
	AOutcastCharacter();

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(
    class UInputComponent* PlayerInputComponent) override;
};
