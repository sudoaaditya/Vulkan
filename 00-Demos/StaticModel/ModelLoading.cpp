#include "vk.h"
#include "ModelLoading.h"


Mesh::Mesh(std::vector<float> vertices, std::vector<uint32_t> indices)
{
	this->m_Vertices = vertices;
	this->m_Indices = indices;
}


BOOLEAN StaticModel::LoadModel(std::string sPath)
{

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(sPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);


	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		fprintf(fptr, "StaticModel::LoadModel:: importer.ReadFile() Failed with %s\n", importer.GetErrorString());
		return FALSE;
	}

	ProcessNode(scene->mRootNode, scene);

	return TRUE;
}

void StaticModel::ProcessNode(aiNode* pNode, const aiScene* pScene)
{
	for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
	{
		aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
		this->m_Mesh.push_back(ProcessMesh(mesh, pScene));
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		ProcessNode(pNode->mChildren[i], pScene);
	}
}

Mesh StaticModel::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
{
	std::vector<uint32_t> indices;
	std::vector<float> vertices;

	for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
	{
		float x, y, z;

		x = pMesh->mVertices[i].x;
		y = pMesh->mVertices[i].y;
		z = pMesh->mVertices[i].z;

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
	}

	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		aiFace face = pMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return Mesh(vertices, indices);
}


