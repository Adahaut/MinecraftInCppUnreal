// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreedyChunk.h"
#include "ChunkWorld.generated.h"


UCLASS()
class AChunkWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunkWorld();

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	TSubclassOf<AActor> Chunk;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	int DrawDistance = 5;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	int ChunkSize = 32;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	float ChunkSpawnRate = 5;

	virtual void Tick(float DeltaTime) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	void ExtandChunk(FVector ActChunkPos, FVector InitPos);

	AGreedyChunk* SpawnChunk(FVector Position);

	float timer = 0;

	TMap<FVector, AGreedyChunk*> spawnedChuncks;

	FVector GetRoundPos(FVector Position);
};
