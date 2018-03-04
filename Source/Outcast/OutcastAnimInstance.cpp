#include "OutcastAnimInstance.h"

UOutcastAnimInstance::UOutcastAnimInstance(const FObjectInitializer& ObjectInitializer)
  : 
  Super(ObjectInitializer),
  bIsJumping(false),
  bIsRunning(false),
  AttackMovementBlendWeight(0.0f),
  bIsSlashingLeft(false),
  bIsSlashingRight(false),
  bIsSlashingForward(false),
  Speed(0.0f),
  PlayRate(1.0f),
  TorsoRotation(FRotator()),
  LegsRotation(FRotator()),
  CurrentLegsRotationBufferIndex(0)
{
}

void UOutcastAnimInstance::SetMyActor(AActor* const NewActor)
{
  MyActor = NewActor;
}

AActor* UOutcastAnimInstance::GetMyActor()
{
  return MyActor;
}

void UOutcastAnimInstance::SetIsRunning(const bool bNewIsRunning)
{
  bIsRunning = bNewIsRunning;
}

bool UOutcastAnimInstance::GetIsRunning() const
{
  return bIsRunning;
}

void UOutcastAnimInstance::SetAccelerationAndLegRotation(const float NewSpeed, const float ForwardDirection, const float SidewardDirection)
{
  Speed = NewSpeed;

  FRotator NewLegsRotation;
  NewLegsRotation = FRotator::ZeroRotator;

  if (ForwardDirection == 1.0f)
  {
    if (SidewardDirection == 1.0f)
    {
      NewLegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
    else if (SidewardDirection == -1.0f)
    {
      NewLegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
  }
  else if (ForwardDirection == -1.0f)
  {
    if (SidewardDirection == 1.0f)
    {
      NewLegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
    else if (SidewardDirection == -1.0f)
    {
      NewLegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
  }
  else if (SidewardDirection == 1.0f)
  {
    NewLegsRotation = FRotator(0.0f, 90.0f, 0.0f);
  }
  else if (SidewardDirection == -1.0f)
  {
    NewLegsRotation = FRotator(0.0f, -90.0f, 0.0f);
  }

  SetLegsRotation(NewLegsRotation);
}

void UOutcastAnimInstance::SetIsJumping(const bool bNewIsJumping)
{
  bIsJumping = bNewIsJumping;
}

bool UOutcastAnimInstance::GetIsJumping() const
{
  return bIsJumping;
}

void UOutcastAnimInstance::AddAttackMovementBlendWeight(const float DeltaAttackMovementBlendWeight)
{
  AttackMovementBlendWeight = FMath::Clamp(AttackMovementBlendWeight + DeltaAttackMovementBlendWeight, 0.0f, 1.0f);
}

void UOutcastAnimInstance::SubtractAttackMovementBlendWeight(const float DeltaAttackMovementBlendWeight)
{
  AttackMovementBlendWeight = FMath::Clamp(AttackMovementBlendWeight - DeltaAttackMovementBlendWeight, 0.0f, 1.0f);
}

void UOutcastAnimInstance::SetAttackMovementBlendWeight(const float NewAttackMovementBlendWeight)
{
  AttackMovementBlendWeight = NewAttackMovementBlendWeight;
}

float UOutcastAnimInstance::GetAttackMovementBlendWeight() const
{
  return AttackMovementBlendWeight;
}

void UOutcastAnimInstance::SetIsSlashingLeft(const bool bNewIsSlashingLeft)
{
  bIsSlashingLeft = bNewIsSlashingLeft;
}

bool UOutcastAnimInstance::GetIsSlashingLeft() const
{
  return bIsSlashingLeft;
}

void UOutcastAnimInstance::SetIsSlashingRight(const bool bNewIsSlashingRight)
{
  bIsSlashingRight = bNewIsSlashingRight;
}

bool UOutcastAnimInstance::GetIsSlashingRight() const
{
  return bIsSlashingRight;
}

void UOutcastAnimInstance::SetIsSlashingForward(const bool bNewIsSlashingForward)
{
  bIsSlashingForward = bNewIsSlashingForward;
}

bool UOutcastAnimInstance::GetIsSlashingForward() const
{
  return bIsSlashingForward;
}

void UOutcastAnimInstance::SetWalkPlayrate(const float NewPlayRate)
{
  if (NewPlayRate == 0.0f)
  {
    PlayRate = 1.0f;
  }
  else
  {
    PlayRate = NewPlayRate;
  }
}

void UOutcastAnimInstance::SetTorsoRotation(const float MouseInputY)
{
  TorsoRotation.Roll = FMath::Clamp(TorsoRotation.Roll - MouseInputY, -80.0f, 80.0f);
}


FRotator UOutcastAnimInstance::GetTorsoRotation() const
{
  return TorsoRotation;
}

void UOutcastAnimInstance::SetLegsRotation(const FRotator& NewLegsRotation)
{
  if (CurrentLegsRotationBufferIndex == 10)
  {
    CurrentLegsRotationBufferIndex = 0;
  }

  if (LegsRotationBuffer.Num() - 1 >= CurrentLegsRotationBufferIndex)
  {
    LegsRotationBuffer[CurrentLegsRotationBufferIndex] = NewLegsRotation;
  }
  else
  {
    LegsRotationBuffer.Add(NewLegsRotation);
  }

  ++CurrentLegsRotationBufferIndex;

  LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
  for (FRotator Rotation : LegsRotationBuffer)
  {
    LegsRotation = LegsRotation + Rotation;
  }

  const float LegsRotationBufferNum = static_cast<float>(LegsRotationBuffer.Num());
  LegsRotation = FRotator(LegsRotation.Pitch / LegsRotationBufferNum, LegsRotation.Yaw / LegsRotationBufferNum, LegsRotation.Roll / LegsRotationBufferNum);
}

FRotator UOutcastAnimInstance::GetLegsRotation() const
{
  return LegsRotation;
}