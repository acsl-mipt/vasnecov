#include "vasnecovworld.h"
#include "technologist.h"
#include "vasnecovmaterial.h"
#include "vasnecovproduct.h"
#include "vasnecovlabel.h"
#include "vasnecovfigure.h"
#include "configuration.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

/*!
 \brief

 \fn VasnecovWorld::VasnecovWorld
 \param mutex
 \param pipeline
 \param mx
 \param my
 \param width
 \param height
*/
VasnecovWorld::VasnecovWorld(QMutex *mutex,
							 VasnecovPipeline *pipeline,
							 GLint mx, GLint my,
							 GLsizei width, GLsizei height,
							 const GLstring &name) :
	Vasnecov::CoreObject(mutex, pipeline, name),
	m_parameters(raw_wasUpdated, Parameters),
	m_perspective(raw_wasUpdated, Perspective),
	m_ortho(raw_wasUpdated, Ortho),
	m_camera(raw_wasUpdated, Cameras),
	m_lightModel(),

	m_elements()
{
	m_parameters.editableRaw().x = mx;
	m_parameters.editableRaw().y = my;
	m_parameters.editableRaw().width = width;
	m_parameters.editableRaw().height = height;

	// Перспективная проекция
	m_perspective.editableRaw().ratio = static_cast<GLfloat>(width)/height;

	designerUpdateOrtho();

	renderUpdateData();
}
/*!
 \brief

 \fn VasnecovWorld::~VasnecovWorld
*/
VasnecovWorld::~VasnecovWorld()
{
}

/*!
 \brief Обновление данных для ортогональной проекции.

 Составляется матрица проектирования для ортогональной проекции, основанная на матрице перспективной
 проекции и положению камеры. Камера нужна для определения передней/задней ограничивающих плоскостей.
*/
void VasnecovWorld::designerUpdateOrtho()
{
	// Ортогональная проекция (по данным перспективной и положению камеры)
	GLfloat dist;
	dist = (m_camera.raw().target - m_camera.raw().position).length();

	if(dist == 0)
	{
		dist = 1.0;
	}

	Vasnecov::Ortho ortho;

	ortho.top = tan(m_perspective.raw().angle*c_degToRad*0.5)*dist;
	ortho.bottom = - ortho.top;
	ortho.left = m_perspective.raw().ratio*ortho.bottom;
	ortho.right = m_perspective.raw().ratio*ortho.top;
	ortho.front = -dist*100; // NOTE: it's stupidy, but i don't know how to make it correct
	ortho.back = (m_perspective.raw().backBorder+dist)*100;

	m_ortho.set(ortho);
}

/*!
 \brief

 \fn VasnecovWorld::setCameraAngles
 \param yaw
 \param pitch
*/
void VasnecovWorld::setCameraAngles(GLfloat yaw, GLfloat pitch)
{
	QMutexLocker locker(mtx_data);

	QVector3D vec(1.0, 0.0, 0.0); // Единичный вектор по направлению X

	QQuaternion qYaw = QQuaternion::fromAxisAndAngle(0, 0, 1, yaw);
	QQuaternion qPitch = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(QVector3D(0, 0, 1), vec).normalized(), pitch);

	vec = qYaw.rotatedVector(vec);
	vec = qPitch.rotatedVector(vec);

	vec *= (m_camera.raw().target - m_camera.raw().position).length(); // Увеличение на длину

	QVector3D target = vec + m_camera.raw().position;
	if(m_camera.raw().target != target)
	{
		m_camera.editableRaw().target = target;
		designerUpdateOrtho();
	}
}
/*!
 \brief

 \fn VasnecovWorld::setCameraAngles
 \param yaw
 \param pitch
 \param roll
*/
void VasnecovWorld::setCameraAngles(GLfloat yaw, GLfloat pitch, GLfloat roll)
{
	QMutexLocker locker(mtx_data);
	if(m_camera.raw().roll != roll)
	{
		m_camera.editableRaw().roll = roll;
	}
	locker.unlock();

	setCameraAngles(yaw, pitch);
}

