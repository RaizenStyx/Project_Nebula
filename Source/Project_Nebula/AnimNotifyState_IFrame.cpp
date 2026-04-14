#include "AnimNotifyState_IFrame.h"
#include "Project_NebulaCharacter.h"

void UAnimNotifyState_IFrame::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        AProject_NebulaCharacter* Player = Cast<AProject_NebulaCharacter>(MeshComp->GetOwner());
        if (Player)
        {
            Player->bIsInvincible = true;
        }
    }
}

void UAnimNotifyState_IFrame::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        AProject_NebulaCharacter* Player = Cast<AProject_NebulaCharacter>(MeshComp->GetOwner());
        if (Player)
        {
            Player->bIsInvincible = false;
        }
    }
}