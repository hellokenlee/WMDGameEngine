﻿#include "PyCameraController.h"
#include "../NeneEngine/CameraController.h"

namespace py = pybind11;

void BindCameraController(py::module& mod)
{
	auto cls = py::class_<CameraController, std::shared_ptr<CameraController>>(mod, "CameraController")
		.def(py::init<>())
		.def("Update", &CameraController::Update)
		.def("get_camera", &CameraController::getCamera)
		.def_readwrite("speed", &CameraController::mSpeed)
		.def_readwrite("sensitivity", &CameraController::mSensitivity)
		;
}
