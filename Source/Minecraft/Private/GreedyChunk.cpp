#include "GreedyChunk.h"

#include "Enums.h"
#include "FastNoiseLite.h"
#include "ProceduralMeshComponent.h"

AGreedyChunk::AGreedyChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	Noise = new FastNoiseLite();
	
	

	Blocks.SetNum(Size.X * Size.Y * Size.Z);

	Mesh->SetCastShadow(false);

	SetRootComponent(Mesh);
}

void AGreedyChunk::ModifyVoxel(const FIntVector Position, const EBlock Block)
{
	if (Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Bedrock) return;

	//if (Position.X >= Size.X || Position.Y >= Size.Y || Position.Z >= Size.Z || Position.X < 0 || Position.Y < 0 || Position.Z < 0) return;
	if (Position.Z >= Size.Z || Position.Z < 0) return;

	else if (Position.X < 0) {
		if (RearChunk != nullptr) {
			RearChunk->ModifyVoxel(FIntVector(Position.X + Size.X, Position.Y, Position.Z), Block);
		}
	}
	else if (Position.X >= Size.X) {
		if (FrontChunk != nullptr) {
			FrontChunk->ModifyVoxel(FIntVector(Position.X - Size.X, Position.Y, Position.Z), Block);
		}
	}
	else if (Position.Y < 0) {
		if (LeftChunk != nullptr) {
			LeftChunk->ModifyVoxel(FIntVector(Position.X, Position.Y + Size.Y, Position.Z), Block);
		}
	}
	else if (Position.Y >= Size.Y) {
		if (RightChunk != nullptr) {
			RightChunk->ModifyVoxel(FIntVector(Position.X, Position.Y - Size.Y, Position.Z), Block);
		}
	}

	ModifyVoxelData(Position, Block);

	ClearMesh();

	GenerateMesh();

	ApplyMesh();
}

void AGreedyChunk::ModifyVoxelData(const FIntVector Position, EBlock Block)
{
	const int Index = GetBlockIndex(Position.X, Position.Y, Position.Z);

	Blocks[Index] = Block;
}

void AGreedyChunk::ClearMesh()
{
	VertexCount = 0;
	MeshData.Clear();
}

void AGreedyChunk::BeginPlay()
{
	Super::BeginPlay();


	Noise->SetFrequency(Frequency);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);
	


	//GenerateBlocks();

	GenerateHeightMap();

	GenerateMesh();

	ApplyMesh();

}

void AGreedyChunk::GenerateBlocks()
{
	const auto Location = GetActorLocation();

	for (int x = 0; x < Size.X; ++x)
	{
		for (int y = 0; y < Size.Y; ++y)
		{
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;

			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, Ypos) + 1) * Size.Z / 2), 0, Size.Z);

			for (int z = 0; z < Height; ++z)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}

			for (int z = Height; z < Size.Z; ++z)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}
}

void AGreedyChunk::Generate2DHeightMap(const FVector Position)
{
	int treesToSpawn = FMath::RandRange(0, 30);

	TArray<FVector> treesLocation;

	TArray<FVector> oresLocation;

	int treeWidth = 2;
	
	for (int i = 0; i < treesToSpawn; ++i)
	{
		FVector newTree;

		do {
			newTree = FVector(FMath::RandRange(treeWidth, Size.X - treeWidth), FMath::RandRange(treeWidth, Size.Y - treeWidth), 0);
		} while (GetNearestPos(newTree, treesLocation) < 2.0f);

		treesLocation.Add(newTree);
	}

	for (int x = 0; x < Size.X; x++)
	{
		for (int y = 0; y < Size.Y; y++)
		{
			const float Xpos = x + Position.X;
			const float Ypos = y + Position.Y;

			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, Ypos) + 1) * Size.Z / 2), 0, Size.Z);

			for (int z = 0; z < Size.Z; z++)
			{
				if (Blocks[GetBlockIndex(x, y, z)] != EBlock::Null) continue;

				if (z == 0) Blocks[GetBlockIndex(x, y, z)] = EBlock::Bedrock;
				else if (z < Height - 3)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;

					if (z < Height - 8)
					{
						int rand = FMath::RandRange(0, 1000);
						if (rand < 7)
						{
							oresLocation.Add(FVector(x, y, z));
						}
					}
				}
				else if (z < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
				else if (z == Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Grass;
				else Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}

	for (auto treePos : treesLocation)
	{
		const float Xpos = treePos.X + Position.X;
		const float Ypos = treePos.Y + Position.Y;
		const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(Xpos, Ypos) + 1) * Size.Z / 2), 0, Size.Z);

		GenerateTree(treePos.X, treePos.Y, Height);
	}

	for (auto orePos : oresLocation)
	{
		GenerateOre(orePos.X, orePos.Y, orePos.Z, EBlock::IronOre, 50);
	}
}

