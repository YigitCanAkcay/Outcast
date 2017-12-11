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

#include "OutcastAnimInstance.h"

#include "OutcastCharacter.generated.h"

UCLASS()
class OUTCAST_API AOutcastCharacter : public ACharacter
{
	GENERATED_BODY()

  //******** COMPONENTS ********
  UPROPERTY(VisibleAnywhere, Category = SkeletalMesh)
    USkeletalMeshComponent* SkeletalMeshComp;

  UCapsuleComponent* Capsule;

  UPROPERTY(VisibleAnywhere, Category = Camera)
    UCameraComponent* Camera;

  UCharacterMovementComponent* Movement;
  //******** COMPONENTS ********

  //******** ANIMATION ********
  UOutcastAnimInstance* Anim;
  //******** ANIMATION ********

  //******** BASIC MOVEMENT ********
  float Speed;
  FVector Direction;
  float WalkPlayrate;

  enum class EJump
  {
    NONE,
    Upwards,
    Downwards,
    BunnyHop
  };
  EJump Jumping;
  float JumpHeight;
  float JumpStartLocZ;
  const float JumpHeightLimit = 1000.0f;
  const float MinJumpHeight   = 75.0f;

  float BunnyHopSpeedRatio;
  const float DefaultJumpSpeedRatio = 50.0f;
  const float BunnyHopMaxHeight         = 100.0f;
  //******** BASIC MOVEMENT ********


  //******** KEYBOARD INPUT ********
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
  void MouseUpDown(const float AxisValue);
  void MouseRightLeft(const float AxisValue);
  //******** KEYBOARD INPUT ********

  UFUNCTION()
  void OnHit(
    UPrimitiveComponent* HitComp,
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, 
    const FHitResult& Hit);

public:
	AOutcastCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(
    class UInputComponent* PlayerInputComponent) override;

	
	
};
