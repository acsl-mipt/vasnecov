// Сцена для вывода OpenGL (для связки View-Scene)
#ifndef VASNECOV_SCENE_H
#define VASNECOV_SCENE_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include <QGraphicsScene>
#include <GL/gl.h>

#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

class VasnecovUniverse;
class VasnecovWorld;

class VasnecovLamp;
class VasnecovProduct;
class VasnecovFigure;
class VasnecovLabel;
class VasnecovMaterial;
class VasnecovTexture;

class VasnecovScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit VasnecovScene(QObject *parent = 0);
    VasnecovUniverse *universe() const;

public slots:
    virtual void setUniverse(VasnecovUniverse *universe);
    virtual bool removeUniverse();

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &);
    GLsizei windowWidth()  {return m_width;}
    GLsizei windowHeight() {return m_height;}

private:
    // Геометрия окна
    GLsizei m_width;
    GLsizei m_height;

    VasnecovUniverse *m_universe;

    Q_DISABLE_COPY(VasnecovScene)
};


inline VasnecovUniverse *VasnecovScene::universe() const
{
    return m_universe;
}
inline bool VasnecovScene::removeUniverse()
{
    if(m_universe)
    {
        m_universe = 0;
        return true;
    }
    else
    {
        return false;
    }
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_SCENE_H