void VasnecovWorld::flyCamera(const QVector3D &step)
{
	QMutexLocker locker(mtx_data);

	QVector3D forward = (m_camera.raw().target - m_camera.raw().position).normalized();

	bool updated(false);
	QVector3D target, position;

	if(step.x())
	{
		forward *= step.x();

		target = forward + m_camera.raw().target;
		position = forward + m_camera.raw().position;

		updated = true;
	}
	if(step.y())
	{
		QVector3D side = QVector3D::crossProduct(QVector3D(0, 0, 1), forward).normalized();
		side *= step.y();

		target = side + m_camera.raw().target;
		position = side + m_camera.raw().position;

		updated = true;
	}
	if(step.z())
	{
		target = QVector3D(0, 0, 1)*step.z() + m_camera.raw().target;
		position = QVector3D(0, 0, 1)*step.z() + m_camera.raw().position;

		updated = true;
	}

	if(updated)
	{
		GLboolean orth(false);

		if(m_camera.raw().target != target)
		{
			m_camera.editableRaw().target = target;
			orth = true;
		}
		if(m_camera.raw().position != position)
		{
			m_camera.editableRaw().position = position;
			orth = true;
		}

		if(orth)
			designerUpdateOrtho();
	}
}

void VasnecovWorld::flyCamera(GLfloat x, GLfloat y, GLfloat z)
{
	flyCamera(QVector3D(x, y, z));
}

void VasnecovWorld::moveCamera(const QVector3D &step)
{
	QMutexLocker locker(mtx_data);

	bool updated(false);
	QVector3D target, position;

	// Перемещение в горизонтальной плоскости
	if(step.x() || step.y())
	{
		// Направление на цель в горизональной плоскости (x)
		QVector3D forward = (m_camera.raw().target - m_camera.raw().position);
		forward.setZ(0);
		forward.normalize();
		// Направление вбок (y)
		QVector3D side = QVector3D::crossProduct(QVector3D(0, 0, 1), forward).normalized();
		side.normalize();

		forward *= step.x();
		side *= step.y();

		target = forward + side + m_camera.raw().target;
		position = forward + side + m_camera.raw().position;

		updated = true;

	}
	if(step.z())
	{
		target = QVector3D(0, 0, 1)*step.z() + m_camera.raw().target;
		position = QVector3D(0, 0, 1)*step.z() + m_camera.raw().position;

		updated = true;
	}

	if(updated)
	{
		GLboolean orth(false);

		if(m_camera.raw().target != target)
		{
			m_camera.editableRaw().target = target;
			orth = true;
		}
		if(m_camera.raw().position != position)
		{
			m_camera.editableRaw().position = position;
			orth = true;
		}

		if(orth)
			designerUpdateOrtho();
	}
}

void VasnecovWorld::moveCamera(GLfloat x, GLfloat y, GLfloat z)
{
	moveCamera(QVector3D(x, y, z));
}

void VasnecovWorld::rotateCamera(GLfloat yaw, GLfloat pitch)
{
	QMutexLocker locker(mtx_data);

	QVector3D vec = m_camera.raw().target - m_camera.raw().position;

	QQuaternion qYaw = QQuaternion::fromAxisAndAngle(0, 0, 1, yaw);
	QQuaternion qPitch = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(QVector3D(0, 0, 1), vec).normalized(), pitch);

	vec = qYaw.rotatedVector(vec);
	vec = qPitch.rotatedVector(vec);

	QVector3D target = vec + m_camera.raw().position;
	if(m_camera.raw().target != target)
	{
		m_camera.editableRaw().target = target;
		designerUpdateOrtho();
	}
}

void VasnecovWorld::rotateCamera(GLfloat yaw, GLfloat pitch, GLfloat roll)
{
	QMutexLocker locker(mtx_data);
	if(m_camera.raw().roll != roll)
	{
		m_camera.editableRaw().roll = roll;
	}
	locker.unlock();

	rotateCamera(yaw, pitch);
}

void VasnecovWorld::setCameraRoll(GLfloat roll)
{
	QMutexLocker locker(mtx_data);
	if(m_camera.raw().roll != roll)
	{
		m_camera.editableRaw().roll = roll;
		designerUpdateOrtho();
	}
}

void VasnecovWorld::tiltCamera(GLfloat roll)
{
	QMutexLocker locker(mtx_data);
	if(roll)
	{
		m_camera.editableRaw().roll = m_camera.raw().roll + roll;
		designerUpdateOrtho();
	}
}

