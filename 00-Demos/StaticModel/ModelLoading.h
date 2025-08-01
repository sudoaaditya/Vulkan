#pragma once

#ifndef _MODEL_LOADING_H_
#define _MODEL_LOADING_H_

class Mesh
{
    public:
    std::vector<float> m_Vertices;
    std::vector<uint32_t> m_Indices;
    Mesh(std::vector<float> vertices, std::vector<uint32_t> indices);

    void *GetPosData()
    {
        return &m_Vertices[0];
    }

    size_t GetPosDataSize()
    {
        return sizeof(float) * m_Vertices.size();
    }

    size_t GetPosCount()
    {
        return m_Vertices.size();
    }

    void *GetIndicesData()
    {
        return &m_Indices[0];
    }

    size_t GetIndicesDataSize()
    {
        return sizeof(uint32_t) * m_Indices.size();
    }

    size_t GetIndicesCount()
    {
        return m_Indices.size();
    }

    void Uninitialize(void)
    {
        m_Vertices.clear();
        m_Indices.clear();
    }
};

class StaticModel
{

public:
    std::vector<Mesh> m_Mesh;

private:
    void ProcessNode(aiNode *pNode, const aiScene *pScene);
    Mesh ProcessMesh(aiMesh *pMesh, const aiScene *pScene);

public:
    BOOLEAN LoadModel(std::string path);
};

#endif