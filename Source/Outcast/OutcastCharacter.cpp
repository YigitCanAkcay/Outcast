#include "OutcastCharacter.h"
#include "OutcastGameMode.h"

#define COL_Player ECC_GameTraceChannel1
#define COL_Weapon ECC_GameTraceChannel2

AOutcastCharacter::AOutcastCharacter()
  :
  DefaultMeshLocationOffset(FVector(0.0f, 0.0f, -120.0f)),
  DefaultMeshRotationOffset(FRotator(0.0f, -90.0f, 0.0f)),
  ServerState(),
  bReconcilingWithServer(false),
  MaxInterpolationDistance(1000.0f),
  MeshTranslationOffset(FVector::ZeroVector),
  MaxInterpolationDeltaRotation(360.0f),
  MeshRotationOffset(FRotator::ZeroRotator),
  ForwardDirection(0.0f),
  SidewardDirection(0.0f),
  Acceleration(0.0f),
  AccelerationCoefficient(5.0f),
  MaxWalkSpeed(10),

  Jumping(EJump::Downwards),
  JumpHeight(0.0f),
  JumpStartLocZ(0.0f),
  BunnyHopSpeedRatio(DefaultJumpSpeedRatio),
  CurrentAttack(EAttack::NONE),
  Health(100),
  MouseInput(FVector2D::ZeroVector)
{
 	PrimaryActorTick.bCanEverTick = true;

  // Since TMaps can't be replicated a TArray will be used instead
  // pre initialized with 2 elements so that EMouse can be used to index
  MouseMap.Add(false);
  MouseMap.Add(false);

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Warrior.SK_Cat_Warrior'"));
  if (BodyMesh.Succeeded())
  {
    Body = GetMesh();
    Body->SetSkeletalMesh(BodyMesh.Object);

    static ConstructorHelpers::FObjectFinder<UClass> AnimBP(TEXT("Class'/Game/Feline_Warrior/Animations/Character_Animation_BP.Character_Animation_BP_C'"));
    if (AnimBP.Succeeded())
    {
      Body->SetAnimInstanceClass(AnimBP.Object);
    }

    Body->SetRelativeLocation(DefaultMeshLocationOffset);
    Body->SetRelativeRotation(DefaultMeshRotationOffset);
  }

   ConstructorHelpers::FObjectFinder<UStaticMesh> WeaponMesh(TEXT("StaticMesh'/Game/Meshes/Weapon_StaticMesh.Weapon_StaticMesh'"));
  if (WeaponMesh.Succeeded())
  {
    Weapon = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), FName(TEXT("Sword")));
    Weapon->SetStaticMesh(WeaponMesh.Object);

    Weapon->SetCollisionProfileName(TEXT("Weapon"));
  }

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> EyeballMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Eyeball.SK_Cat_Eyeball'"));
  if (EyeballMesh.Succeeded())
  {
    Eye_R = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Eye_R")));
    Eye_R->SetSkeletalMesh(EyeballMesh.Object);

    Eye_L = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Eye_L")));
    Eye_L->SetSkeletalMesh(EyeballMesh.Object);
  }

  static ConstructorHelpers::FObjectFinder<USkeletalMesh> ArmorMesh(TEXT("SkeletalMesh'/Game/Feline_Warrior/Meshes/SK_Cat_Chest.SK_Cat_Chest'"));
  if (ArmorMesh.Succeeded())
  {
    Armor = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(TEXT("Armor")));
    Armor->SetSkeletalMesh(ArmorMesh.Object);
  }

  Capsule = Cast<UCapsuleComponent>(RootComponent);
  Capsule->SetCapsuleHalfHeight(120.0f);
  Capsule->SetCapsuleRadius(60.0f);
  Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, 122.0f));
  Capsule->SetCollisionProfileName(TEXT("Player"));
  Capsule->OnComponentHit.__Internal_AddDynamic(this, &AOutcastCharacter::OnHit, FName("OnHit"));
  Capsule->OnComponentBeginOverlap.AddDynamic(this, &AOutcastCharacter::BodyOverlapBegin);
  Capsule->OnComponentEndOverlap.AddDynamic(this, &AOutcastCharacter::BodyOverlapEnd);
  
  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  CameraRotator = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraRotator"));
  CameraRotator->SetupAttachment(RootComponent);
  Camera->SetupAttachment(CameraRotator);
  Camera->SetRelativeLocation(FVector(-423.0f, 0.0f, 200.0f));
  Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

  // UNUSED
  Movement               = Cast<UCharacterMovementComponent>(GetMovementComponent());
  Movement->AirControl   = 1.0f;
  Movement->MaxWalkSpeed = 1000.0f;
  Movement->MaxFlySpeed  = 10000.0f;
  Movement->GravityScale = 5.0f;
  // UNUSED

  bUseControllerRotationYaw   = false;
  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll  = false;

  if (HasAuthority())
  {
    bReplicates        = true;
    bReplicateMovement = false;
  }
  
  // Sounds
  static ConstructorHelpers::FObjectFinder<USoundAttenuation> Atten(TEXT("SoundAttenuation'/Game/Audio/Distance.Distance'"));
  if (Atten.Succeeded())
  {
    Attenuation = Atten.Object;
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> Step1(TEXT("SoundWave'/Game/Audio/Walk/Step1.Step1'"));
  if (Step1.Succeeded())
  {
    StepSounds.Add(Step1.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step2(TEXT("SoundWave'/Game/Audio/Walk/Step2.Step2'"));
  if (Step2.Succeeded())
  {
    StepSounds.Add(Step2.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step3(TEXT("SoundWave'/Game/Audio/Walk/Step3.Step3'"));
  if (Step3.Succeeded())
  {
    StepSounds.Add(Step3.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step4(TEXT("SoundWave'/Game/Audio/Walk/Step4.Step4'"));
  if (Step4.Succeeded())
  {
    StepSounds.Add(Step4.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step5(TEXT("SoundWave'/Game/Audio/Walk/Step5.Step5'"));
  if (Step5.Succeeded())
  {
    StepSounds.Add(Step5.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step6(TEXT("SoundWave'/Game/Audio/Walk/Step6.Step6'"));
  if (Step6.Succeeded())
  {
    StepSounds.Add(Step6.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step7(TEXT("SoundWave'/Game/Audio/Walk/Step7.Step7'"));
  if (Step7.Succeeded())
  {
    StepSounds.Add(Step7.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> Step8(TEXT("SoundWave'/Game/Audio/Walk/Step8.Step8'"));
  if (Step8.Succeeded())
  {
    StepSounds.Add(Step8.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal1(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump1.Jump1'"));
  if (JumpVocal1.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal1.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal2(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump2.Jump2'"));
  if (JumpVocal2.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal2.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal3(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump3.Jump3'"));
  if (JumpVocal3.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal3.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal4(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump4.Jump4'"));
  if (JumpVocal4.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal4.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal5(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump5.Jump5'"));
  if (JumpVocal5.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal5.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal6(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump6.Jump6'"));
  if (JumpVocal6.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal6.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal7(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump7.Jump7'"));
  if (JumpVocal7.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal7.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal8(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump8.Jump8'"));
  if (JumpVocal8.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal8.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> JumpVocal9(TEXT("SoundWave'/Game/Audio/Jump/Vocals/Jump9.Jump9'"));
  if (JumpVocal9.Succeeded())
  {
    JumpVocalSounds.Add(JumpVocal9.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> AttackVocal1(TEXT("SoundWave'/Game/Audio/Attacking/Vocals/Attack1.Attack1'"));
  if (AttackVocal1.Succeeded())
  {
    AttackVocalSounds.Add(AttackVocal1.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> AttackVocal2(TEXT("SoundWave'/Game/Audio/Attacking/Vocals/Attack2.Attack2'"));
  if (AttackVocal2.Succeeded())
  {
    AttackVocalSounds.Add(AttackVocal2.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> AttackVocal3(TEXT("SoundWave'/Game/Audio/Attacking/Vocals/Attack3.Attack3'"));
  if (AttackVocal3.Succeeded())
  {
    AttackVocalSounds.Add(AttackVocal3.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> AttackVocal4(TEXT("SoundWave'/Game/Audio/Attacking/Vocals/Attack4.Attack4'"));
  if (AttackVocal4.Succeeded())
  {
    AttackVocalSounds.Add(AttackVocal4.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> Attack1(TEXT("SoundWave'/Game/Audio/Attacking/Attack1.Attack1'"));
  if (Attack1.Succeeded())
  {
    AttackSounds.Add(Attack1.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> Hit(TEXT("SoundWave'/Game/Audio/Hit/Hit.Hit'"));
  if (Hit.Succeeded())
  {
    HitSounds.Add(Hit.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> HitVocal1(TEXT("SoundWave'/Game/Audio/Hit/Vocals/Hit1.Hit1'"));
  if (HitVocal1.Succeeded())
  {
    HitVocalSounds.Add(HitVocal1.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> HitVocal2(TEXT("SoundWave'/Game/Audio/Hit/Vocals/Hit2.Hit2'"));
  if (HitVocal2.Succeeded())
  {
    HitVocalSounds.Add(HitVocal2.Object);
  }
  static ConstructorHelpers::FObjectFinder<USoundWave> HitVocal3(TEXT("SoundWave'/Game/Audio/Hit/Vocals/Hit3.Hit3'"));
  if (HitVocal3.Succeeded())
  {
    HitVocalSounds.Add(HitVocal3.Object);
  }

  static ConstructorHelpers::FObjectFinder<USoundWave> IdleHit(TEXT("SoundWave'/Game/Audio/Hit/IdleHit.IdleHit'"));
  if (IdleHit.Succeeded())
  {
    IdleHitSound = IdleHit.Object;
  }
}

void AOutcastCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AOutcastCharacter, ServerState);
}

void AOutcastCharacter::BeginPlay()
{
  Super::BeginPlay();

  Anim = Cast<UOutcastAnimInstance>(Body->GetAnimInstance());
  if (!Anim)
  {
    Destroy();
  }
  Anim->SetMyActor(this);

  if (Body && Eye_L && Eye_R && Weapon && Armor)
  {
    const FName SwordSocket = TEXT("Sword");
    Weapon->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      SwordSocket);

    const FName EyeSocket_R = TEXT("Eye_R");
    const FName EyeSocket_L = TEXT("Eye_L");
    Eye_R->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      EyeSocket_R);
    Eye_L->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      EyeSocket_L);

    const FName ArmorSocket = TEXT("Chest");
    Armor->AttachToComponent(
      Body,
      FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
      ArmorSocket);
  }
}

void AOutcastCharacter::ResetToServerState(const FState& State)
{
  SetActorLocation(State.Location);
  SetActorRotation(ReturnFRotator(State.Rotation));
  Health = State.Health;

  if (Anim)
  {
    Anim->SetAccelerationAndLegRotation(State.Move.Acceleration, State.Move.ForwardDirection, State.Move.SidewardDirection);
    Anim->SetTorsoRotation(ReturnFRotator(State.TorsoRotation));

    // Uncommenting this interrupts client side attack animation when lagging
    /*Anim->SetIsSlashingLeft(   State.CurrentAttack == EAttack::Left);
    Anim->SetIsSlashingRight(  State.CurrentAttack == EAttack::Right);
    Anim->SetIsSlashingForward(State.CurrentAttack == EAttack::Forward);*/
  }
}

void AOutcastCharacter::OnRep_ServerState()
{
  bReconcilingWithServer = true;
  if (IsLocallyControlled())
  {
    ResetToServerState(ServerState);
    CleanUnacknowledgedMoves();
    ReconcileWithServer();
  }
  else if (Role == ROLE_SimulatedProxy)
  {
    FVector NewToOldLocation = GetActorLocation() - ServerState.Location;
    float Distance = NewToOldLocation.Size();
    if (Distance > MaxInterpolationDistance)
    {
      MeshTranslationOffset = FVector::ZeroVector;
    }
    else
    {
      MeshTranslationOffset = MeshTranslationOffset + NewToOldLocation;
    }

    FVector NewToOldRotation = ReturnFVector(GetActorRotation() - ReturnFRotator(ServerState.Rotation));
    float DeltaRot = NewToOldRotation.Size();
    if (DeltaRot > MaxInterpolationDeltaRotation)
    {
      MeshRotationOffset = FRotator::ZeroRotator;
    }
    else
    {
      MeshRotationOffset = MeshRotationOffset + ReturnFRotator(NewToOldRotation);
    }

    ResetToServerState(ServerState);
    SimulateAttacks(ServerState.Move);
  }
  bReconcilingWithServer = false;
}

FState AOutcastCharacter::CreateState(const FMove& Move)
{
  FState NewState;

  NewState.Location      = GetActorLocation();
  NewState.Rotation      = ReturnFVector(GetActorRotation());
  NewState.TorsoRotation = ReturnFVector(Anim->GetTorsoRotation());
  NewState.CurrentAttack = CurrentAttack;
  NewState.Health        = Health;
  NewState.Move          = Move;

  return NewState;
}

FMove AOutcastCharacter::CreateMove(const float DeltaTime)
{
  FMove NewMove;

  NewMove.DeltaTime         = DeltaTime;
  NewMove.TimeStamp         = GetWorld()->TimeSeconds;
  NewMove.ForwardDirection  = ForwardDirection;
  NewMove.SidewardDirection = SidewardDirection;
  NewMove.Acceleration      = Acceleration;
  NewMove.MouseInput        = MouseInput;
  NewMove.MouseMap          = MouseMap;

  if (!HasAuthority())
  {
    UnacknowledgedMoves.Add(NewMove);
  }

  return NewMove;
}

void AOutcastCharacter::Server_SendMove_Implementation(const FMove Move)
{
  Simulate(Move);

  ServerState = CreateState(Move);

  //Multicast_SendMove(Move);
}

bool AOutcastCharacter::Server_SendMove_Validate(const FMove Move)
{
  return true;
}

void AOutcastCharacter::Multicast_SendMove_Implementation(const FMove Move)
{
  if (Role == ROLE_SimulatedProxy)
  {
    Simulate(Move);
  }
}

bool AOutcastCharacter::Multicast_SendMove_Validate(const FMove Move)
{
  return true;
}

void AOutcastCharacter::CleanUnacknowledgedMoves()
{
  TArray<FMove> NewList;

  for (const FMove& Move : UnacknowledgedMoves)
  {
    if (Move.TimeStamp > ServerState.Move.TimeStamp)
    {
      NewList.Add(Move);
    }
  }

  UnacknowledgedMoves = NewList;
}

void AOutcastCharacter::ReconcileWithServer()
{
  for (const FMove& Move : UnacknowledgedMoves)
  {
    SimulateMovement(Move);
    SimulateLookAround(Move);
  }
}

void AOutcastCharacter::Simulate(const FMove& Move)
{
  SimulateLookAround(Move);
  SimulateMovement(Move); 
  SimulateAttacks(Move);
  SimulateConsecutiveDamage(Move.DeltaTime);
}

void AOutcastCharacter::SimulateLookAround(const FMove& Move)
{
  // Rotate whole Character
  FRotator CharacterRotation = GetActorRotation();
  CharacterRotation.Yaw      = CharacterRotation.Yaw + Move.MouseInput.X * Move.DeltaTime;
  SetActorRotation(CharacterRotation);

  if (Anim)
  {
    Anim->CalculateTorsoRotation(Move.MouseInput.Y * Move.DeltaTime);

    if (IsLocallyControlled())
    {
      // Rotate the camera
      FRotator NewCameraRot = CameraRotator->GetComponentRotation();
      NewCameraRot.Pitch    = FMath::Clamp((Anim->GetTorsoRotation().Roll + Move.MouseInput.Y * Move.DeltaTime) * -1, -80.0f, 80.0f);

      CameraRotator->SetWorldRotation(NewCameraRot);
    }
  }
}

void AOutcastCharacter::SimulateMovement(const FMove& Move)
{
  Capsule->AddWorldOffset(GetActorForwardVector() * Move.ForwardDirection * Move.Acceleration * MaxWalkSpeed * Move.DeltaTime, true);
  Capsule->AddWorldOffset(GetActorRightVector() * Move.SidewardDirection * Move.Acceleration * MaxWalkSpeed * Move.DeltaTime, true);

  if (Anim)
  {
    Anim->SetAccelerationAndLegRotation(Move.Acceleration, Move.ForwardDirection, Move.SidewardDirection);
    Anim->SetWalkPlayrate(Move.ForwardDirection);
  }

  // Gravity
  Capsule->AddWorldOffset(GetActorUpVector() * -1 * 990 * Move.DeltaTime, true);
}

void AOutcastCharacter::SimulateAttacks(const FMove& Move)
{
  if (CurrentAttack == EAttack::Left)
  {
    CurrentAttack = Anim->GetIsSlashingLeft() ? EAttack::Left : EAttack::NONE;
  }
  else if (CurrentAttack == EAttack::Right)
  {
    CurrentAttack = Anim->GetIsSlashingRight() ? EAttack::Right : EAttack::NONE;
  }
  else if (CurrentAttack == EAttack::Forward)
  {
    CurrentAttack = Anim->GetIsSlashingForward() ? EAttack::Forward : EAttack::NONE;
  }

  if (Move.MouseMap[static_cast<int>(EMouse::Left)])
  {
    if (CurrentAttack == EAttack::NONE)
    {
      // Slashing left/right has priority over forward
      if (Move.SidewardDirection == -1.0f)
      {
        CurrentAttack = EAttack::Left;
      }
      else if (Move.SidewardDirection == 1.0f)
      {
        CurrentAttack = EAttack::Right;
      }
      else if (Move.SidewardDirection == 0.0f && Move.ForwardDirection != 0.0f)
      {
        CurrentAttack = EAttack::Forward;
      }
      else
      {
        int Random = FMath::RandRange(1, 3);
        CurrentAttack = static_cast<EAttack>(Random);
      }
    }
  }

  if (Anim)
  {
    Anim->SetIsSlashingLeft(   CurrentAttack == EAttack::Left);
    Anim->SetIsSlashingRight(  CurrentAttack == EAttack::Right);
    Anim->SetIsSlashingForward(CurrentAttack == EAttack::Forward);

    // Specify the blend weight between sword attacks/basic movement
    if (CurrentAttack != EAttack::NONE)
    {
      Anim->AddAttackMovementBlendWeight(0.1f);
    }
    else
    {
      Anim->SubtractAttackMovementBlendWeight(0.05f);
    }
  }
}

void AOutcastCharacter::SimulateConsecutiveDamage(const float DeltaTime)
{
  if (HasAuthority())
  {
    if (Health == 0)
    {
      Cast<AOutcastGameMode>(GetWorld()->GetAuthGameMode())->Respawn(MyPlayerController);
    }
  }

  for (auto AttackerIt = DamageTakenBy.CreateIterator(); AttackerIt; ++AttackerIt)
  {
    AttackerIt->Value = AttackerIt->Value + DeltaTime;

    if (AttackerIt->Value >= 1.0f && !bReconcilingWithServer)
    {
      AttackerIt->Value = 0.0f;

      if (AttackerIt->Key->GetAttack() == EAttack::NONE)
      {
        Health = FMath::Clamp(Health - 1, 0, 100);
        PlayIdleHitSound();
      }
      else
      {
        Health = FMath::Clamp(Health - 20, 0, 100);
        PlayHitSound();
      }
      PlayHitVocalSound();
    }
  }
}

FVector AOutcastCharacter::ReturnFVector(const FRotator& Rotator)
{
  FVector Vector;
  Vector.X = Rotator.Pitch;
  Vector.Y = Rotator.Roll;
  Vector.Z = Rotator.Yaw;
  return Vector;
}
FRotator AOutcastCharacter::ReturnFRotator(const FVector& Vector)
{
  FRotator Rotator;
  Rotator.Pitch = Vector.X;
  Rotator.Roll  = Vector.Y;
  Rotator.Yaw   = Vector.Z;
  return Rotator;
}

int AOutcastCharacter::GetHealth()
{
  return Health;
}

void AOutcastCharacter::PlayStepSound()
{
  if (StepSounds.Num() > 0)
  {
    int Random = FMath::RandRange(0, StepSounds.Num() - 1);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), StepSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayJumpVocalSound()
{
  if (JumpVocalSounds.Num() > 0)
  {
    int Random = FMath::RandRange(0, JumpVocalSounds.Num() - 1);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayAttackVocalSound()
{
  if (AttackVocalSounds.Num() > 0)
  {
    int Random = FMath::RandRange(0, AttackVocalSounds.Num() - 1);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayAttackSound()
{
  if (AttackSounds.Num() > 0)
  {
    int Random = 0;//FMath::RandRange(0, 0);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSounds[Random], GetActorLocation(), 0.35f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayHitSound()
{
  if (HitSounds.Num() > 0)
  {
    int Random = 0;// FMath::RandRange(0, 3);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayHitVocalSound()
{
  if (HitVocalSounds.Num() > 0)
  {
    int Random = FMath::RandRange(0, HitVocalSounds.Num() - 1);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::PlayIdleHitSound()
{
  if (IdleHitSound)
  {
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), IdleHitSound, GetActorLocation(), 0.5f, 1.0f, 0.0f, Attenuation);
  }
}

void AOutcastCharacter::BodyOverlapBegin(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult)
{ 
  if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && !bReconcilingWithServer)
  {
    AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

    if (!DamageTakenBy.Contains(AttackingCharacter))
    {
      DamageTakenBy.Add(AttackingCharacter, 0.0f);

      if (AttackingCharacter->GetAttack() == EAttack::NONE)
      {
        Health = FMath::Clamp(Health - 1, 0, 100);
        //PlayIdleHitSound();
      }
      else
      {
        Health = FMath::Clamp(Health - 20, 0, 100);
        //PlayHitSound();
      }
      PlayHitVocalSound();
    }
  }
}

void AOutcastCharacter::BodyOverlapEnd(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex)
{
  if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && !bReconcilingWithServer)
  {
    AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

    if (DamageTakenBy.Contains(AttackingCharacter))
    {
      DamageTakenBy.Remove(AttackingCharacter);
    }
  }
}

void AOutcastCharacter::Jump()
{
  /*if (GetKeyPressed(EKey::Space))
  {
    if (Jumping != EJump::BunnyHop)
    {
      if (JumpHeight < JumpHeightLimit
        && Jumping != EJump::Downwards)
      {
        if (Jumping == EJump::NONE)
        {
          JumpStartLocZ = GetActorLocation().Z;
        }
        Jumping = EJump::Upwards;
      }
      else
      {
        Jumping = EJump::Downwards;
      }
    }
  }
  else if ((Jumping == EJump::Upwards || Jumping == EJump::BunnyHop) && JumpHeight >= MinJumpHeight)
  {
    Jumping = EJump::Downwards;
  }

  Anim->SetIsJumping(Jumping != EJump::NONE);

  if (Jumping == EJump::Upwards)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;

    Movement->SetMovementMode(MOVE_Flying);
    AddMovementInput(GetActorUpVector(), 5.0f);
  }
  else if (Jumping == EJump::Downwards)
  {
    Movement->SetMovementMode(MOVE_Falling);
  }
  else if (Jumping == EJump::NONE)
  {
    JumpHeight = 0.0f;
    Movement->SetMovementMode(MOVE_Walking);
  }
  else if (Jumping == EJump::BunnyHop)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;

    AddMovementInput(GetActorUpVector(), 5.0f);
    Movement->SetMovementMode(MOVE_Flying);
    if (JumpHeight > BunnyHopMaxHeight)
    {
      Jumping = EJump::Upwards;
      BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
    }
  }
  */
}

void AOutcastCharacter::RegulateAcceleration()
{
  if ( ForwardDirection != 0.0f
    || SidewardDirection != 0.0f)
  {
    Acceleration = FMath::Clamp(Acceleration + AccelerationCoefficient, 0, 100);
  }
  else
  {
    Acceleration = FMath::Clamp(Acceleration - AccelerationCoefficient, 0, 100);
  }
}

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (IsLocallyControlled())
  {
    // Regulate it before (or after) creating the move and simulating
    // it, but not in between.
    RegulateAcceleration();

    FMove Move = CreateMove(DeltaTime);

    Simulate(Move);

    if (HasAuthority())
    {
      ServerState = CreateState(Move);
    }
    else
    {
      Server_SendMove(Move);

      //UE_LOG(LogTemp, Warning, TEXT("%d"), UnacknowledgedMoves.Num());
    }
  }
  else
  {
    MeshTranslationOffset = MeshTranslationOffset * (1.0f - DeltaTime / 0.125);
    FVector NewRelativeTranslation = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), MeshTranslationOffset) + DefaultMeshLocationOffset;
    
    Body->SetRelativeLocation(NewRelativeTranslation);

    MeshRotationOffset = MeshRotationOffset * (1.0f - DeltaTime / 0.125);
    FRotator NewRelativeRotation = MeshRotationOffset + DefaultMeshRotationOffset;

    Body->SetRelativeRotation(NewRelativeRotation);
  }
}

void AOutcastCharacter::SetMyPlayerController(APlayerController* const NewPlayerController)
{
  MyPlayerController = NewPlayerController;
}

EAttack AOutcastCharacter::GetAttack()
{
  return CurrentAttack;
}

void AOutcastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Keyboard input
  PlayerInputComponent->BindAxis(
    "MoveForward",
    this,
    &AOutcastCharacter::MoveForward);
  PlayerInputComponent->BindAxis(
    "MoveRight",
    this,
    &AOutcastCharacter::MoveRight);

  // Mouse input
  PlayerInputComponent->BindAction(
    "MouseLeft",
    IE_Pressed,
    this,
    &AOutcastCharacter::MouseLeftPressed);
  PlayerInputComponent->BindAction(
    "MouseRight",
    IE_Pressed,
    this,
    &AOutcastCharacter::MouseRightPressed);

  PlayerInputComponent->BindAction(
    "MouseLeft",
    IE_Released,
    this,
    &AOutcastCharacter::MouseLeftReleased);
  PlayerInputComponent->BindAction(
    "MouseRight",
    IE_Released,
    this,
    &AOutcastCharacter::MouseRightReleased);

  PlayerInputComponent->BindAxis(
    "MouseUpDown",
    this,
    &AOutcastCharacter::MouseUpDown);
  PlayerInputComponent->BindAxis(
    "MouseRightLeft",
    this,
    &AOutcastCharacter::MouseRightLeft);
}

void AOutcastCharacter::MoveForward(const float AxisValue)
{
  ForwardDirection = AxisValue;
}

void AOutcastCharacter::MoveRight(const float AxisValue)
{
  SidewardDirection = AxisValue;
}

bool AOutcastCharacter::GetMousePressed(const EMouse MouseKey)
{
  return MouseMap[static_cast<int>(MouseKey)];
}

void AOutcastCharacter::SetMousePressed(const EMouse MouseKey, const bool bIsPressed)
{
  MouseMap[static_cast<int>(MouseKey)] = bIsPressed;
}

void AOutcastCharacter::MouseLeftPressed()
{
  SetMousePressed(EMouse::Left, true);
}

void AOutcastCharacter::MouseRightPressed()
{
  SetMousePressed(EMouse::Right, true);
}

void AOutcastCharacter::MouseLeftReleased()
{
  SetMousePressed(EMouse::Left, false);
}

void AOutcastCharacter::MouseRightReleased()
{
  SetMousePressed(EMouse::Right, false);
}

void AOutcastCharacter::MouseUpDown(const float AxisValue)
{
  MouseInput.Y = AxisValue;
}

void AOutcastCharacter::MouseRightLeft(const float AxisValue)
{
  MouseInput.X = AxisValue;
}

void AOutcastCharacter::OnHit(
  UPrimitiveComponent* HitComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  FVector NormalImpulse,
  const FHitResult& Hit)
{
  /*
  // Character is on top of a walkable plane
  if (OtherActor->ActorHasTag(FName("Walkable")))
  {
    if (  GetKeyPressed(EKey::Space)
      && (GetKeyPressed(EKey::A) || GetKeyPressed(EKey::D))
      && (Jumping == EJump::Downwards || Jumping == EJump::BunnyHop))
    {
      Jumping = EJump::BunnyHop;
      JumpHeight = 0.0f;
      BunnyHopSpeedRatio = FMath::Clamp(BunnyHopSpeedRatio / 3.0f, BunnyHopFastestSpeedRatio, DefaultJumpSpeedRatio);

      PlayJumpVocalSound();
    }
    else
    {
      Jumping = EJump::NONE;
      BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
    }

    PlayStepSound();
    Anim->SetIsJumping(false);
  }*/
}