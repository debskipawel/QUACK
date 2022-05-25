#include "modelLoader.h"
#include "inputLayoutManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace DirectX;
using namespace Assimp;
using namespace mini;
using namespace gk2;

static constexpr unsigned int ImportFlags = aiProcess_ConvertToLeftHanded |
	aiProcess_SortByPType | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_RemoveComponent |
	aiProcess_GenUVCoords | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;

static constexpr unsigned  int RemoveComponentFlags = aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS |
	aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_MATERIALS |
	aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_TEXTURES;

static constexpr unsigned int RemovePrimitiveFlags = aiPrimitiveType_POINT | aiPrimitiveType_LINE;

static constexpr DXGI_FORMAT ComponentFormats[4] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };

static constexpr D3D11_INPUT_ELEMENT_DESC PositionElement{ "POSITION", 0, ComponentFormats[2], 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
static constexpr D3D11_INPUT_ELEMENT_DESC NormalElement{ "NORMAL", 0, ComponentFormats[2], 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };


static constexpr D3D11_INPUT_ELEMENT_DESC TexCoordElement(unsigned int index, unsigned int components)
{
	return{ "TEXCOORD", index, ComponentFormats[components-1], 2 + index, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
}

static inline unsigned getImportFlags(bool smoothNormals)
{
	return ImportFlags | (smoothNormals ? aiProcess_GenSmoothNormals : aiProcess_GenNormals);
}

void ModelLoader::initLoader(Importer& importer)
{
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, RemoveComponentFlags);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, RemovePrimitiveFlags);
}

const aiScene* ModelLoader::readFromFile(const string& filename, Importer& importer, bool smoothNormals)
{
	return importer.ReadFile(filename, getImportFlags(smoothNormals));
}

const aiScene* ModelLoader::readFromMemory(const string& buffer, Importer& importer, bool smoothNormals)
{
	return importer.ReadFileFromMemory(buffer.data(), buffer.size(), getImportFlags(smoothNormals), "nff");
}

Model ModelLoader::convertToModel(const aiScene* scene, InputLayoutManager& layouts)
{
	if (!scene)
		return Model{};
	assert(scene->HasMeshes());
	vector<Mesh> meshes;
	vector<size_t> meshSignatures;
	meshes.reserve(scene->mNumMeshes);
	meshSignatures.reserve(scene->mNumMeshes);
	for (auto ppMesh = scene->mMeshes; ppMesh < scene->mMeshes + scene->mNumMeshes; ++ppMesh)
	{
		auto pAIMesh = *ppMesh;
		assert(pAIMesh->HasFaces() && pAIMesh->HasPositions() && pAIMesh->HasNormals() && pAIMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
		dx_ptr_vector<ID3D11Buffer> vertexBuffers;
		vector<D3D11_INPUT_ELEMENT_DESC> bufferElements;
		vertexBuffers.push_back(m_device.CreateVertexBuffer(pAIMesh->mVertices, pAIMesh->mNumVertices));
		bufferElements.push_back(PositionElement);
		vertexBuffers.push_back(m_device.CreateVertexBuffer(pAIMesh->mNormals, pAIMesh->mNumVertices));
		bufferElements.push_back(NormalElement);
		vector<unsigned> vbStrides(2, sizeof(aiVector3D));
		for (unsigned int texCoordIdx = 0; pAIMesh->HasTextureCoords(texCoordIdx); ++texCoordIdx)
		{
			unsigned texComponenets = pAIMesh->mNumUVComponents[texCoordIdx];
			if (texComponenets == 3)
			{
				vertexBuffers.push_back(m_device.CreateVertexBuffer(pAIMesh->mTextureCoords[texCoordIdx], pAIMesh->mNumVertices));
			}
			else
			{
				vector<float> texData;
				texData.reserve(texComponenets * pAIMesh->mNumVertices);
				for (unsigned vertIdx = 0; vertIdx < pAIMesh->mNumVertices; ++vertIdx)
					for (unsigned texCompIdx = 0; texCompIdx < texComponenets; ++texCompIdx)
						texData.push_back(pAIMesh->mTextureCoords[texCoordIdx][vertIdx][texCompIdx]);
				vertexBuffers.push_back(m_device.CreateVertexBuffer(texData));

			}
			bufferElements.push_back(TexCoordElement(texCoordIdx, pAIMesh->mNumUVComponents[texCoordIdx]));
			vbStrides.push_back(texComponenets * sizeof(float));
		}
		vector<unsigned short> indices;
		indices.reserve(3 * pAIMesh->mNumFaces);
		for (unsigned int fIdx = 0; fIdx < pAIMesh->mNumFaces; ++fIdx)
		{
			aiFace& face = pAIMesh->mFaces[fIdx];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		meshes.emplace_back(move(vertexBuffers), std::move(vbStrides), m_device.CreateIndexBuffer(indices),
			static_cast<unsigned int>(indices.size()));
		meshSignatures.push_back(layouts.registerVertexAttributesID(move(bufferElements)));
	}
	vector<ModelNode> nodes;
	//there should be at least as many nodes as there are meshes, so that number is a good first approximation
	//of the number of nodes
	nodes.reserve(scene->mNumMeshes);
	addNode(nodes, scene->mRootNode);
	return Model(move(meshes), move(meshSignatures), move(nodes));
}

Model ModelLoader::LoadFromFile(const string& filename, InputLayoutManager& layouts, bool smoothNormals)
{
	Importer importer;
	initLoader(importer);
	return convertToModel(readFromFile(filename, importer, smoothNormals), layouts);
}

Model ModelLoader::LoadFromString(const string& modelDescription, InputLayoutManager& layouts, bool smoothNormals)
{
	Importer importer;
	initLoader(importer);
	return convertToModel(readFromMemory(modelDescription, importer, smoothNormals), layouts);
}

int* ModelLoader::addNode(vector<ModelNode>& nodes, aiNode* pAINode)
{
	nodes.emplace_back();
	memcpy(&nodes.back().localTransform, &pAINode->mTransformation.Transpose(), 16 * sizeof(float));
	int* myNext = &nodes.back().nextIndex;
	if (pAINode->mNumMeshes == 1)
		nodes.back().meshIndex = pAINode->mMeshes[0];
	int* pChildIndex = &nodes.back().childIndex;
	for (unsigned int i = 0; i < pAINode->mNumChildren; ++i)
	{
		*pChildIndex = static_cast<int>(nodes.size());
		pChildIndex = addNode(nodes, pAINode->mChildren[i]);
	}
	if(pAINode->mNumMeshes > 1)
	{
		for (unsigned int i = 0; i < pAINode->mNumMeshes; ++i)
		{
			*pChildIndex = static_cast<int>(nodes.size());
			nodes.emplace_back();
			nodes.back().meshIndex = pAINode->mMeshes[i];
			pChildIndex = &nodes.back().nextIndex;
		}
	}
	return myNext;
}
