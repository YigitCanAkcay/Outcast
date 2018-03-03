#include "OutcastCharacter.h"
#include "OutcastGameMode.h"

#define COL_Player ECC_GameTraceChannel1
#define COL_Weapon ECC_GameTraceChannel2

AOutcastCharacter::AOutcastCharacter()
  :
  Speed(0.0f),
  Direction(FVector()),
  WalkPlayrate(1.0f),
  LegsRotation(FRotator()),
  TorsoRotation(FRotator()),
  CharacterRotation(FRotator()),
  CharacterLocation(FVector()),
  Jumping(EJump::Downwards),
  JumpHeight(0.0f),
  JumpStartLocZ(0.0f),
  BunnyHopSpeedRatio(DefaultJumpSpeedRatio),
  Attacking(EAttack::NONE),
  Health(100),
  MouseInput(FVector2D::ZeroVector)
{
 	PrimaryActorTick.bCanEverTick = true;

  // Initialize KeyMap
  // Since TMaps can't be replicated a TArray will be used instead
  // pre initialized with 5 elements so that EKey can be used to index
  /*KeyMap.Add(EKey::W, false);
  KeyMap.Add(EKey::A, false);
  KeyMap.Add(EKey::S, false);
  KeyMap.Add(EKey::D, false);
  KeyMap.Add(EKey::Space, false);*/
  KeyMap.Add(false);
  KeyMap.Add(false);
  KeyMap.Add(false);
  KeyMap.Add(false);
  KeyMap.Add(false);

  // Initialize Mouse Input
  // See comment above on KeyMap
  /*MouseMap.Add(EMouse::Left, false);
  MouseMap.Add(EMouse::Right, false);*/
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

    Body->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    Body->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
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
  Camera->SetupAttachment(RootComponent);
  Camera->SetRelativeLocation(FVector(-423.0f, 0.0f, 200.0f));
  Camera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

  Movement               = Cast<UCharacterMovementComponent>(GetMovementComponent());
  Movement->AirControl   = 1.0f;
  Movement->MaxWalkSpeed = 1000.0f;
  Movement->MaxFlySpeed  = 10000.0f;
  Movement->GravityScale = 5.0f;

  bUseControllerRotationYaw   = false;
  bUseControllerRotationPitch = false;
  bUseControllerRotationRoll  = false;

  bReplicates        = true;
  bReplicateMovement = true;
  
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

  DOREPLIFETIME(AOutcastCharacter, CurrentState);
  DOREPLIFETIME(AOutcastCharacter, LastMove);
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

void AOutcastCharacter::ExtractState()
{
  Jumping           = CurrentState.Data.Jumping;
  CharacterLocation = CurrentState.Data.CharacterLocation;
  CharacterRotation = ReturnFRotator(CurrentState.Data.CharacterRotation);
  Health            = CurrentState.Data.Health;

  Speed         = CurrentState.AnimData.Speed;
  WalkPlayrate  = CurrentState.AnimData.WalkPlayrate;
  LegsRotation  = ReturnFRotator(CurrentState.AnimData.LegsRotation);
  TorsoRotation = ReturnFRotator(CurrentState.AnimData.TorsoRotation);
  Attacking     = CurrentState.AnimData.Attacking;
}

void AOutcastCharacter::OnRep_ApplyState()
{
  ExtractState();

  SetActorRotation(CharacterRotation);
  SetActorLocation(CharacterLocation);

  if (Anim)
  {
    Anim->SetIsJumping(Jumping != EJump::NONE);
    Anim->SetAcceleration(Speed);
    Anim->SetWalkPlayrate(WalkPlayrate);
    Anim->SetLegsRotation(LegsRotation);
    Anim->SetTorsoRotation(TorsoRotation);
    Anim->SetIsSlashingLeft(Attacking == EAttack::Left);
    Anim->SetIsSlashingRight(Attacking == EAttack::Right);
    Anim->SetIsSlashingForward(Attacking == EAttack::Forward);
  }
}

void AOutcastCharacter::CreateState()
{
  CurrentState.Data.Jumping           = Jumping;
  CurrentState.Data.CharacterLocation = CharacterLocation;
  CurrentState.Data.CharacterRotation = ReturnFVector(CharacterRotation);
  CurrentState.Data.Health            = Health;

  CurrentState.AnimData.Speed         = Speed;
  CurrentState.AnimData.WalkPlayrate  = WalkPlayrate;
  CurrentState.AnimData.LegsRotation  = ReturnFVector(LegsRotation);
  CurrentState.AnimData.TorsoRotation = ReturnFVector(TorsoRotation);
  CurrentState.AnimData.Attacking     = Attacking;
}

void AOutcastCharacter::OnRep_LastMove()
{
  CleanMoveList();
  ReplayMoves();
}

void AOutcastCharacter::CleanMoveList()
{}

void AOutcastCharacter::ReplayMoves()
{
  if (MoveList.Num() > 0)
  {
    UE_LOG(LogTemp, Warning, TEXT("%s %s"), *LastMove.Time.ToString(), *MoveList.Last().Time.ToString());
  }
}

void AOutcastCharacter::AddCurrentMove()
{
  //TArray<bool> KeyArray;
  //TArray<bool> MouseArray;
  //KeyMap.GenerateValueArray(KeyArray);
  //MouseMap.GenerateValueArray(MouseArray);
  if ( KeyMap.Contains(true)
    || MouseMap.Contains(true)
    || MouseInput.X != 0
    || MouseInput.Y != 0)
  {
    CurrentMove.KeyMap     = KeyMap;
    CurrentMove.MouseMap   = MouseMap;
    CurrentMove.MouseInput = MouseInput;
    CurrentMove.Time       = FDateTime::Now();

    if (IsLocallyControlled() && !HasAuthority())
    {
      MoveList.Add(CurrentMove);
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
  int Random = FMath::RandRange(0, StepSounds.Num() - 1);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), StepSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayJumpVocalSound()
{
  int Random = FMath::RandRange(0, JumpVocalSounds.Num() - 1);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayAttackVocalSound()
{
  int Random = FMath::RandRange(0, AttackVocalSounds.Num() - 1);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayAttackSound()
{
  int Random = 0;//FMath::RandRange(0, 0);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSounds[Random], GetActorLocation(), 0.35f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayHitSound()
{
  int Random = 0;// FMath::RandRange(0, 3);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayHitVocalSound()
{
  int Random = FMath::RandRange(0, HitVocalSounds.Num() - 1);
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitVocalSounds[Random], GetActorLocation(), 0.25f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::PlayIdleHitSound()
{
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), IdleHitSound, GetActorLocation(), 0.5f, 1.0f, 0.0f, Attenuation);
}

void AOutcastCharacter::BodyOverlapBegin(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult)
{
  // Other Actor is the actor that triggered the event. Check that is not ourself.  
  if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
  {
    AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

    if (!DamageTakenBy.Contains(AttackingCharacter))
    {
      DamageTakenBy.Add(AttackingCharacter, 0.0f);

      if (AttackingCharacter->GetAttack() == EAttack::NONE)
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

void AOutcastCharacter::BodyOverlapEnd(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex)
{
  // Other Actor is the actor that triggered the event. Check that is not ourself.  
  if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
  {
    AOutcastCharacter* AttackingCharacter = Cast<AOutcastCharacter>(OtherActor);

    if (DamageTakenBy.Contains(AttackingCharacter))
    {
      DamageTakenBy.Remove(AttackingCharacter);
    }
  }
}

void AOutcastCharacter::Look()
{
  // Rotate whole Character
  CharacterRotation = GetActorRotation();
  CharacterRotation.Yaw = CharacterRotation.Yaw + MouseInput.X;
  SetActorRotation(CharacterRotation);

  // Rotate the torso of the character
  TorsoRotation = Anim->GetTorsoRotation();
  TorsoRotation.Roll = FMath::Clamp(TorsoRotation.Roll - MouseInput.Y, -80.0f, 80.0f);
  Anim->SetTorsoRotation(TorsoRotation);

  // Rotate the camera
  FRotator NewCameraRot = Camera->GetComponentRotation();
  NewCameraRot.Pitch = NewCameraRot.Pitch + MouseInput.Y;

  if (NewCameraRot.Pitch >= -80.0f
    && NewCameraRot.Pitch <= 80.0f)
  {
    Camera->SetWorldRotation(NewCameraRot);

    FVector NewCameraLoc = Camera->RelativeLocation;
    const float NewRadius = NewCameraLoc.Size();
    const float Angle = FMath::Atan2(NewCameraLoc.Z, (NewCameraLoc.X == 0 ? 1 : NewCameraLoc.X));

    NewCameraLoc.Z = NewRadius * FMath::Sin(Angle + FMath::DegreesToRadians(MouseInput.Y));
    NewCameraLoc.X = NewRadius * FMath::Cos(Angle + FMath::DegreesToRadians(MouseInput.Y));

    Camera->SetRelativeLocation(NewCameraLoc);
  }
}

void AOutcastCharacter::Walk()
{
  Direction = FVector(0.0f, 0.0f, 0.0f);
  WalkPlayrate = 1.0f;

  //**** DIRECTION ****
  if (GetKeyPressed(EKey::W))
  {
    Direction = Direction + GetActorRotation().Vector();
  }

  if (GetKeyPressed(EKey::A))
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw - 90.0f;
    Direction = Direction + Rot.Vector();
  }

  if (GetKeyPressed(EKey::S))
  {
    Direction = Direction + GetActorRotation().Vector() * -1;
    WalkPlayrate = -1.0f;
  }

  if (GetKeyPressed(EKey::D))
  {
    FRotator Rot = GetActorRotation();
    Rot.Yaw = Rot.Yaw + 90.0f;
    Direction = Direction + Rot.Vector();
  }
  //**** DIRECTION ****

  //**** SPEED ****
  if ( GetKeyPressed(EKey::W) && !GetKeyPressed(EKey::S)
    || GetKeyPressed(EKey::A) && !GetKeyPressed(EKey::D)
    || GetKeyPressed(EKey::S) && !GetKeyPressed(EKey::W)
    || GetKeyPressed(EKey::D) && !GetKeyPressed(EKey::A))
  {
    Speed = FMath::Clamp(Speed + 5.0f, MinSpeed, MaxSpeed);
  }
  else
  {
    Speed = FMath::Clamp(Speed - 5.0f, MinSpeed, MaxSpeed);
  }

  Anim->SetAcceleration(Speed);
  Anim->SetWalkPlayrate(WalkPlayrate);

  if (Speed == 0.0f)
  {
    Direction = GetActorRotation().Vector();
  }
  //**** SPEED ****

  //**** LEGS ROTATION ****
  if (GetKeyPressed(EKey::W))
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (GetKeyPressed(EKey::A))
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
    else if (GetKeyPressed(EKey::D))
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
  }
  else if (GetKeyPressed(EKey::S))
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    if (GetKeyPressed(EKey::A))
    {
      LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
    }
    else if (GetKeyPressed(EKey::D))
    {
      LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
    }
  }
  else if (GetKeyPressed(EKey::A) && !GetKeyPressed(EKey::D))
  {
    LegsRotation = FRotator(0.0f, -90.0f, 0.0f);
  }
  else if (GetKeyPressed(EKey::D) && !GetKeyPressed(EKey::A))
  {
    LegsRotation = FRotator(0.0f, 90.0f, 0.0f);
  }
  else
  {
    LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
  }

  Anim->SetLegsRotation(LegsRotation);
  //**** LEGS ROTATION ****

  if (!Direction.IsZero())
  {
    if (Jumping == EJump::NONE)
    {
      AddMovementInput(Direction, Speed / 5);
    }
    else if (Jumping == EJump::BunnyHop)
    {
      AddMovementInput(Direction, Speed / BunnyHopSpeedRatio);
    }
    else
    {
      AddMovementInput(Direction, Speed / DefaultJumpSpeedRatio);
    }
  }

  if (HasAuthority())
  {
    CharacterLocation = GetActorLocation();
  }
}

void AOutcastCharacter::Jump()
{
  if (GetKeyPressed(EKey::Space))
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
}


void AOutcastCharacter::DoAttack(const float DeltaTime)
{
  if (Attacking == EAttack::Left)
  {
    Attacking = Anim->GetIsSlashingLeft() ? EAttack::Left : EAttack::NONE;
  }
  else if (Attacking == EAttack::Right)
  {
    Attacking = Anim->GetIsSlashingRight() ? EAttack::Right : EAttack::NONE;
  }
  else if (Attacking == EAttack::Forward)
  {
    Attacking = Anim->GetIsSlashingForward() ? EAttack::Forward : EAttack::NONE;
  }

  if (GetMousePressed(EMouse::Left))
  {
    if (Attacking == EAttack::NONE)
    {
      // Slashing left/right has priority over forward
      if (GetKeyPressed(EKey::A) && !GetKeyPressed(EKey::D))
      {
        Attacking = EAttack::Left;
      }
      else if (!GetKeyPressed(EKey::A) && GetKeyPressed(EKey::D))
      {
        Attacking = EAttack::Right;
      }
      else if (!GetKeyPressed(EKey::A) && !GetKeyPressed(EKey::D) && GetKeyPressed(EKey::W))
      {
        Attacking = EAttack::Forward;
      }
      else
      {
        int Random = FMath::RandRange(1, 3);
        Attacking = static_cast<EAttack>(Random);
      }
    }
  }

  Anim->SetIsSlashingLeft(Attacking == EAttack::Left);
  Anim->SetIsSlashingRight(Attacking == EAttack::Right);
  Anim->SetIsSlashingForward(Attacking == EAttack::Forward);

  // Specify the blend weight for sword attacks/basic movement
  if (Attacking != EAttack::NONE)
  {
    Anim->AddAttackMovementBlendWeight(0.1f);
  }
  else
  {
    Anim->SubtractAttackMovementBlendWeight(0.05f);
  }
}

void AOutcastCharacter::TakeConsecutiveDamage(const float DeltaTime)
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

    if (AttackerIt->Value >= 1.0f)
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

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  Look();

  Walk();

  Jump();

  DoAttack(DeltaTime);

  TakeConsecutiveDamage(DeltaTime);

  AddCurrentMove();

  if (HasAuthority())
  {
    CreateState();
    LastMove = CurrentMove;
  }
}

void AOutcastCharacter::SetMyPlayerController(APlayerController* const NewPlayerController)
{
  MyPlayerController = NewPlayerController;
}

EAttack AOutcastCharacter::GetAttack()
{
  return Attacking;
}

void AOutcastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  // Keyboard input
  PlayerInputComponent->BindAction(
    "W",
    IE_Pressed,
    this,
    &AOutcastCharacter::WPressed);
  PlayerInputComponent->BindAction(
    "W",
    IE_Released,
    this,
    &AOutcastCharacter::WReleased);

  PlayerInputComponent->BindAction(
    "A",
    IE_Pressed,
    this,
    &AOutcastCharacter::APressed);
  PlayerInputComponent->BindAction(
    "A",
    IE_Released,
    this,
    &AOutcastCharacter::AReleased);

  PlayerInputComponent->BindAction(
    "S",
    IE_Pressed,
    this,
    &AOutcastCharacter::SPressed);
  PlayerInputComponent->BindAction(
    "S",
    IE_Released,
    this,
    &AOutcastCharacter::SReleased);

  PlayerInputComponent->BindAction(
    "D",
    IE_Pressed,
    this,
    &AOutcastCharacter::DPressed);
  PlayerInputComponent->BindAction(
    "D",
    IE_Released,
    this,
    &AOutcastCharacter::DReleased);

  PlayerInputComponent->BindAction(
    "Space",
    IE_Pressed,
    this,
    &AOutcastCharacter::SpacePressed);
  PlayerInputComponent->BindAction(
    "Space",
    IE_Released,
    this,
    &AOutcastCharacter::SpaceReleased);

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

bool AOutcastCharacter::GetKeyPressed(const EKey Key)
{
  return KeyMap[static_cast<int>(Key)];
}

void AOutcastCharacter::SetKeyPressed(const EKey Key, const bool bValue)
{
  KeyMap[static_cast<int>(Key)] = bValue;
}

void AOutcastCharacter::WPressed()
{
  //KeyMap.Add(EKey::W, true);
  SetKeyPressed(EKey::W, true);
  Server_SetKey(EKey::W, true);
}

void AOutcastCharacter::WReleased()
{
  //KeyMap.Add(EKey::W, false);
  SetKeyPressed(EKey::W, false);
  Server_SetKey(EKey::W, false);
}

void AOutcastCharacter::APressed()
{
  //KeyMap.Add(EKey::A, true);
  SetKeyPressed(EKey::A, true);
  Server_SetKey(EKey::A, true);
}

void AOutcastCharacter::AReleased()
{
  //KeyMap.Add(EKey::A, false);
  SetKeyPressed(EKey::A, false);
  Server_SetKey(EKey::A, false);
}

void AOutcastCharacter::SPressed()
{
  //KeyMap.Add(EKey::S, true);
  SetKeyPressed(EKey::S, true);
  Server_SetKey(EKey::S, true);
}

void AOutcastCharacter::SReleased()
{
  //KeyMap.Add(EKey::S, false);
  SetKeyPressed(EKey::S, false);
  Server_SetKey(EKey::S, false);
}

void AOutcastCharacter::DPressed()
{
  //KeyMap.Add(EKey::D, true);
  SetKeyPressed(EKey::D, true);
  Server_SetKey(EKey::D, true);
}

void AOutcastCharacter::DReleased()
{
  //KeyMap.Add(EKey::D, false);
  SetKeyPressed(EKey::D, false);
  Server_SetKey(EKey::D, false);
}

void AOutcastCharacter::SpacePressed()
{
  //KeyMap.Add(EKey::Space, true);
  SetKeyPressed(EKey::Space, true);
  Server_SetKey(EKey::Space, true);
}

void AOutcastCharacter::SpaceReleased()
{
  //KeyMap.Add(EKey::Space, false);
  SetKeyPressed(EKey::Space, false);
  Server_SetKey(EKey::Space, false);
}

void AOutcastCharacter::Server_SetKey_Implementation(const EKey Key, const bool bIsPressed)
{
  //KeyMap.Add(Key, bIsPressed);
  SetKeyPressed(Key, bIsPressed);
}

bool AOutcastCharacter::Server_SetKey_Validate(const EKey Key, const bool bIsPressed)
{
  return true;
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
  Server_SetMouse(EMouse::Left, true);
}

void AOutcastCharacter::MouseRightPressed()
{
  SetMousePressed(EMouse::Right, true);
  Server_SetMouse(EMouse::Right, true);
}

void AOutcastCharacter::MouseLeftReleased()
{
  SetMousePressed(EMouse::Left, false);
  Server_SetMouse(EMouse::Left, false);
}

void AOutcastCharacter::MouseRightReleased()
{
  SetMousePressed(EMouse::Right, false);
  Server_SetMouse(EMouse::Right, false);
}

void AOutcastCharacter::MouseUpDown(const float AxisValue)
{
  MouseInput.Y = AxisValue;
  Server_SetMouseInput(MouseInput);
}

void AOutcastCharacter::MouseRightLeft(const float AxisValue)
{
  MouseInput.X = AxisValue;
  Server_SetMouseInput(MouseInput);
}

void AOutcastCharacter::Server_SetMouse_Implementation(const EMouse Key, const bool bIsPressed)
{
  //MouseMap.Add(Key, bIsPressed);
  SetMousePressed(Key, bIsPressed);
}

bool AOutcastCharacter::Server_SetMouse_Validate(const EMouse Key, const bool bIsPressed)
{
  return true;
}

void AOutcastCharacter::Server_SetMouseInput_Implementation(const FVector2D NewMouseInput)
{
  MouseInput = NewMouseInput;
}

bool AOutcastCharacter::Server_SetMouseInput_Validate(const FVector2D NewMouseInput)
{
  return true;
}

void AOutcastCharacter::OnHit(
  UPrimitiveComponent* HitComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  FVector NormalImpulse,
  const FHitResult& Hit)
{
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
  }
}