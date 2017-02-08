// Сцена для вывода OpenGL (для связки View-Scene)
#ifndef VASNECOV_SCENE_H
#define VASNECOV_SCENE_H

#pragma GCC diagnostic ignored "-Weffc++"
#include <QGraphicsScene>
#include "vasnecovuniverse.h"

#pragma GCC diagnostic warning "-Weffc++"

class VasnecovScene : public QGraphicsScene
{
	Q_OBJECT

public:
	explicit VasnecovScene(QObject *parent = 0);
	virtual void drawBackground(QPainter *painter, const QRectF &);
	VasnecovUniverse *universe() const;
	
signals:
	
public slots:
	virtual void setUniverse(VasnecovUniverse * universe);
	virtual bool removeUniverse();

protected:
	// Геометрия окна
	GLsizei m_width;
	GLsizei m_height;

	VasnecovUniverse *m_universe;

private:
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

#pragma GCC diagnostic ignored "-Weffc++"
#endif // VASNECOV_SCENE_H