/*!
 \brief

 \fn VasnecovWorld::renderUpdateData
 \return GLenum
*/
GLenum VasnecovWorld::renderUpdateData()
{
	GLenum updated(0);

	// Обновление своих данных
	updated |= m_elements.synchronizeAll();

	if(raw_wasUpdated)
	{
		pure_pipeline->setSomethingWasUpdated();

		m_parameters.update();
		m_perspective.update();
		m_ortho.update();
		m_camera.update();

		Vasnecov::CoreObject::renderUpdateData();
	}
	return updated;
}

/*!
 \brief

 \fn VasnecovWorld::renderDraw
*/
void VasnecovWorld::renderDraw()
{
	if(!m_isHidden.pure())
	{
		pure_pipeline->clearZBuffer();

		// Задание характеристик мира
		pure_pipeline->activateDepth(m_parameters.pure().depth);
		pure_pipeline->setDrawingType(m_parameters.pure().drawingType);
		pure_pipeline->setColor(QColor(255, 255, 255, 255));

		// Проецирование и установка камеры
		pure_pipeline->setViewport(m_parameters.pure().x, m_parameters.pure().y, m_parameters.pure().width, m_parameters.pure().height);

		if(m_parameters.pure().projection == Vasnecov::WorldTypePerspective) // Перспективная проекция
		{
			pure_pipeline->setPerspective(m_perspective.pure(), renderCalculateCamera());
		}
		else // Ортогональная
		{
			pure_pipeline->setOrtho(m_ortho.pure(), renderCalculateCamera());
		}

		// Моделирование
		// Обработка источников света
		pure_pipeline->disableAllConcreteLamps(); // Выключение ламп, которые могли быть задействованы
		pure_pipeline->disableLamps();

		GLboolean lampsWork(false);
		if(m_elements.hasPureLamps() && m_parameters.pure().light)
		{
			pure_pipeline->setAmbientColor(m_lightModel.ambientColor());
			pure_pipeline->enableLamps();
			lampsWork = true;

			m_elements.forEachPureLamp(renderDrawElement<VasnecovLamp>);
		}

		// Рисование фигур (непрозрачных)
		std::vector<VasnecovFigure *> transFigures;

		if(m_elements.hasPureFigures())
		{
			// Задание материала по умолчанию
			VasnecovMaterial mat(mtx_data, pure_pipeline);
			mat.renderDraw();

			pure_pipeline->disableLamps();
			pure_pipeline->enableBackFaces();
			pure_pipeline->disableTexture2D();
			pure_pipeline->disableSmoothShading();

			transFigures.reserve(m_elements.pureFigures().size());
			for(std::vector<VasnecovFigure *>::const_iterator fit = m_elements.pureFigures().begin();
				fit != m_elements.pureFigures().end(); ++fit)
			{
				VasnecovFigure *fig(*fit);
				if(fig)
				{
					if(fig->renderIsTransparency())
					{
						transFigures.push_back(fig);
					}
					else
					{
						if(fig->renderLighting() && lampsWork)
						{
							pure_pipeline->enableLamps();
						}
						else
						{
							pure_pipeline->disableLamps();
						}
						fig->renderDraw();
					}
				}
			}

			pure_pipeline->setLineWidth(1.0f);
			pure_pipeline->setPointSize(1.0f);
			pure_pipeline->disableBackFaces();
			pure_pipeline->enableSmoothShading();
			renderSwitchLamps();
		}


		// Отрисовка непрозрачных изделий (деталей)
		std::vector<VasnecovProduct *> transProducts;

		if(m_elements.hasPureProducts())
		{
			transProducts.reserve(m_elements.pureProducts().size());
			for(std::vector<VasnecovProduct *>::const_iterator pit = m_elements.pureProducts().begin();
				pit != m_elements.pureProducts().end(); ++pit)
			{
				VasnecovProduct *prod(*pit);
				if(prod)
				{
					if(prod->renderIsTransparency())
					{
						transProducts.push_back(prod);
					}
					else
					{
						prod->renderDraw();
					}
				}
			}
		}

		// Прозрачные и полупрозрачные изделия (детали)
		if(!transProducts.empty())
		{			
			if(Vasnecov::cfg_sortTransparency)
			{
				QVector3D viewVector = (m_camera.pure().target - m_camera.pure().position).normalized();
				QVector3D viewPoint = m_camera.pure().position;

				for(std::vector<VasnecovProduct *>::const_iterator pit = transProducts.begin();
					pit != transProducts.end(); ++pit)
				{
					VasnecovProduct *prod(*pit);
					if(prod)
					{
						prod->renderCalculateDistanceToPlane(viewPoint, viewVector);
					}
				}

				std::sort(transProducts.begin(), transProducts.end(), VasnecovElement::renderCompareByReverseDistance);
			}

			for(std::vector<VasnecovProduct *>::const_iterator pit = transProducts.begin();
				pit != transProducts.end(); ++pit)
			{
				VasnecovProduct *prod(*pit);
				if(prod)
				{
					prod->renderDraw();
				}
			}
		}

		// Рисование фигур (прозрачных)
		if(!transFigures.empty())
		{
			if(Vasnecov::cfg_sortTransparency)
			{
				QVector3D viewVector = (m_camera.pure().target - m_camera.pure().position).normalized();
				QVector3D viewPoint = m_camera.pure().position;

				for(std::vector<VasnecovFigure *>::iterator fit = transFigures.begin();
					fit != transFigures.end(); ++fit)
				{
					VasnecovFigure *fig(*fit);
					if(fig)
					{
						fig->renderCalculateDistanceToPlane(viewPoint, viewVector);
					}
				}

				std::sort(transFigures.begin(), transFigures.end(), VasnecovElement::renderCompareByReverseDistance);
			}

			// Задание материала по умолчанию
			VasnecovMaterial mat(mtx_data, pure_pipeline);
			mat.renderDraw();

			pure_pipeline->disableLamps();
			pure_pipeline->enableBackFaces();
			pure_pipeline->disableTexture2D();
			pure_pipeline->disableSmoothShading();

			for(std::vector<VasnecovFigure *>::iterator fit = transFigures.begin();
				fit != transFigures.end(); ++fit)
			{
				VasnecovFigure *fig(*fit);
				if(fig)
				{
					if(fig->renderLighting() && lampsWork)
					{
						pure_pipeline->enableLamps();
					}
					else
					{
						pure_pipeline->disableLamps();
					}
					fig->renderDraw();
				}
			}

			pure_pipeline->setLineWidth(1.0f);
			pure_pipeline->setPointSize(1.0f);
			pure_pipeline->disableBackFaces();
			pure_pipeline->enableSmoothShading();
			renderSwitchLamps();
		}

		// Отрисовка меток
		if(m_elements.hasPureLabels())
		{
			pure_pipeline->disableDepth();
			pure_pipeline->clearZBuffer();
			pure_pipeline->disableLamps();
			pure_pipeline->disableBackFaces();

			pure_pipeline->setOrtho2D();
				m_elements.forEachPureLabel(renderDrawElement<VasnecovLabel>);
			pure_pipeline->unsetOrtho2D();
		}
	}
}

