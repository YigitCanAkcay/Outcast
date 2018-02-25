#pragma once

#include "CoreMinimal.h"

#include "OutcastAnimInstance.generated.h"


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class OUTCAST_API UOutcastAnimInstance : public UAnimInstance
{
  GENERATED_UCLASS_BODY()

  //******** BASIC MOVEMENT ********
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsJumping;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsRunning;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    float Speed;  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    float PlayRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    FRotator TorsoRotation;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    FRotator LegsRotation;

  TArray<FRotator> LegsRotationBuffer;
  int CurrentLegsRotationBufferIndex;
  //******** BASIC MOVEMENT ********

  //******** ATTACKING ********
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsSwordAttacking;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsSlashingLeft;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsSlashingRight;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BasicMovement)
    bool bIsSlashingForward;
  //******** ATTACKING ********

public:
  void SetIsRunning(const bool bNewIsRunning);
  bool GetIsRunning() const;

  void SetAcceleration(const float NewSpeed);

  void SetIsJumping(const bool bNewIsJumping);
  bool GetIsJumping() const;

  void SetIsSwordAttacking(const bool bNewIsSwordAttacking);
  bool GetIsSwordAttacking() const;

  void SetIsSlashingLeft(const bool bNewIsSlashingLeft);
  bool GetIsSlashingLeft() const;

  void SetIsSlashingRight(const bool bNewIsSlashingRight);
  bool GetIsSlashingRight() const;

  void SetIsSlashingForward(const bool bNewIsSlashingForward);
  bool GetIsSlashingForward() const;

  void SetWalkPlayrate(const float NewPlayRate);

  void SetTorsoRotation(const FRotator& NewTorsoRotation);
  FRotator GetTorsoRotation() const;

  void SetLegsRotation(const FRotator& NewLegsRotation);
  FRotator GetLegsRotation() const;
};
