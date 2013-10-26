
#include "core\renderthread.h"
#include "core\render.cuh"

#include <QSettings>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>
#include <QDebug>

#include <time.h>

QRenderer::QRenderer(QObject* Parent /*= 0*/) :
	QObject(Parent),
	Settings("renderer.ini", QSettings::IniFormat),
	RenderTimer(),
	AvgFps(),
	Renderer()
{
	connect(&this->RenderTimer, SIGNAL(timeout()), this, SLOT(OnRender()));
}

void QRenderer::Start()
{
	this->RenderTimer.start(Settings.value("rendering/targetfps", 60).toInt());
}

void QRenderer::OnRender()
{
	this->Renderer.Camera.SetApertureSize(0.005f);
	this->Renderer.Camera.SetFocalDistance(0.1f);

	this->Renderer.Camera.Update();
	
	const clock_t Begin = clock();
	
	ExposureRender::Render(this->Renderer.Camera);

	this->Renderer.Camera.GetFilm().IncrementNoEstimates();

	const clock_t End = clock();

	AvgFps.PushValue(1000.0f / (End - Begin));
}