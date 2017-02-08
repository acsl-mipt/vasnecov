// Класс описания трехмерных объектов для рендеринга

#ifndef VASNECOVMESH_H
#define VASNECOVMESH_H

#pragma GCC diagnostic ignored "-Weffc++"
#include <vector>
#include "configuration.h"
#pragma GCC diagnostic warning "-Weffc++"

class VasnecovPipeline;

class VasnecovMesh
{
	struct QuadsIndices
	{
		GLuint vertices[4];
		GLuint normals[4];
		GLuint textures[4];

		QuadsIndices() :
			vertices(),
			normals(),
			textures()
		{}
		void clear()
		{
			memset(this, 0, sizeof(*this));
		}
	};
	struct TrianglesIndices
	{
		GLuint vertices[3];
		GLuint normals[3];
		GLuint textures[3];

		TrianglesIndices() :
			vertices(),
			normals(),
			textures()
		{}
		void clear()
		{
			memset(this, 0, sizeof(*this));
		}
		TrianglesIndices &operator=(const QuadsIndices &value)
		{
			for(GLuint i = 0; i < 3; ++i)
			{
				vertices[i] = value.vertices[i];
				normals[i] = value.normals[i];
				textures[i] = value.textures[i];
			}

			return *this;
		}
	};

public:
	VasnecovMesh(const GLstring &meshPath, VasnecovPipeline *pipeline, const GLstring &name = "");

	void setName(GLstring name); // Задать имя меша (необязательный параметр)
	GLboolean loadModel(GLboolean readFromMTL = Vasnecov::cfg_readFromMTL);
	GLboolean loadModel(const GLstring &path, GLboolean readFromMTL = Vasnecov::cfg_readFromMTL); // Загрузка модели (obj-файл)
	void drawModel(); // Отрисовка модели
	QVector3D cm() const;
	void drawBorderBox(); // Рисовать ограничивающий бокс

protected:
	void optimizeData();
	void calculateBox();

protected:
	VasnecovPipeline *const m_pipeline;
	GLstring m_name; // Имя меша (то, что пишется в мап мешей). Необязательный атрибут, для текстур и т.п.
	GLboolean m_isHidden; // Флаг на отрисовку
	GLstring m_meshPath; // Адрес (относительно директории приложения) файла модели.
	GLboolean m_isLoaded;

	std::vector<GLuint> m_indices; // Индексы для отрисовки
	std::vector<QVector3D> m_vertices; // Координаты вершин
	std::vector<QVector3D> m_normals; // Координаты нормалей
	std::vector<QVector2D> m_textures; // Координаты текстур

	GLboolean m_hasTexture; // Флаг наличия внешней текстуры

	std::vector <QVector3D> m_borderBoxVertices; // Координаты ограничивающего бокса
	std::vector <GLuint> m_borderBoxIndices; // Индексы для ограничивающего бокса
	QVector3D m_cm; // Координата центра масс (по вершинам ограничивающей коробки)

private:
	Q_DISABLE_COPY(VasnecovMesh)

};

inline void VasnecovMesh :: setName(GLstring name)
{
	m_name = name;
}

inline QVector3D VasnecovMesh :: cm() const
{
	return m_cm;
}

#pragma GCC diagnostic ignored "-Weffc++"
#endif // VASNECOVMESH_H
