#include "Technologist.h"
#include "VasnecovMesh.h"
#include "VasnecovResourceManager.h"
#include "VasnecovTexture.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>

VasnecovResourceManager::VasnecovResourceManager()
    : _meshes()
    , _textures()
    , _dirMeshes(Vasnecov::cfg_dirMeshes)
    , _dirTextures(Vasnecov::cfg_dirTextures)
    , _dirTexturesDPref(Vasnecov::cfg_dirTexturesDPref)
    , _dirTexturesNPref(Vasnecov::cfg_dirTexturesNPref)
    , _dirTexturesIPref(Vasnecov::cfg_dirTexturesIPref)
    , _meshesForLoading()
    , _texturesForLoading()
{
}

VasnecovResourceManager::~VasnecovResourceManager()
{
    for(std::map<QString, VasnecovMesh *>::iterator rit = _meshes.begin();
        rit != _meshes.end(); ++rit)
    {
        delete (rit->second);
        rit->second = nullptr;
    }
    for(std::map<QString, VasnecovTexture *>::iterator rit = _textures.begin();
        rit != _textures.end(); ++rit)
    {
        delete (rit->second);
        rit->second = nullptr;
    }
}

GLboolean VasnecovResourceManager::setTexturesDir(const QString& dir)
{
    return setDirectory(dir, _dirTextures);
}

GLboolean VasnecovResourceManager::setMeshesDir(const QString& dir)
{
    return setDirectory(dir, _dirMeshes);
}

GLboolean VasnecovResourceManager::loadMeshFile(const QString& fileName)
{
    QString path = _dirMeshes + fileName; // Путь файла с расширением
    QString fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_meshFormat))
    {
        if(!_meshes.count(fileId))
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

GLboolean VasnecovResourceManager::loadMeshFileByPath(const QString& filePath)
{
    if(filePath.isEmpty())
        return false;

    if(_meshes.count(filePath))
        return false;

    VasnecovMesh *mesh = new VasnecovMesh(filePath, filePath);
    bool loaded(false);

    if(filePath.endsWith(QString(".%1").arg(Vasnecov::cfg_meshFormat)))
        loaded = mesh->loadModel();
    else if(filePath.endsWith(QString(".%1").arg(Vasnecov::cfg_rawMeshFormat)))
        loaded = mesh->loadRawModel();

    if(loaded)
    {
        if(addMesh(mesh, filePath))
            return true;
    }

    delete mesh;
    mesh = nullptr;
    return false;
}

GLboolean VasnecovResourceManager::loadTextureFile(const QString& fileName)
{
    // Поиск префикса типа текстуры в адресе
    Vasnecov::TextureTypes type(Vasnecov::TextureTypeUndefined);

    if(fileName.startsWith(_dirTexturesDPref))
        type = Vasnecov::TextureTypeDiffuse;
    else if(fileName.startsWith(_dirTexturesIPref))
        type = Vasnecov::TextureTypeInterface;
    else if(fileName.startsWith(_dirTexturesNPref))
        type = Vasnecov::TextureTypeNormal;

    QString path = _dirTextures + fileName; // Путь файла с расширением

    QString fileId = fileName;

    if(correctPath(path, fileId, Vasnecov::cfg_textureFormat))
    {
        if(_textures.find(fileId) == _textures.end())
            return createTexture(path, type, fileId);
    }
    return false;
}

GLboolean VasnecovResourceManager::loadTextureFileByPath(const QString& filePath, Vasnecov::TextureTypes type)
{
    QFile file(filePath);
    if(!file.exists())
        return false;

    if(_textures.find(filePath) != _textures.end())
        return false;

    if(!filePath.endsWith(QString(".%1").arg(Vasnecov::cfg_textureFormat)))
        return false;

    return createTexture(filePath, type);
}

size_t VasnecovResourceManager::meshesAmount() const
{
    return _meshes.size();
}

size_t VasnecovResourceManager::texturesAmount() const
{
    return _textures.size();
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

GLboolean VasnecovResourceManager::createTexture(const QString& path, Vasnecov::TextureTypes type, const QString& name)
{
    QImage image(path);

    if(image.isNull())
        return false;

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
                Vasnecov::problem("Incorrent texture type: ", path);
                return false;
        }

        if(addTexture(texture, name.isEmpty() ? path : name))
        {
            return true;
        }

        delete texture;
        texture = nullptr;
    }
    else
    {
        Vasnecov::problem("Incorrect texture size: ", path);
    }

    return false;
}

