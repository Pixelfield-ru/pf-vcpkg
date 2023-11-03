//
// Created by stuka on 07.05.2023.
//

#include "ModelAsset.h"

#include "SGCore/Main/CoreSettings.h"
#include "SGCore/Main/CoreMain.h"
#include "SGCore/Utils/AssimpUtils.h"

size_t polygonsNumber = 0;

std::shared_ptr<Core::Memory::Assets::IAsset> Core::Memory::Assets::ModelAsset::load(const std::string& path)
{
    m_path = path;

    m_importerFlags = Main::CoreSettings::ModelsImport::IMPORTER_FLAGS;

    Assimp::Importer importer;

    // TODO: maybe shared_ptr
    const aiScene* aiImportedScene(importer.ReadFile(m_path.string(), m_importerFlags));

    if(!aiImportedScene || aiImportedScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiImportedScene->mRootNode)
    {
        SGC_ERROR("Assimp error (while importing scene): " + std::string(importer.GetErrorString()));
        return shared_from_this();
    }

    m_name = aiImportedScene->mName.data;

    m_nodes.push_back(processNode(aiImportedScene->mRootNode, aiImportedScene));

    SGC_SUCCESS("Loaded model '" + m_name + "'. Nodes count: " + std::to_string(m_nodes.size()));

    return shared_from_this();
}

std::shared_ptr<Core::ImportedScene::Node> Core::Memory::Assets::ModelAsset::processNode(const aiNode* aiNode, const aiScene* aiScene)
{
    std::shared_ptr<ImportedScene::Node> sgNode = std::make_shared<ImportedScene::Node>();
    sgNode->m_name = aiNode->mName.data;

    aiVector3D position;
    aiQuaternion rotationQ;
    aiVector3D scale;
    aiNode->mTransformation.Decompose(scale, rotationQ, position);

    sgNode->m_position = { position.x, position.y, position.z };
    sgNode->m_rotationQuaternion = { rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z };
    sgNode->m_scale = { scale.x, scale.y, scale.z };

    // process all meshes in node
    for(unsigned int i = 0; i < aiNode->mNumMeshes; i++)
    {
        aiMesh *mesh = aiScene->mMeshes[aiNode->mMeshes[i]];
        sgNode->m_meshes.push_back(processMesh(mesh, aiScene));
    }

    // and go to the next node
    for(unsigned int i = 0; i < aiNode->mNumChildren; i++)
    {
        sgNode->m_children.push_back(processNode(aiNode->mChildren[i], aiScene));
    }

    return sgNode;
}