/*!
 \brief

 \fn VasnecovWorld::worldParameters
 \return Vasnecov::WorldParameters
*/
Vasnecov::WorldParameters VasnecovWorld::worldParameters() const
{
	QMutexLocker locker(mtx_data);

	Vasnecov::WorldParameters parameters(m_parameters.raw());
	return parameters;
}

Vasnecov::WorldTypes VasnecovWorld::projection() const
{
	QMutexLocker locker(mtx_data);

	Vasnecov::WorldTypes projection(m_parameters.raw().projection);
	return projection;
}
/*!
 \brief

 \fn VasnecovWorld::setDrawingType
 \param type
*/
void VasnecovWorld::setDrawingType(Vasnecov::PolygonDrawingTypes type)
{
	QMutexLocker locker(mtx_data);

	if(m_parameters.raw().drawingType != type)
	{
		m_parameters.editableRaw().drawingType = type;
	}
}

void VasnecovWorld::setCameraPosition(const QVector3D &position)
{
	QMutexLocker locker(mtx_data);

	if(m_camera.raw().position != position)
	{
		m_camera.editableRaw().position = position;
		designerUpdateOrtho();
	}
}
/*!
 \brief

 \fn VasnecovWorld::setCameraPosition
 \param x
 \param y
 \param z
*/
void VasnecovWorld::setCameraPosition(GLfloat x, GLfloat y, GLfloat z)
{
	setCameraPosition(QVector3D(x, y, z));
}
void VasnecovWorld::setCameraTarget(const QVector3D &target)
{
	QMutexLocker locker(mtx_data);

	if(m_camera.raw().target != target)
	{
		m_camera.editableRaw().target = target;
		designerUpdateOrtho();
	}
}
/*!
 \brief

 \fn VasnecovWorld::setCameraTarget
 \param x
 \param y
 \param z
*/
void VasnecovWorld::setCameraTarget(GLfloat x, GLfloat y, GLfloat z)
{
	setCameraTarget(QVector3D(x, y, z));
}
/*!
 \brief

 \fn VasnecovWorld::setCamera
 \param camera
*/
void VasnecovWorld::setCamera(const Vasnecov::Camera &camera)
{
	QMutexLocker locker(mtx_data);

	if(m_camera.set(camera))
		designerUpdateOrtho(); // т.к. изменяется расстояние между фронтальными границами
}

