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
#include "Kismet/GameplayStatics.h"
#include "Runtime/Core/Public/Misc/DateTime.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

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
  W     = 0,
  A     = 1,
  S     = 2,
  D     = 3,
  Space = 4
};

UENUM()
enum class EMouse
{
  Left  = 0,
  Right = 1
};

USTRUCT()
struct FMove
{
  GENERATED_BODY()

  FMove() 
    : 
    DeltaTime(0.0f),
    TimeStamp(0.0f), 
    ForwardDirection(0.0f), 
    SidewardDirection(0.0f),
    Acceleration(0.0f)
  {}

  UPROPERTY()
  float DeltaTime;

  UPROPERTY()
  float TimeStamp;

  UPROPERTY()
  float ForwardDirection;
  
  UPROPERTY()
  float SidewardDirection;

  UPROPERTY()
  float Acceleration;
};

USTRUCT()
struct FState
{
  GENERATED_BODY()

  UPROPERTY()
  FVector Location;
  
  UPROPERTY()
  FVector Rotation;

  UPROPERTY()
  float TimeStamp;
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

  //******** ASSETS ********
  TArray<USoundWave*> AttackSounds;
  TArray<USoundWave*> HitSounds;
  TArray<USoundWave*> HitVocalSounds;
  USoundWave* IdleHitSound;
  TArray<USoundWave*> StepSounds;
  TArray<USoundWave*> JumpVocalSounds;
  TArray<USoundWave*> AttackVocalSounds;

  UFUNCTION(BlueprintCallable)
  void PlayStepSound();
  UFUNCTION(BlueprintCallable)
  void PlayJumpVocalSound();
  UFUNCTION(BlueprintCallable)
  void PlayAttackVocalSound();
  UFUNCTION(BlueprintCallable)
  void PlayAttackSound();
  UFUNCTION(BlueprintCallable)
  void PlayHitSound();
  UFUNCTION(BlueprintCallable)
  void PlayHitVocalSound();
  UFUNCTION(BlueprintCallable)
  void PlayIdleHitSound();

  USoundAttenuation* Attenuation;
  //******** ASSETS ********

  APlayerController* MyPlayerController;

  //******** ANIMATION ********
  UOutcastAnimInstance* Anim;
  //******** ANIMATION ********

  //******** REPLICATION ********
  UPROPERTY(ReplicatedUsing = OnRep_ServerState)
  FState ServerState;
  UFUNCTION()
  void OnRep_ServerState();
  FState CreateState(const float TimeStamp);

  TArray<FState> ServerStates;

  FMove CreateMove(const float DeltaTime);
  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SendMove(const FMove Move);

  TArray<FMove> UnacknowledgedMoves;
  void CleanUnacknowledgedMoves();
  void ReconcileWithServer();
  //******** REPLICATION ********

  //******** HELPERS ********
  FVector ReturnFVector(const FRotator& Rotator);
  FRotator ReturnFRotator(const FVector& Vector);
  //******** HELPERS ********

  //******** BASIC MOVEMENT ********
  float ForwardDirection;
  float SidewardDirection;
  int Acceleration;
  int MaxWalkSpeed;

  void RegulateAcceleration();
  void SimulateMovement(const FMove& Move);

  void MoveForward(const float AxisValue);
  void MoveRight(const float AxisValue);

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
  TArray<bool> KeyMap;
  bool GetKeyPressed(const EKey Key);
  void SetKeyPressed(const EKey Key, const bool bValue);
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
  TArray<bool> MouseMap;
  bool GetMousePressed(const EMouse MouseKey);
  void SetMousePressed(const EMouse MouseKey, const bool bIsPressed);
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
  void Look();
  void Walk();
  void Jump();
  void DoAttack(const float DeltaTime);
  void TakeConsecutiveDamage(const float DeltaTime);
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

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
