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
  Jumping(EJump::NONE),
  JumpHeight(0.0f),
  JumpStartLocZ(0.0f),
  BunnyHopSpeedRatio(DefaultJumpSpeedRatio),
  Attacking(EAttack::NONE),
  Health(100),
  LastHealth(100),
  LeftMouseTimer(0.0f),
  MouseInput(FVector2D::ZeroVector)
{
 	PrimaryActorTick.bCanEverTick = true;

  // Initialize KeyMap
  KeyMap.Add(EKeys::W, false);
  KeyMap.Add(EKeys::A, false);
  KeyMap.Add(EKeys::S, false);
  KeyMap.Add(EKeys::D, false);
  KeyMap.Add(EKeys::Space, false);

  // Initialize Mouse Input
  MouseMap.Add(EMouse::Left, false);
  MouseMap.Add(EMouse::Right, false);

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
}

void AOutcastCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(AOutcastCharacter, Speed);
  DOREPLIFETIME(AOutcastCharacter, WalkPlayrate);
  DOREPLIFETIME(AOutcastCharacter, LegsRotation);
  DOREPLIFETIME(AOutcastCharacter, TorsoRotation);
  DOREPLIFETIME(AOutcastCharacter, CharacterRotation);
  DOREPLIFETIME(AOutcastCharacter, Jumping);
  DOREPLIFETIME(AOutcastCharacter, Attacking);
  DOREPLIFETIME(AOutcastCharacter, CharacterLocation);
  DOREPLIFETIME(AOutcastCharacter, Health);
}

void AOutcastCharacter::BeginPlay()
{
  Super::BeginPlay();

  Anim = Cast<UOutcastAnimInstance>(Body->GetAnimInstance());
  if (!Anim)
  {
    Destroy();
  }

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

void AOutcastCharacter::SetSpeed(const float NewSpeed)
{
  Speed = NewSpeed;
  if (!HasAuthority())
  {
    Server_SetSpeed(NewSpeed);
  }
}
void AOutcastCharacter::Server_SetSpeed_Implementation(const float NewSpeed)
{
  Speed = NewSpeed;
}
bool AOutcastCharacter::Server_SetSpeed_Validate(const float NewSpeed)
{
  return true;
}

void AOutcastCharacter::SetWalkPlayrate(const float NewWalkPlayrate)
{
  WalkPlayrate = NewWalkPlayrate;
  if (!HasAuthority())
  {
    Server_SetWalkPlayrate(NewWalkPlayrate);
  }
}
void AOutcastCharacter::Server_SetWalkPlayrate_Implementation(const float NewWalkPlayrate)
{
  WalkPlayrate = NewWalkPlayrate;
}
bool AOutcastCharacter::Server_SetWalkPlayrate_Validate(const float NewWalkPlayrate)
{
  return true;
}

void AOutcastCharacter::SetLegsRotation(const FRotator NewLegsRotation)
{
  LegsRotation = NewLegsRotation;
  if (!HasAuthority())
  {
    Server_SetLegsRotation(NewLegsRotation);
  }
}
void AOutcastCharacter::Server_SetLegsRotation_Implementation(const FRotator NewLegsRotation)
{
  LegsRotation = NewLegsRotation;
}
bool AOutcastCharacter::Server_SetLegsRotation_Validate(const FRotator NewLegsRotation)
{
  return true;
}

void AOutcastCharacter::SetTorsoRotation(const FRotator NewTorsoRotation)
{
  TorsoRotation = NewTorsoRotation;
  if (!HasAuthority())
  {
    Server_SetTorsoRotation(NewTorsoRotation);
  }
}
void AOutcastCharacter::Server_SetTorsoRotation_Implementation(const FRotator NewTorsoRotation)
{
  TorsoRotation = NewTorsoRotation;
}
bool AOutcastCharacter::Server_SetTorsoRotation_Validate(const FRotator NewTorsoRotation)
{
  return true;
}

void AOutcastCharacter::SetCharacterRotation(const FRotator NewCharacterRotation)
{
  CharacterRotation = NewCharacterRotation;
  if (!HasAuthority())
  {
    Server_SetCharacterRotation(CharacterRotation);
  }
}
void AOutcastCharacter::Server_SetCharacterRotation_Implementation(const FRotator NewCharacterRotation)
{
  CharacterRotation = NewCharacterRotation;
}
bool AOutcastCharacter::Server_SetCharacterRotation_Validate(const FRotator NewCharacterRotation)
{
  return true;
}

void AOutcastCharacter::SetJumping(const EJump NewJumping)
{
  Jumping = NewJumping;
  if (!HasAuthority())
  {
    Server_SetJumping(Jumping);
  }
}
void AOutcastCharacter::Server_SetJumping_Implementation(const EJump NewJumping)
{
  Jumping = NewJumping;
}
bool AOutcastCharacter::Server_SetJumping_Validate(const EJump NewJumping)
{
  return true;
}

void AOutcastCharacter::SetAttack(const EAttack NewAttack)
{
  Attacking = NewAttack;
  if (!HasAuthority())
  {
    Server_SetAttack(Attacking);
  }
}
void AOutcastCharacter::Server_SetAttack_Implementation(const EAttack NewAttack)
{
  Attacking = NewAttack;
}
bool AOutcastCharacter::Server_SetAttack_Validate(const EAttack NewAttack)
{
  return true;
}

void AOutcastCharacter::SetHealth(const int NewHealth)
{
  Health = NewHealth;
  if (!HasAuthority())
  {
    Server_SetHealth(Health);
  }
}
void AOutcastCharacter::Server_SetHealth_Implementation(const int NewHealth)
{
  Health = NewHealth;
}
bool AOutcastCharacter::Server_SetHealth_Validate(const int NewHealth)
{
  return true;
}

int AOutcastCharacter::GetHealth()
{
  return Health;
}

void AOutcastCharacter::BodyOverlapBegin(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult)
{
  if (Role == ROLE_AutonomousProxy)
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
          SetHealth(FMath::Clamp(Health - 1, 0, 100));
        }
        else
        {
          SetHealth(FMath::Clamp(Health - 20, 0, 100));
        }
      }
    }
  }
}