void AGreedyChunk::Generate3DHeightMap(const FVector Position)
{
	for (int x = 0; x < Size.X; ++x)
	{
		for (int y = 0; y < Size.Y; ++y)
		{
			for (int z = 0; z < Size.Z; ++z)
			{
				const auto NoiseValue = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);

				if (NoiseValue >= 0)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
				else
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
				}
			}
		}
	}
}

void AGreedyChunk::GenerateHeightMap()
{
	if (is3D)
	{
		Generate3DHeightMap(GetActorLocation() / 100);
	}
	else
	{
		Generate2DHeightMap(GetActorLocation() / 100);
	}

}

void AGreedyChunk::GenerateTree(int PosX, int PosY, int PosZ)
{

	Blocks[GetBlockIndex(PosX, PosY, PosZ)] = EBlock::Wood;
	Blocks[GetBlockIndex(PosX, PosY, PosZ + 1)] = EBlock::Wood;
	Blocks[GetBlockIndex(PosX, PosY, PosZ + 2)] = EBlock::Wood;
	Blocks[GetBlockIndex(PosX, PosY, PosZ + 3)] = EBlock::Leaf;

	int treeHeight = 2;

	if (FMath::RandRange(0, 100) > 50) {
		Blocks[GetBlockIndex(PosX, PosY, PosZ + 3)] = EBlock::Wood;
		Blocks[GetBlockIndex(PosX, PosY, PosZ + 4)] = EBlock::Leaf;
		treeHeight++;
		if (FMath::RandRange(0, 100) > 70) {
			Blocks[GetBlockIndex(PosX, PosY, PosZ + 4)] = EBlock::Wood;
			Blocks[GetBlockIndex(PosX, PosY, PosZ + 5)] = EBlock::Leaf;
			treeHeight++;
		}
	}
	
	//int leavesStart = FMath::RandRange(0, treeHeight) + 2;

	TArray<FVector> leavesOrganization = TArray<FVector>({
		FVector(1, 0, 0),
		FVector(-1, 0, 0),
		FVector(0, 1, 0),
		FVector(0, -1, 0),
		});

	int leavesType = FMath::RandRange(0, 2);

	if (leavesType == 0)
	{
		Blocks[GetBlockIndex(PosX + 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX - 1, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX + 1, PosY, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY + 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY - 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
	}
	else if (leavesType == 1)
	{
		Blocks[GetBlockIndex(PosX + 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX - 1, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX + 1, PosY, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY + 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY - 1, PosZ + treeHeight + 1)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX - 1, PosY - 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY - 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY + 1, PosZ + treeHeight + 1)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX + 1, PosY + 1, PosZ + treeHeight + 1)] = EBlock::Leaf;

		Blocks[GetBlockIndex(PosX, PosY, PosZ + treeHeight + 2)] = EBlock::Leaf;
	}
	else
	{
		Blocks[GetBlockIndex(PosX + 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX - 1, PosY, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY + 1, PosZ + treeHeight)] = EBlock::Leaf;
		Blocks[GetBlockIndex(PosX, PosY - 1, PosZ + treeHeight)] = EBlock::Leaf;
	}

	
}

float AGreedyChunk::GetNearestPos(FVector basePos, TArray<FVector> positions)
{
	float nearest = 9999.99f;

	for (auto pos : positions)
	{
		float newDist = FVector::Dist(basePos, pos);
		if (newDist < nearest) nearest = newDist;

	}

	return nearest;
}

void AGreedyChunk::GenerateOre(int PosX, int PosY, int PosZ, EBlock ore, int spawnChance = 100)
{
	if (FMath::RandRange(0, 100) > spawnChance || Blocks[GetBlockIndex(PosX, PosY, PosZ)] != EBlock::Stone || Blocks[GetBlockIndex(PosX, PosY, PosZ)] == EBlock::IronOre) return;

	Blocks[GetBlockIndex(PosX, PosY, PosZ)] = EBlock::IronOre;

	GenerateOre(PosX + 1, PosY, PosZ, ore, spawnChance / 2);
	GenerateOre(PosX - 1, PosY, PosZ, ore, spawnChance / 2);
	GenerateOre(PosX, PosY + 1, PosZ, ore, spawnChance / 2);
	GenerateOre(PosX, PosY - 1, PosZ, ore, spawnChance / 2);
	GenerateOre(PosX, PosY, PosZ + 1, ore, spawnChance / 2);
	GenerateOre(PosX, PosY, PosZ - 1, ore, spawnChance / 2);


}

