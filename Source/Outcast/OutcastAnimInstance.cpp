#include "OutcastAnimInstance.h"

UOutcastAnimInstance::UOutcastAnimInstance(const FObjectInitializer& ObjectInitializer)
  : 
  Super(ObjectInitializer),
  bIsJumping(false),
  bIsRunning(false),
  bIsSlashingLeft(false),
  Speed(0.0f),
  PlayRate(1.0f),
  TorsoRotation(FRotator()),
  LegsRotation(FRotator()),
  CurrentLegsRotationBufferIndex(0)
{
}

void UOutcastAnimInstance::SetIsRunning(const bool bNewIsRunning)
{
  bIsRunning = bNewIsRunning;
}
bool UOutcastAnimInstance::GetIsRunning() const
{
  return bIsRunning;
}

void UOutcastAnimInstance::SetAcceleration(const float NewSpeed)
{
  Speed = NewSpeed;
}

void UOutcastAnimInstance::SetIsJumping(const bool bNewIsJumping)
{
  bIsJumping = bNewIsJumping;
}
bool UOutcastAnimInstance::GetIsJumping() const
{
  return bIsJumping;
}

void UOutcastAnimInstance::SetIsSlashingLeft(const bool bNewIsSlashingLeft)
{
  bIsSlashingLeft = bNewIsSlashingLeft;
}
bool UOutcastAnimInstance::GetIsSlashingLeft() const
{
  return bIsSlashingLeft;
}

void UOutcastAnimInstance::SetWalkPlayrate(const float NewPlayRate)
{
  PlayRate = NewPlayRate;
}

void UOutcastAnimInstance::SetTorsoRotation(const FRotator& NewTorsoRotation)
{
  TorsoRotation = NewTorsoRotation;
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