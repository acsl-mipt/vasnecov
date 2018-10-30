#include "vasnecovresourcemanager.h"
#include "technologist.h"
#include "vasnecovmesh.h"
#include "vasnecovtexture.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>

VasnecovResourceManager::VasnecovResourceManager()
    : meshes()
    , textures()
    , dirMeshes(Vasnecov::cfg_dirMeshes)
    , dirTextures(Vasnecov::cfg_dirTextures)
    , dirTexturesDPref(Vasnecov::cfg_dirTexturesDPref)
    , dirTexturesNPref(Vasnecov::cfg_dirTexturesNPref)
    , dirTexturesIPref(Vasnecov::cfg_dirTexturesIPref)
    , meshesForLoading()
    , texturesForLoading()
{
}

VasnecovResourceManager::~VasnecovResourceManager()
{
    for(std::map<QString, VasnecovMesh *>::iterator rit = meshes.begin();
        rit != meshes.end(); ++rit)
    {
        delete (rit->second);
        rit->second = nullptr;
    }
    for(std::map<QString, VasnecovTexture *>::iterator rit = textures.begin();
        rit != textures.end(); ++rit)
    {
        delete (rit->second);
        rit->second = nullptr;
    }
}

GLboolean VasnecovResourceManager::loadMeshFile(const QString& fileName)
{
    QString path = dirMeshes + fileName; // Путь файла с расширением
    QString fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_meshFormat))
    {
        if(!meshes.count(fileId))
        {
            VasnecovMesh *mesh = new VasnecovMesh(path, fileId);
            if(mesh->loadModel())
            {
                if(addMesh(mesh, fileId))
                {
                    return true;
                }
            }
            delete mesh;
            mesh = nullptr;
        }
    }
    return false;
}

GLboolean VasnecovResourceManager::loadTextureFile(const QString& fileName)
{
    // Поиск префикса типа текстуры в адресе
    Vasnecov::TextureTypes type(Vasnecov::TextureTypeUndefined);

    if(fileName.startsWith(dirTexturesDPref))
        type = Vasnecov::TextureTypeDiffuse;
    else if(fileName.startsWith(dirTexturesIPref))
        type = Vasnecov::TextureTypeInterface;
    else if(fileName.startsWith(dirTexturesNPref))
        type = Vasnecov::TextureTypeNormal;

    QString path = dirTextures + fileName; // Путь файла с расширением

    QString fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_textureFormat))
    {
        if(!textures.count(fileId)) // Данные
        {
            QImage image(path);

            if(!image.isNull())
            {
                // Проверка на соотношение сторон (чудо-алгоритм от Мастана)
                if((image.width() & (image.width() - 1)) == 0 && (image.height() & (image.height() - 1)) == 0)
                {
                    VasnecovTexture *texture(nullptr);

                    switch(type)
                    {
                        case Vasnecov::TextureTypeDiffuse:
                            texture = new VasnecovTextureDiffuse(image);
                            break;
                        case Vasnecov::TextureTypeInterface:
                            texture = new VasnecovTextureInterface(image);
                            break;
                        case Vasnecov::TextureTypeNormal:
                            texture = new VasnecovTextureNormal(image);
                            break;
                        default:
                            Vasnecov::problem("Тип текстуры указан неверно: ", path);
                            return false;
                    }

                    if(addTexture(texture, fileId))
                    {
                        return true;
                    }

                    delete texture;
                    texture = nullptr;
                }
                else
                {
                    Vasnecov::problem("Текстура неверного размера: ", path);
                }
            }
        }
    }
    return false;
}

GLboolean VasnecovResourceManager::setDirectory(const QString& newDir, QString& oldDir)
{
    if(!newDir.isEmpty())
    {
        QDir qdir(newDir);
        if(qdir.exists())
        {
            QString forNewDir = qdir.path();
            forNewDir += "/"; // Для дальнейшей подстановки в адреса
            if(forNewDir != oldDir)
            {
                oldDir = forNewDir;
                return true;
            }
        }
    }
    return false;
}

GLboolean VasnecovResourceManager::correctPath(QString& path, QString& fileId, const QString& format)
{
    if(!format.isEmpty())
    {
        // Только файлы нужного расширения
        if(fileId.endsWith("." + format, Qt::CaseInsensitive))
            // Отрезать расширение
            fileId.remove(fileId.size() - 2 - format.size(), format.size() + 1);
        else
            path = path + "." + format;
    }

    QFile file(path);
    if(file.exists())
    {
        return true;
    }

    return false;

}

QString VasnecovResourceManager::correctFileId(const QString& fileId, const QString& format)
{
    QString res(fileId);

    if(!format.isEmpty())
    {
        // Отрезать расширение
        if(res.endsWith("." + format, Qt::CaseInsensitive))
            res.remove(fileId.size() - 2 - format.size(), format.size() + 1);
    }
    return res;
}

GLboolean VasnecovResourceManager::addTexture(VasnecovTexture* texture, const QString& fileId)
{
    if(texture)
    {
        GLboolean added(true);

        if(textures.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            textures[fileId] = texture;
            texturesForLoading.push_back(texture);
            raw_data.setUpdateFlag(Textures);
        }

        return added;
    }
    Vasnecov::problem("Неверная текстура либо дублирование данных");
    return false;
}

GLboolean VasnecovResourceManager::addMesh(VasnecovMesh* mesh, const QString& fileId)
{
    if(mesh)
    {
        GLboolean added(true);

        if(meshes.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            meshes[fileId] = mesh;
//			meshesForLoading.push_back(texture);
//			raw_data.setUpdateFlag(Meshes);
        }

        return added;
    }
    Vasnecov::problem("Неверный меш либо дублирование данных");
    return false;
}

GLuint VasnecovResourceManager::handleFilesInDir(const QString& dirPref, const QString& targetDir, const QString& format, GLboolean (VasnecovResourceManager::*workFun)(const QString&), GLboolean withSub)
{
    GLuint res(0);

    QString dotFormat = "." + format;

    QDir dir(dirPref + targetDir);

    QDirIterator::IteratorFlag flag(QDirIterator::NoIteratorFlags);
    if(withSub)
    {
        flag = QDirIterator::Subdirectories;
    }

    QDirIterator iterator(dir.path(), flag);

    while(iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileInfo().isDir())
        {
            QString fileName = iterator.fileName();

            if(fileName.endsWith(dotFormat))
            {
                QString fullFileName = iterator.filePath();
                fullFileName.remove(0, dirPref.size());

                res += (this->*workFun)(fullFileName);
            }
        }
    }
    return res;
}