void AGreedyChunk::GenerateMesh()
{
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;

		const int MainAxisLimit = Size[Axis];

		int Axis1Limit = Size[Axis1];
		int Axis2Limit = Size[Axis2];

		auto DeltaAxis1 = FIntVector::ZeroValue;
		auto DeltaAxis2 = FIntVector::ZeroValue;

		auto ChunkItr = FIntVector::ZeroValue;
		auto AxisMask = FIntVector::ZeroValue;

		AxisMask[Axis] = 1;

		TArray<FMask> Mask;
		Mask.SetNum(Axis1Limit * Axis2Limit);

		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;)
		{
			int N = 0;

			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis2Limit; ++ChunkItr[Axis2])
			{
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis1Limit; ++ChunkItr[Axis1])
				{
					const auto CurrentBlock = GetBlock(ChunkItr);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask);

					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;

					if (CurrentBlockOpaque == CompareBlockOpaque)
					{
						Mask[N++] = FMask{ EBlock::Null, 0 };
					}
					else if (CurrentBlockOpaque)
					{
						Mask[N++] = FMask{ CurrentBlock, 1 };
					}
					else
					{
						Mask[N++] = FMask{ CompareBlock, -1 };
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			for (int j = 0; j < Axis2Limit; ++j)
			{
				for (int i = 0; i < Axis1Limit;)
				{
					if (Mask[N].Normal != 0)
					{
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int width;


						for (width = 1; i + width < Axis1Limit && CompareMask(Mask[N + width], CurrentMask); ++width)
						{

						}

						int height;
						bool done = false;

						for (height = 1; j + height < Axis2Limit; ++height)
						{
							for (int k = 0; k < width; ++k)
							{
								if (CompareMask(Mask[N + k + height * Axis1Limit], CurrentMask)) continue;

								done = true;
								break;
							}

							if (done) break;
						}

						DeltaAxis1[Axis1] = width;
						DeltaAxis2[Axis2] = height;

						CreateQuad(CurrentMask, AxisMask, width, height,
							ChunkItr,
							ChunkItr + DeltaAxis1,
							ChunkItr + DeltaAxis2,
							ChunkItr + DeltaAxis1 + DeltaAxis2
						);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < height; ++l)
						{
							for (int k = 0; k < width; ++k)
							{
								Mask[N + k + l * Axis1Limit] = FMask{ EBlock::Null, 0 };
							}
						}

						i += width;
						N += width;
					}
					else
					{
						i++;
						N++;
					}
				}
			}
		}
	}
}

void AGreedyChunk::ApplyMesh() const
{
	Mesh->SetMaterial(0, VertexMaterial);
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), true);

}

void AGreedyChunk::CreateQuad(const FMask Mask, const FIntVector AxisMask, const int Width, const int Height, const FIntVector V1, const FIntVector V2, const FIntVector V3, const FIntVector V4)
{
	const auto Normal = FVector(AxisMask * Mask.Normal);

	MeshData.Vertices.Add(FVector(V1) * 100);
	MeshData.Vertices.Add(FVector(V2) * 100);
	MeshData.Vertices.Add(FVector(V3) * 100);
	MeshData.Vertices.Add(FVector(V4) * 100);

	MeshData.Triangles.Add(VertexCount);
	MeshData.Triangles.Add(VertexCount + 2 + Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 2 - Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 3);
	MeshData.Triangles.Add(VertexCount + 1 - Mask.Normal);
	MeshData.Triangles.Add(VertexCount + 1 + Mask.Normal);

	if (Normal.X == 1 || Normal.X == -1)
	{
		MeshData.UV0.Add(FVector2D(Width, Height));
		MeshData.UV0.Add(FVector2D(0, Height));
		MeshData.UV0.Add(FVector2D(Width, 0));
		MeshData.UV0.Add(FVector2D(0, 0));
	}
	else
	{
		MeshData.UV0.Add(FVector2D(Height, Width));
		MeshData.UV0.Add(FVector2D(Height, 0));
		MeshData.UV0.Add(FVector2D(0, Width));
		MeshData.UV0.Add(FVector2D(0, 0));
	}

	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);
	MeshData.Normals.Add(Normal);

	const auto Color = FColor(0, 0, 0, GetTextureIndex(Mask.Block, Normal));

	MeshData.Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	VertexCount += 4;

}

int AGreedyChunk::GetBlockIndex(int X, int Y, int Z) const
{
	return Z * Size.X * Size.Y + Y * Size.X + X;
}

EBlock AGreedyChunk::GetBlock(FIntVector Index) const
{
	if (Index.X >= Size.X || Index.Y >= Size.Y || Index.Z >= Size.Z || Index.X < 0 || Index.Y < 0 || Index.Z < 0)
	{
		return EBlock::Air;
	}
	return Blocks[GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

bool AGreedyChunk::CompareMask(FMask M1, FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

int AGreedyChunk::GetTextureIndex(EBlock Block, FVector Normal) const
{
	switch (Block)
	{
	case EBlock::Stone: return 3;
	case EBlock::Dirt: return 2;
	case EBlock::Grass: 
	{
		if (Normal == FVector::UpVector) return 0;
		if (Normal == FVector::DownVector) return 2;
		return 1;
	}
	case EBlock::Wood:
	{
		if (Normal == FVector::UpVector || Normal == FVector::DownVector) return 4;
		return 5;
	}
	case EBlock::Leaf: return 6;
	case EBlock::Bedrock: return 7;
	case EBlock::IronOre: return 8;
	default: return 255;
	}
}