std::shared_ptr<Core::ImportedScene::IMesh> Core::Memory::Assets::ModelAsset::processMesh(const aiMesh* aiMesh, const aiScene* aiScene)
{
    std::shared_ptr<ImportedScene::IMesh> sgMesh(Core::Main::CoreMain::getRenderer().createMesh());
    sgMesh->m_positions.reserve(aiMesh->mNumVertices * 3);
    sgMesh->m_normals.reserve(aiMesh->mNumVertices * 3);
    sgMesh->m_tangents.reserve(aiMesh->mNumVertices * 3);
    sgMesh->m_bitangents.reserve(aiMesh->mNumVertices * 3);
    // TODO: make reserve for all texture coordinates
    sgMesh->m_uv.reserve(aiMesh->mNumVertices * 3);

    sgMesh->m_name = aiMesh->mName.data;

    polygonsNumber += aiMesh->mNumVertices / 3;
    std::cout << "current polygons num: " << std::to_string(polygonsNumber) << std::endl;

    for(unsigned i = 0; i < aiMesh->mNumVertices; i++)
    {
        sgMesh->m_positions.push_back(aiMesh->mVertices[i].x);
        sgMesh->m_positions.push_back(aiMesh->mVertices[i].y);
        sgMesh->m_positions.push_back(aiMesh->mVertices[i].z);

        sgMesh->m_normals.push_back(aiMesh->mNormals[i].x);
        sgMesh->m_normals.push_back(aiMesh->mNormals[i].y);
        sgMesh->m_normals.push_back(aiMesh->mNormals[i].z);

        if(aiMesh->mTangents)
        {
            sgMesh->m_tangents.push_back(aiMesh->mTangents[i].x);
            sgMesh->m_tangents.push_back(aiMesh->mTangents[i].y);
            sgMesh->m_tangents.push_back(aiMesh->mTangents[i].z);
        }

        if(aiMesh->mBitangents)
        {
            sgMesh->m_bitangents.push_back(aiMesh->mBitangents[i].x);
            sgMesh->m_bitangents.push_back(aiMesh->mBitangents[i].y);
            sgMesh->m_bitangents.push_back(aiMesh->mBitangents[i].z);
        }

        // if mesh has texture coordinates
        // TODO: make process all texture coordinates
        if(aiMesh->mTextureCoords[0])
        {
            sgMesh->m_uv.push_back(aiMesh->mTextureCoords[0][i].x);
            sgMesh->m_uv.push_back(aiMesh->mTextureCoords[0][i].y);
            sgMesh->m_uv.push_back(aiMesh->mTextureCoords[0][i].z);
        }
        else
        {
            sgMesh->m_uv.push_back(0.0f);
            sgMesh->m_uv.push_back(0.0f);
            sgMesh->m_uv.push_back(0.0f);
        }
    }

    for(unsigned i = 0; i < aiMesh->mNumFaces; i++)
    {
        const auto& face = aiMesh->mFaces[i];
        for(unsigned j = 0; j < face.mNumIndices; j++)
        {
            sgMesh->m_indices.push_back(face.mIndices[j]);
        }
    }

    // if has material
    if(aiMesh->mMaterialIndex >= 0)
    {
        // get current mesh material
        auto* aiMat = aiScene->mMaterials[aiMesh->mMaterialIndex];

        aiColor4D diffuseColor;
        aiColor4D specularColor;
        aiColor4D ambientColor;
        aiColor4D emissionColor;
        float shininess;
        float metallic;
        float roughness;

        if(aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor) == AI_SUCCESS)
        {
            sgMesh->m_material->m_diffuseColor = Utils::AssimpUtils::aiVectorToGLM(diffuseColor);
        }

        if(aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_SPECULAR, &specularColor) == AI_SUCCESS)
        {
            sgMesh->m_material->m_specularColor = Utils::AssimpUtils::aiVectorToGLM(specularColor);
        }

        if(aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == AI_SUCCESS)
        {
            sgMesh->m_material->m_ambientColor = Utils::AssimpUtils::aiVectorToGLM(ambientColor);
        }

        if(aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_EMISSIVE, &emissionColor) == AI_SUCCESS)
        {
            sgMesh->m_material->m_emissionColor = Utils::AssimpUtils::aiVectorToGLM(emissionColor);
        }

        if(aiGetMaterialFloat(aiMat, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS)
        {
            sgMesh->m_material->m_shininess = shininess;
        }

        if(aiGetMaterialFloat(aiMat, AI_MATKEY_METALLIC_FACTOR, &metallic) == AI_SUCCESS)
        {
            sgMesh->m_material->m_metallicFactor = metallic;
        }

        if(aiGetMaterialFloat(aiMat, AI_MATKEY_ROUGHNESS_FACTOR, &roughness) == AI_SUCCESS)
        {
            sgMesh->m_material->m_roughnessFactor = roughness;
        }

        sgMesh->m_material->m_name = aiMat->GetName().data;

        loadTextures(aiMat, sgMesh->m_material, aiTextureType_EMISSIVE, SGTextureType::SGTP_EMISSIVE);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_AMBIENT_OCCLUSION, SGTextureType::SGTP_AMBIENT_OCCLUSION);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_AMBIENT, SGTextureType::SGTP_AMBIENT);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_DIFFUSE_ROUGHNESS, SGTextureType::SGTP_DIFFUSE_ROUGHNESS);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_DIFFUSE, SGTextureType::SGTP_DIFFUSE);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_DISPLACEMENT, SGTextureType::SGTP_DISPLACEMENT);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_HEIGHT, SGTextureType::SGTP_HEIGHT);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_NORMALS, SGTextureType::SGTP_NORMALS);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_BASE_COLOR, SGTextureType::SGTP_BASE_COLOR);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_CLEARCOAT, SGTextureType::SGTP_CLEARCOAT);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_EMISSION_COLOR, SGTextureType::SGTP_EMISSION_COLOR);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_LIGHTMAP, SGTextureType::SGTP_LIGHTMAP);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_METALNESS, SGTextureType::SGTP_METALNESS);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_NORMAL_CAMERA, SGTextureType::SGTP_NORMAL_CAMERA);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_OPACITY, SGTextureType::SGTP_OPACITY);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_REFLECTION, SGTextureType::SGTP_REFLECTION);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_SHEEN, SGTextureType::SGTP_SHEEN);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_SHININESS, SGTextureType::SGTP_SHININESS);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_SPECULAR, SGTextureType::SGTP_SPECULAR);
        loadTextures(aiMat, sgMesh->m_material, aiTextureType_TRANSMISSION, SGTextureType::SGTP_TRANSMISSION);

        // SGC_SUCCESS("Loaded material '" + sgMesh->m_material->m_name + "'");
    }

    sgMesh->prepare();

    // SGC_SUCCESS("Loaded mesh '" + sgMesh->m_name + "'. Vertices count: " + std::to_string(sgMesh->m_positions.size()) + ", indices count: " + std::to_string(sgMesh->m_indices.size()));

    return sgMesh;
}

void Core::Memory::Assets::ModelAsset::loadTextures
(aiMaterial* aiMat, std::shared_ptr<IMaterial>& sgMaterial, const aiTextureType& aiTexType, const SGTextureType& sgMaterialTextureType)
{
    for(unsigned i = 0; i < aiMat->GetTextureCount(aiTexType); i++)
    {
        aiString texturePath;
        aiMat->GetTexture(aiTexType, i, &texturePath);

        // final path is model directory file + separator + relative texture path
        const std::string finalPath = m_path.parent_path().string() + "/" + texturePath.data;

        sgMaterial->findAndAddTexture2D(sgMaterialTextureType, finalPath);

        //SGC_SUCCESS("Loaded material`s '" + std::string(aiMat->GetName().data) + "' texture. Raw type name: '" +
        //                    sgMaterialTextureTypeToString(sgMaterialTextureType) + "', path: " + std::string(finalPath));
    }
}