void AOutcastCharacter::BodyOverlapEnd(
  UPrimitiveComponent* OverlappedComp,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex)
{
  if (Role == ROLE_AutonomousProxy)
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
}

void AOutcastCharacter::LookAround()
{
  if (Role == ROLE_AutonomousProxy)
  {
    // Rotate whole Character
    CharacterRotation = GetActorRotation();
    CharacterRotation.Yaw = CharacterRotation.Yaw + MouseInput.X;
    SetCharacterRotation(CharacterRotation);

    // Rotate the torso of the character
    TorsoRotation = Anim->GetTorsoRotation();
    TorsoRotation.Roll = FMath::Clamp(TorsoRotation.Roll - MouseInput.Y, -80.0f, 80.0f);
    SetTorsoRotation(TorsoRotation);

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

  Anim->SetTorsoRotation(TorsoRotation);
  SetActorRotation(CharacterRotation);
}

void AOutcastCharacter::MoveAround()
{
  if (Role == ROLE_AutonomousProxy)
  {
    Direction = FVector(0.0f, 0.0f, 0.0f);
    WalkPlayrate = 1.0f;

    //**** DIRECTION ****
    if (KeyMap[EKeys::W])
    {
      Direction = Direction + GetActorRotation().Vector();
    }

    if (KeyMap[EKeys::A])
    {
      FRotator Rot = GetActorRotation();
      Rot.Yaw      = Rot.Yaw - 90.0f;
      Direction    = Direction + Rot.Vector();
    }

    if (KeyMap[EKeys::S])
    {
      Direction    = Direction + GetActorRotation().Vector() * -1;
      WalkPlayrate = -1.0f;
    }

    if (KeyMap[EKeys::D])
    {
      FRotator Rot = GetActorRotation();
      Rot.Yaw      = Rot.Yaw + 90.0f;
      Direction    = Direction + Rot.Vector();
    }
    //**** DIRECTION ****

    //**** SPEED ****
    if ( KeyMap[EKeys::W] && !KeyMap[EKeys::S]
      || KeyMap[EKeys::A] && !KeyMap[EKeys::D]
      || KeyMap[EKeys::S] && !KeyMap[EKeys::W]
      || KeyMap[EKeys::D] && !KeyMap[EKeys::A])
    {
      Speed = FMath::Clamp(Speed + 5.0f, MinSpeed, MaxSpeed);
    }
    else
    {
      Speed = FMath::Clamp(Speed - 5.0f, MinSpeed, MaxSpeed);
    }

    SetSpeed(Speed);
    SetWalkPlayrate(WalkPlayrate);
  }

  Anim->SetAcceleration(Speed);
  Anim->SetWalkPlayrate(WalkPlayrate);

  if (Speed == 0.0f)
  {
    Direction = GetActorRotation().Vector();
  }
  //**** SPEED ****

  //**** LEGS ROTATION ****
  if (Role == ROLE_AutonomousProxy)
  {
    if (KeyMap[EKeys::W])
    {
      LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
      if (KeyMap[EKeys::A])
      {
        LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
      }
      else if (KeyMap[EKeys::D])
      {
        LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
      }
    }
    else if (KeyMap[EKeys::S])
    {
      LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
      if (KeyMap[EKeys::A])
      {
        LegsRotation = FRotator(0.0f, 45.0f, 0.0f);
      }
      else if (KeyMap[EKeys::D])
      {
        LegsRotation = FRotator(0.0f, -45.0f, 0.0f);
      }
    }
    else if (KeyMap[EKeys::A] && !KeyMap[EKeys::D])
    {
      LegsRotation = FRotator(0.0f, -90.0f, 0.0f);
    }
    else if (KeyMap[EKeys::D] && !KeyMap[EKeys::A])
    {
      LegsRotation = FRotator(0.0f, 90.0f, 0.0f);
    }
    else
    {
      LegsRotation = FRotator(0.0f, 0.0f, 0.0f);
    }

    SetLegsRotation(LegsRotation);
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
  else
  {
    SetActorLocation(CharacterLocation);
  }
}

void AOutcastCharacter::Jump()
{
  if (Role == ROLE_AutonomousProxy)
  {
    if (KeyMap[EKeys::Space])
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
          SetJumping(EJump::Upwards);
        }
        else
        {
          SetJumping(EJump::Downwards);
        }
      }
    }
    else if ((Jumping == EJump::Upwards || Jumping == EJump::BunnyHop) && JumpHeight >= MinJumpHeight)
    {
      SetJumping(EJump::Downwards);
    }
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
    //Movement->SetMovementMode(MOVE_Walking);
  }
  else if (Jumping == EJump::BunnyHop)
  {
    JumpHeight = GetActorLocation().Z - JumpStartLocZ;

    AddMovementInput(GetActorUpVector(), 5.0f);
    Movement->SetMovementMode(MOVE_Flying);
    if (Role == ROLE_AutonomousProxy)
    {
      if (JumpHeight > BunnyHopMaxHeight)
      {
        SetJumping(EJump::Upwards);
        BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
      }
    }
  }
}