GLboolean VasnecovResourceManager::addTexture(VasnecovTexture* texture, const QString& fileId)
{
    if(texture)
    {
        GLboolean added(true);

        if(_textures.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            _textures[fileId] = texture;
            _texturesForLoading.push_back(texture);
            raw_data.setUpdateFlag(Textures);
        }

        return added;
    }
    Vasnecov::problem("Incorrect texture or data duplicating");
    return false;
}

GLboolean VasnecovResourceManager::addMesh(VasnecovMesh* mesh, const QString& fileId)
{
    if(mesh)
    {
        GLboolean added(true);

        if(_meshes.count(fileId))
        {
            added = false;
        }
        if(added)
        {
            _meshes[fileId] = mesh;
//			meshesForLoading.push_back(texture);
//			raw_data.setUpdateFlag(Meshes);
        }

        return added;
    }
    Vasnecov::problem("Incorrect mesh or data duplicating");
    return false;
}

VasnecovMesh*VasnecovResourceManager::designerFindMesh(const QString& name)
{
    if(_meshes.find(name) == _meshes.end())
        return nullptr;

    return _meshes[name];
}

VasnecovTexture*VasnecovResourceManager::designerFindTexture(const QString& name)
{
    if(_textures.find(name) == _textures.end())
        return nullptr;

    return _textures[name];
}

bool VasnecovResourceManager::handleMeshesDir(const QString& dirName, GLboolean withSub)
{
    return handleFilesInDir(_dirMeshes, dirName, Vasnecov::cfg_meshFormat, &VasnecovResourceManager::loadMeshFile, withSub);
}

bool VasnecovResourceManager::handleTexturesDir(const QString& dirName, GLboolean withSub)
{
    GLuint res(0);

    res  = handleFilesInDir(_dirTextures, _dirTexturesDPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovResourceManager::loadTextureFile, withSub);
    res += handleFilesInDir(_dirTextures, _dirTexturesIPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovResourceManager::loadTextureFile, withSub);
    res += handleFilesInDir(_dirTextures, _dirTexturesNPref + dirName, Vasnecov::cfg_textureFormat, &VasnecovResourceManager::loadTextureFile, withSub);

    return res;
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

bool VasnecovResourceManager::renderUpdate()
{
    bool wasUpdated(false);

    if(raw_data.wasUpdated)
    {
        // Загрузка (догрузка) ресурсов
        // Всё это делается с захваченным мьютексом, поэтому при больших загрузках стоять будет всё
        if(raw_data.isUpdateFlag(Meshes))
        {
            if(!_meshesForLoading.empty())
            {
//				for(std::vector<VasnecovMesh *>::iterator mit = raw_data.meshesForLoading.begin();
//					mit != raw_data.meshesForLoading.end(); ++mit)
//				{

//				}
            }
        }
        if(raw_data.isUpdateFlag(Textures))
        {
            if(!_texturesForLoading.empty())
            {
                for(std::vector<VasnecovTexture *>::iterator tit = _texturesForLoading.begin();
                    tit != _texturesForLoading.end(); ++tit)
                {
                    if(!(*tit)->loadImage())
                    {
                        // TODO: remove wrong textures from raw_data.textures and all objects
                    }
                }
                _texturesForLoading.clear();
            }
        }

        wasUpdated = true;
    }

    return wasUpdated;
}
