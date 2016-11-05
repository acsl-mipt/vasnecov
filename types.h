#ifndef VASNECOV_TYPES_H
#define VASNECOV_TYPES_H

#pragma GCC diagnostic ignored "-Weffc++"
#include <GL/gl.h>
#include <string>
#include "vasnecovmatrix4x4.h"
#pragma GCC diagnostic warning "-Weffc++"

typedef std::string GLstring;
typedef VasnecovMatrix4x4 GLmatrix;

const float M_2PI(M_PI*2);

const float c_radToDeg = 180/M_PI; // Радианы в градусы
const float c_degToRad = M_PI/180; // Градусы в радианы

class VasnecovElement;
class VasnecovMaterial;
class VasnecovMesh;
class VasnecovProduct;
class VasnecovFigure;
class QMutex;

namespace Vasnecov
{
	enum WorldTypes
	{
		WorldTypePerspective = 1, // Перспективный тип проекции
		WorldTypeOrthographic  = 2 // Ортогональный тип проекции
	};

	enum PolygonDrawingTypes
	{
		PolygonDrawingTypeNormal = GL_FILL,
		PolygonDrawingTypeLines = GL_LINE,
		PolygonDrawingTypePoints = GL_POINT
	};
	// Характеристики вида
	struct WorldParameters
	{
		Vasnecov::WorldTypes projection; // Тип проекции (орто/перспектива)
		GLint x, y; // координаты мира (окна просмотра) в плоскости экрана
		GLsizei width, height;	// ширина, высота окна просмотра
		Vasnecov::PolygonDrawingTypes drawingType; // GL_FILL, GL_LINE, GL_POINT
		GLboolean depth; // Тест глубины
		GLboolean light;

		WorldParameters() :
			projection(WorldTypePerspective),
			x(0), y(0),
			width(320), height(280),
			drawingType(Vasnecov::PolygonDrawingTypeNormal),
			depth(true),
			light(true)
		{
		}
		bool operator!=(const WorldParameters& other) const
		{
			return projection != other.projection ||
				   x != other.x ||
				   y != other.y ||
				   width != other.width ||
				   height != other.height ||
				   drawingType != other.drawingType ||
				   depth != other.depth ||
				   light != other.light;
		}
		bool operator==(const WorldParameters& other) const
		{
			return projection == other.projection &&
				   x == other.x &&
				   y == other.y &&
				   width == other.width &&
				   height == other.height &&
				   drawingType == other.drawingType &&
				   depth == other.depth &&
				   light == other.light;
		}
	};
	struct Perspective
	{
		GLfloat angle; // Угол раствора
		GLfloat ratio; // Соотношение сторон
		GLfloat frontBorder; // Передняя граница
		GLfloat backBorder; // Задняя граница

		Perspective() :
			angle(35.0f),
			ratio(4/3),
			frontBorder(0.1f),
			backBorder(1000.0f)
		{
		}
		bool operator!=(const Perspective& other) const
		{
			return !qFuzzyCompare(*this, other);
		}
		bool operator==(const Perspective& other) const
		{
			return qFuzzyCompare(*this, other);
		}
		bool qFuzzyCompare(const Perspective& first, const Perspective& second) const
		{
			return ::qFuzzyCompare(first.angle, second.angle) &&
			       ::qFuzzyCompare(first.ratio, second.ratio) &&
			       ::qFuzzyCompare(first.frontBorder, second.frontBorder) &&
			       ::qFuzzyCompare(first.backBorder, second.backBorder);
		}
	};
	struct Ortho
	{
		GLfloat left, right;
		GLfloat bottom, top;
		GLfloat front, back;

		Ortho() :
			left(-1.0f), right(1.0f),
			bottom(-1.0f), top(1.0f),
			front(-1.0f), back(1.0f)
		{}
		bool operator!=(const Ortho& other) const
		{
			return !qFuzzyCompare(*this, other);
		}
		bool operator==(const Ortho& other) const
		{
			return qFuzzyCompare(*this, other);
		}
		bool qFuzzyCompare(const Ortho& first, const Ortho& second) const
		{
			return ::qFuzzyCompare(first.left, second.left) &&
			       ::qFuzzyCompare(first.right, second.right) &&
			       ::qFuzzyCompare(first.bottom, second.bottom) &&
			       ::qFuzzyCompare(first.top, second.top) &&
			       ::qFuzzyCompare(first.front, second.front) &&
			       ::qFuzzyCompare(first.back, second.back);
		}
	};
	enum TextureTypes
	{
		TextureTypeUndefined = 0,
		TextureTypeInterface = 1,
		TextureTypeDiffuse = 2,
		TextureTypeNormal = 3
	};

	struct Attributes
	{
		GLenum wasUpdated;

		Attributes() :
			wasUpdated(false)
		{
		}
		virtual ~Attributes(){}

		void setUpdateFlag(GLenum flag)
		{
			wasUpdated |= flag;
		}
		void clearUpdateFlag()
		{
			wasUpdated = 0;
		}
		GLboolean updateFlag() const
		{
			return wasUpdated;
		}
		GLboolean isUpdateFlag(GLenum flag) const
		{
			return (wasUpdated & flag) != 0;
		}
	};

	// Камера
	struct Camera
	{
		QVector3D position; // Позиция камеры в пространстве
		QVector3D target; // Позиция точки, на которую сфокусирована камера
		GLfloat roll;

		Camera() :
			position(0.0, 0.0, 1.85),
			target(),
			roll(0)
		{
		}
		bool operator!=(const Camera& other) const
		{
			return position != other.position ||
				   target != other.target ||
				   roll != other.roll;
		}
		bool operator==(const Camera& other) const
		{
			return position == other.position &&
				   target == other.target &&
				   roll == other.roll;
		}
	};
}
#pragma GCC diagnostic ignored "-Weffc++"
#endif // VASNECOV_TYPES_H
