#pragma once

#include "CoreMinimal.h"
#include "ChunkMeshData.h"
#include "GameFramework/Actor.h"
#include "GreedyChunk.generated.h"


enum class EBlock : uint8;
class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class AGreedyChunk : public AActor
{
    GENERATED_BODY()

    struct FMask
    {
        EBlock Block;
        int Normal;
    };

public:
    AGreedyChunk();

    UPROPERTY(EditAnywhere, Category = "Height")
    float Frequency = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Chunk")
    FIntVector Size = FIntVector(1, 1, 1) * 32;

    UPROPERTY(EditAnywhere, Category = "Chunk")
    bool is3D;

    UPROPERTY(EditAnywhere, Category = "Chunk")
    TObjectPtr<UMaterialInstance> VertexMaterial;

    UFUNCTION(BlueprintCallable, Category = "Chunk")
    void ModifyVoxel(const FIntVector Position, const EBlock Block);

    void ModifyVoxelData(const FIntVector Position, EBlock Block);

    void ClearMesh();

    AGreedyChunk* FrontChunk;
    AGreedyChunk* RearChunk;
    AGreedyChunk* RightChunk;
    AGreedyChunk* LeftChunk;


protected:
    virtual void BeginPlay() override;

private:
    TObjectPtr<UProceduralMeshComponent> Mesh;
    TObjectPtr<FastNoiseLite> Noise;

    FChunkMeshData MeshData;
    TArray<EBlock> Blocks;

    int VertexCount = 0;

    void GenerateBlocks();

    void GenerateMesh();

    void ApplyMesh() const;

    void CreateQuad(const FMask Mask, const FIntVector AxisMask, const int Width, const int Height, const FIntVector V1, const FIntVector V2, const FIntVector V3, const FIntVector V4);

    int GetBlockIndex(int X, int Y, int Z) const;

    EBlock GetBlock(FIntVector Index) const;

    bool CompareMask(FMask Mask1, FMask Mask2) const;

    int GetTextureIndex(EBlock Block, FVector Normal) const;

    void Generate2DHeightMap(const FVector Position);

    void Generate3DHeightMap(const FVector Position);

    void GenerateHeightMap();

    void GenerateTree(int PosX, int PosY, int PosZ);

    float GetNearestPos(FVector basePos, TArray<FVector> positions);

    void GenerateOre(int PosX, int PosY, int PosZ, EBlock ore, int spawnChance);
};