/*!
 \brief

 \fn VasnecovWorld::Camera
 \return Vasnecov::Camera
*/
Vasnecov::Camera VasnecovWorld::camera() const
{
	QMutexLocker locker(mtx_data);

	Vasnecov::Camera camera(m_camera.raw());
	return camera;
}

/*!
 \brief

 \fn VasnecovWorld::perspective
 \return Vasnecov::Perspective
*/
Vasnecov::Perspective VasnecovWorld::perspective() const
{
	QMutexLocker locker(mtx_data);

	Vasnecov::Perspective perspective(m_perspective.raw());
	return perspective;
}
/*!
 \brief

 \fn VasnecovWorld::ortho
 \return Vasnecov::Ortho
*/
Vasnecov::Ortho VasnecovWorld::ortho() const
{
	QMutexLocker locker(mtx_data);

	Vasnecov::Ortho orto(m_ortho.raw());
	return orto;
}

/*!
 \brief

 \fn VasnecovWorld::setProjection
 \param type
 \return GLboolean
*/
GLboolean VasnecovWorld::setProjection(Vasnecov::WorldTypes type)
{
	if(type == Vasnecov::WorldTypePerspective || type == Vasnecov::WorldTypeOrthographic)
	{
		QMutexLocker locker(mtx_data);

		if(m_parameters.raw().projection != type)
		{
			m_parameters.editableRaw().projection = type;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}
/*!
 \brief

 \fn VasnecovWorld::setWindow
 \param x
 \param y
 \param width
 \param height
 \return GLboolean
*/
GLboolean VasnecovWorld::setWindow(GLint x, GLint y, GLsizei width, GLsizei height)
{
	if(width > Vasnecov::cfg_worldWidthMin && width < Vasnecov::cfg_worldWidthMax &&
	   height > Vasnecov::cfg_worldHeightMin && height < Vasnecov::cfg_worldHeightMax)
	{
		QMutexLocker locker(mtx_data);

		GLboolean updated (false);

		if(m_parameters.raw().x != x)
		{
			m_parameters.editableRaw().x = x;
			updated = true;
		}

		if(m_parameters.raw().y != y)
		{
			m_parameters.editableRaw().y = y;
			updated = true;
		}

		if(m_parameters.raw().width != width)
		{
			m_parameters.editableRaw().width = width;
			updated = true;
		}

		if(m_parameters.raw().height != height)
		{
			m_parameters.editableRaw().height = height;
			updated = true;
		}


		if(!qFuzzyCompare(m_perspective.raw().ratio, static_cast<GLfloat>(width)/height))
		{
			m_perspective.editableRaw().ratio = static_cast<GLfloat>(width)/height;
			updated = true;
		}

		if(updated)
			designerUpdateOrtho();

		return 1;
	}
	else
	{
		return 0;
	}
}

/*!
 \brief

 \fn VasnecovWorld::setParameters
 \param parameters
 \return GLboolean
*/
GLboolean VasnecovWorld::setParameters(Vasnecov::WorldParameters parameters) // TODO: to different parameters
{
	if(parameters.width > Vasnecov::cfg_worldWidthMin && parameters.width < Vasnecov::cfg_worldWidthMax &&
	   parameters.height > Vasnecov::cfg_worldHeightMin && parameters.height < Vasnecov::cfg_worldHeightMax &&
	   (parameters.projection == Vasnecov::WorldTypePerspective ||
		parameters.projection == Vasnecov::WorldTypeOrthographic))
	{
		QMutexLocker locker(mtx_data);

		GLboolean updated(false);

		updated = m_parameters.set(parameters);

		if(!qFuzzyCompare(m_perspective.raw().ratio, static_cast<GLfloat>(parameters.width)/parameters.height))
		{
			m_perspective.editableRaw().ratio = static_cast<GLfloat>(parameters.width)/parameters.height;
			updated = true;
		}

		if(updated)
			designerUpdateOrtho();

		return 1;
	}
	else
	{
		return 0;
	}
}

/*!
 \brief

 \fn VasnecovWorld::setPerspective
 \param angle
 \param frontBorder
 \param backBorder
 \return GLboolean
*/
GLboolean VasnecovWorld::setPerspective(GLfloat angle, GLfloat frontBorder, GLfloat backBorder)
{
	if((angle > 0.0f && angle < 180.0f) && frontBorder > 0.0f)
	{
		QMutexLocker locker(mtx_data);

		GLboolean updated(false);

		if(!qFuzzyCompare(m_perspective.raw().frontBorder, frontBorder))
		{
			m_perspective.editableRaw().frontBorder = frontBorder;
			updated = true;
		}

		if(!qFuzzyCompare(m_perspective.raw().backBorder, backBorder))
		{
			m_perspective.editableRaw().backBorder = backBorder;
			updated = true;
		}

		if(!qFuzzyCompare(m_perspective.raw().angle, angle))
		{
			m_perspective.editableRaw().angle = angle;
			updated = true;
		}

		if(updated)
			designerUpdateOrtho();

		return 1;
	}
	else
	{
		return 0;
	}
}

/*!
 \brief

 \fn VasnecovWorld::setDepth
*/
void VasnecovWorld::setDepth()
{
	QMutexLocker locker(mtx_data);

	if(!m_parameters.raw().depth)
		m_parameters.editableRaw().depth = true;
}
/*!
 \brief

 \fn VasnecovWorld::unsetDepth
*/
void VasnecovWorld::unsetDepth()
{
	QMutexLocker locker(mtx_data);

	if(m_parameters.raw().depth)
		m_parameters.editableRaw().depth = false;
}

GLboolean VasnecovWorld::depth() const
{
	QMutexLocker locker(mtx_data);

	return m_parameters.raw().depth;
}

void VasnecovWorld::switchDepth()
{
	QMutexLocker locker(mtx_data);

	m_parameters.editableRaw().depth = !m_parameters.raw().depth;
}

/*!
 \brief

 \fn VasnecovWorld::setLight
*/
void VasnecovWorld::setLight()
{
	QMutexLocker locker(mtx_data);

	if(!m_parameters.raw().light)
		m_parameters.editableRaw().light = true;
}
/*!
 \brief

 \fn VasnecovWorld::unsetLight
*/
void VasnecovWorld::unsetLight()
{
	QMutexLocker locker(mtx_data);

	if(m_parameters.editableRaw().light)
		m_parameters.editableRaw().light = false;
}

GLboolean VasnecovWorld::light() const
{
	QMutexLocker locker(mtx_data);

	return m_parameters.raw().light;
}

void VasnecovWorld::switchLight()
{
	QMutexLocker locker(mtx_data);

	m_parameters.editableRaw().light = !m_parameters.raw().light;
}

VasnecovPipeline::CameraAttributes VasnecovWorld::renderCalculateCamera() const
{
	// Расчет направлений камеры
	VasnecovPipeline::CameraAttributes cameraAttr;
	QVector3D forward = (m_camera.pure().target - m_camera.pure().position).normalized();

	cameraAttr.eye = m_camera.pure().position;
	cameraAttr.center = m_camera.pure().target;

	// Камере (lookAt) не нужен строгий вектор направления вверх. LookAt всё равно берет с него проекцию.
	// Если вектор направление взгляда вертикален, то up-вектор просто направляем по X
	if(m_camera.pure().position.x() == m_camera.pure().target.x() && m_camera.pure().position.y() == m_camera.pure().target.y())
	{
		cameraAttr.up = QVector3D(1.0f, 0.0f, 0.0f);
	}
	else // иначе, по Z
	{
		cameraAttr.up = QVector3D(0.0f, 0.0f, 1.0f);
	}

	// Вращение вокруг вектора взгляда
	cameraAttr.up = QQuaternion::fromAxisAndAngle(forward, m_camera.pure().roll).rotatedVector(cameraAttr.up);

	return cameraAttr;
}


#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
