// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "GreedyChunk.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
AChunkWorld::AChunkWorld()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();

	//FIntVector playerPos = static_cast<FIntVector>(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());

	FVector playerPos = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation();



	FVector chunkPos = GetRoundPos(playerPos);

	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(FVector(chunkPos.X * 100 * ChunkSize + ChunkSize * 100 / 2, chunkPos.Y * 100 * ChunkSize + ChunkSize * 100 / 2, 2200));

	AGreedyChunk* baseChunk = SpawnChunk(chunkPos);

	ExtandChunk(chunkPos, chunkPos);


	/*for (int x = -DrawDistance; x <= DrawDistance; ++x)
	{
		for (int y = -DrawDistance; y <= DrawDistance; ++y)
		{
			GetWorld()->SpawnActor<AActor>(Chunk, FVector(x * ChunkSize * 100, y * ChunkSize * 100, 0), FRotator::ZeroRotator);
		}
	}*/




}

void AChunkWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("PLay")));

	FIntVector playerPos = static_cast<FIntVector>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation());



	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("PLayer : %f / %f"), playerPos.X, playerPos.Y));*/

	timer += FApp::GetDeltaTime();


	if (timer > ChunkSpawnRate)
	{
		FVector playerPos = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation();
		FVector roundedPlayerPos = GetRoundPos(playerPos);

		timer = 0;
		ExtandChunk(roundedPlayerPos, roundedPlayerPos);
	}
}

void AChunkWorld::ExtandChunk(FVector ActChunkPos, FVector InitPos)
{
	if (FVector::Dist(ActChunkPos, InitPos) > DrawDistance) return;

	AGreedyChunk* ActChunk = *(spawnedChuncks.Find(ActChunkPos));

	FVector front = FVector(ActChunkPos.X + 1, ActChunkPos.Y, 0);
	FVector rear = FVector(ActChunkPos.X - 1, ActChunkPos.Y, 0);
	FVector right = FVector(ActChunkPos.X, ActChunkPos.Y + 1, 0);
	FVector left = FVector(ActChunkPos.X, ActChunkPos.Y - 1, 0);

	TArray<FVector> ChunksToExtand;

	if (!spawnedChuncks.Contains(front)) { auto newChunk = SpawnChunk(front); ActChunk->FrontChunk = newChunk; ChunksToExtand.Add(front); }
	else
	{
		AGreedyChunk* newChunk = *(spawnedChuncks.Find(front)); ActChunk->FrontChunk = newChunk;
		if (FVector::Dist(ActChunkPos, InitPos) <= FVector::Dist(front, InitPos)) ChunksToExtand.Add(front);
	}

	if (!spawnedChuncks.Contains(rear)) { auto newChunk = SpawnChunk(rear); ActChunk->RearChunk = newChunk; ChunksToExtand.Add(rear); }
	else
	{
		AGreedyChunk* newChunk = *(spawnedChuncks.Find(rear)); ActChunk->RearChunk = newChunk;
		if (FVector::Dist(ActChunkPos, InitPos) <= FVector::Dist(rear, InitPos)) ChunksToExtand.Add(rear);
	}

	if (!spawnedChuncks.Contains(right)) { auto newChunk = SpawnChunk(right); ActChunk->RightChunk = newChunk; ChunksToExtand.Add(right); }
	else
	{
		AGreedyChunk* newChunk = *(spawnedChuncks.Find(right)); ActChunk->RightChunk = newChunk;
		if (FVector::Dist(ActChunkPos, InitPos) <= FVector::Dist(right, InitPos)) ChunksToExtand.Add(right);
	}

	if (!spawnedChuncks.Contains(left)) { auto newChunk = SpawnChunk(left); ActChunk->LeftChunk = newChunk; ChunksToExtand.Add(left); }
	else
	{
		AGreedyChunk* newChunk = *(spawnedChuncks.Find(left)); ActChunk->LeftChunk = newChunk;
		if (FVector::Dist(ActChunkPos, InitPos) <= FVector::Dist(left, InitPos)) ChunksToExtand.Add(left);
	}

	for (FVector tmp : ChunksToExtand)
	{
		ExtandChunk(tmp, InitPos);
	}
}

AGreedyChunk* AChunkWorld::SpawnChunk(FVector Position)
{
	FVector NewPosition = Position;
	NewPosition.Z = 0;
	NewPosition *= ChunkSize * 100;



	AActor* actor = GetWorld()->SpawnActor<AActor>(Chunk, NewPosition, FRotator::ZeroRotator);

	AGreedyChunk* c = Cast<AGreedyChunk>(actor);

	spawnedChuncks.Add(Position, c);

	return c;
}

FVector AChunkWorld::GetRoundPos(FVector Position)
{
	FVector Result = Position;
	Result.Z = 0;

	Result.X -= ChunkSize * 100 / 2;
	Result.Y -= ChunkSize * 100 / 2;

	Result.X = FMath::Floor(Result.X / (ChunkSize * 100));
	Result.Y = FMath::Floor(Result.Y / (ChunkSize * 100));

	return Result;
}