void AOutcastCharacter::Attack(const float DeltaTime)
{
  if (Role == ROLE_AutonomousProxy)
  {
    // Otherwise SimulatedProxies won't know when the attack animation has finished
    if (Attacking == EAttack::Left)
    {
      SetAttack(Anim->GetIsSlashingLeft() ? EAttack::Left : EAttack::NONE);
    }
    else if (Attacking == EAttack::Right)
    {
      SetAttack(Anim->GetIsSlashingRight() ? EAttack::Right : EAttack::NONE);
    }
    else if (Attacking == EAttack::Forward)
    {
      SetAttack(Anim->GetIsSlashingForward() ? EAttack::Forward : EAttack::NONE);
    }

    if (MouseMap[EMouse::Left])
    {
      // Disable holding down the left mouse button
      // Each attack has to be pressed separatly
      LeftMouseTimer += DeltaTime;
      if (LeftMouseTimer >= 1.0f)
      {
        MouseMap[EMouse::Left] = false;
        LeftMouseTimer = 0.0f;
      }

      if (Attacking == EAttack::NONE)
      {
        // Slashing left/right has priority over forward
        if (KeyMap[EKeys::A]
          && !KeyMap[EKeys::D])
        {
          SetAttack(EAttack::Left);
        }
        else if (!KeyMap[EKeys::A]
          && KeyMap[EKeys::D])
        {
          SetAttack(EAttack::Right);
        }
        else if (!KeyMap[EKeys::A]
          && !KeyMap[EKeys::D]
          && KeyMap[EKeys::W])
        {
          SetAttack(EAttack::Forward);
        }
        else
        {
          int Random = FMath::RandRange(1, 3);
          SetAttack(static_cast<EAttack>(Random));
        }
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

void AOutcastCharacter::Alive(const float DeltaTime)
{
  if (HasAuthority())
  {
    if (Health == 0)
    {
      Cast<AOutcastGameMode>(GetWorld()->GetAuthGameMode())->Respawn(MyPlayerController);
    }
  }
}

void AOutcastCharacter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  LookAround();

  MoveAround();

  Jump();

  Attack(DeltaTime);

  Alive(DeltaTime);

  for (auto AttackerIt = DamageTakenBy.CreateIterator(); AttackerIt; ++AttackerIt)
  {
    AttackerIt->Value = AttackerIt->Value + DeltaTime;

    if (AttackerIt->Value >= 1.0f)
    {
      AttackerIt->Value = 0.0f;

      if (AttackerIt->Key->GetAttack() == EAttack::NONE)
      {
        SetHealth(FMath::Clamp(Health - 1, 0, 100));
      }
      else
      {
        SetHealth(FMath::Clamp(Health - 20, 0, 100));
      }
    }
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

void AOutcastCharacter::WPressed()
{
  KeyMap.Add(EKeys::W, true);
}

void AOutcastCharacter::WReleased()
{
  KeyMap.Add(EKeys::W, false);
}

void AOutcastCharacter::APressed()
{
  KeyMap.Add(EKeys::A, true);
}

void AOutcastCharacter::AReleased()
{
  KeyMap.Add(EKeys::A, false);
}

void AOutcastCharacter::SPressed()
{
  KeyMap.Add(EKeys::S, true);
}

void AOutcastCharacter::SReleased()
{
  KeyMap.Add(EKeys::S, false);
}

void AOutcastCharacter::DPressed()
{
  KeyMap.Add(EKeys::D, true);
}

void AOutcastCharacter::DReleased()
{
  KeyMap.Add(EKeys::D, false);
}

void AOutcastCharacter::SpacePressed()
{
  KeyMap.Add(EKeys::Space, true);
}

void AOutcastCharacter::SpaceReleased()
{
  KeyMap.Add(EKeys::Space, false);
}

void AOutcastCharacter::MouseLeftPressed()
{
  MouseMap.Add(EMouse::Left, true);
  LeftMouseTimer = 0.0f;
}

void AOutcastCharacter::MouseRightPressed()
{
  MouseMap.Add(EMouse::Right, true);
}

void AOutcastCharacter::MouseLeftReleased()
{
  MouseMap.Add(EMouse::Left, false);
}

void AOutcastCharacter::MouseRightReleased()
{
  MouseMap.Add(EMouse::Right, false);
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
  // Character is on top of a walkable plane
  if (OtherActor->ActorHasTag(FName("Walkable")))
  {
    if (Role == ROLE_AutonomousProxy)
    {
      if (KeyMap[EKeys::Space]
        && (KeyMap[EKeys::A] || KeyMap[EKeys::D])
        && (Jumping == EJump::Downwards || Jumping == EJump::BunnyHop))
      {
        SetJumping(EJump::BunnyHop);
        JumpHeight = 0.0f;
        BunnyHopSpeedRatio = FMath::Clamp(BunnyHopSpeedRatio / 3.0f, BunnyHopFastestSpeedRatio, DefaultJumpSpeedRatio);
      }
      else
      {
        SetJumping(EJump::NONE);
        BunnyHopSpeedRatio = DefaultJumpSpeedRatio;
      }
    }
  }
}