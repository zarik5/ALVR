#include "Settings.h"
#include "Logger.h"
#include "ipctools.h"
#include "resource.h"
#define PICOJSON_USE_INT64
#include <picojson.h>
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

Settings Settings::m_Instance;

Settings::Settings()
{
	m_OffsetPos[0] = 0.0f;
	m_OffsetPos[1] = 0.0f;
	m_OffsetPos[2] = 0.0f;
}

Settings::~Settings()
{
}

void Settings::Load()
{
	try
	{
		auto sessionFile = std::ifstream(g_alvrDir + "/session.json"s);

		auto json = std::string(
			std::istreambuf_iterator<char>(sessionFile),
			std::istreambuf_iterator<char>());

		picojson::value v;
		std::string err = picojson::parse(v, json);
		if (!err.empty())
		{
			Error("Error on parsing json: %hs", err.c_str());
			return;
		}

		auto openvrConfig = v.get("openvrConfig");

		mSerialNumber = openvrConfig.get("headsetSerialNumber").get<std::string>();
		mTrackingSystemName = openvrConfig.get("headsetTrackingSystemName").get<std::string>();
		mModelNumber = openvrConfig.get("headsetModelNumber").get<std::string>();
		mDriverVersion = openvrConfig.get("headsetDriverVersion").get<std::string>();
		mManufacturerName = openvrConfig.get("headsetManufacturerName").get<std::string>();
		mRenderModelName = openvrConfig.get("headsetRenderModelName").get<std::string>();
		mRegisteredDeviceType = openvrConfig.get("headsetRegisteredDeviceType").get<std::string>();

		m_renderWidth = openvrConfig.get("eyeResolutionWidth").get<int64_t>() * 2;
		m_renderHeight = openvrConfig.get("eyeResolutionHeight").get<int64_t>();

		m_recommendedTargetWidth = openvrConfig.get("targetEyeResolutionWidth").get<int64_t>() * 2;
		m_recommendedTargetHeight = openvrConfig.get("targetEyeResolutionHeight").get<int64_t>();

		m_eyeFov[0].left = (float)openvrConfig.get("leftEyeFov").get("left").get<double>();
		m_eyeFov[0].right = (float)openvrConfig.get("leftEyeFov").get("right").get<double>();
		m_eyeFov[0].top = (float)openvrConfig.get("leftEyeFov").get("top").get<double>();
		m_eyeFov[0].bottom = (float)openvrConfig.get("leftEyeFov").get("bottom").get<double>();
		m_eyeFov[1].left = m_eyeFov[0].right; // NB: left and right values are swapped intentionally
		m_eyeFov[1].right = m_eyeFov[0].left;
		m_eyeFov[1].top = m_eyeFov[0].top;
		m_eyeFov[1].bottom = m_eyeFov[0].bottom;

		m_flSecondsFromVsyncToPhotons = (float)openvrConfig.get("secondsFromVsyncToPhotons").get<double>();

		m_flIPD = (float)openvrConfig.get("ipd").get<double>();

		m_nAdapterIndex = (int32_t)openvrConfig.get("adapterIndex").get<int64_t>();

		m_refreshRate = (int)openvrConfig.get("fps").get<int64_t>();

		m_controllerTrackingSystemName = openvrConfig.get("controllersTrackingSystemName").get<std::string>();
		m_controllerManufacturerName = openvrConfig.get("controllersManufacturerName").get<std::string>();
		m_controllerModelNumber = openvrConfig.get("controllersModelNumber").get<std::string>();
		m_controllerRenderModelNameLeft = openvrConfig.get("renderModelNameLeftController").get<std::string>();
		m_controllerRenderModelNameRight = openvrConfig.get("renderModelNameRightController").get<std::string>();
		m_controllerSerialNumber = openvrConfig.get("controllersSerialNumber").get<std::string>();
		m_controllerType = openvrConfig.get("controllersType").get<std::string>();
		mControllerRegisteredDeviceType = openvrConfig.get("controllersRegisteredDeviceType").get<std::string>();
		m_controllerInputProfilePath = openvrConfig.get("controllersInputProfilePath").get<std::string>();

		m_controllerMode = (int32_t)openvrConfig.get("controllersModeIdx").get<int64_t>();

		m_disableController = !openvrConfig.get("controllersEnabled").get<bool>();

		m_enableFoveatedRendering = openvrConfig.get("enableFoveatedRendering").get<bool>();
		m_foveationStrength = (float)openvrConfig.get("foveationStrength").get<double>();
		m_foveationShape = (float)openvrConfig.get("foveationShape").get<double>();
		m_foveationVerticalOffset = (float)openvrConfig.get("foveationVerticalOffset").get<double>();

		m_enableColorCorrection = openvrConfig.get("enableColorCorrection").get<bool>();
		m_brightness = (float)openvrConfig.get("brightness").get<double>();
		m_contrast = (float)openvrConfig.get("contrast").get<double>();
		m_saturation = (float)openvrConfig.get("saturation").get<double>();
		m_gamma = (float)openvrConfig.get("gamma").get<double>();
		m_sharpening = (float)openvrConfig.get("sharpening").get<double>();
	}
	catch (std::exception &e)
	{
		FatalLog("Exception on parsing json: %hs", e.what());
	}
}

void Settings::UpdateForStream(StreamSettings settings)
{
	m_enableSound = settings.gameAudio;
	m_soundDevice = settings.gameAudioDevice;
	m_streamMic = settings.microphone;

	m_keyframeResendIntervalMs = settings.keyframeResendIntervalMs;

	m_codec = settings.codec;
	mEncodeBitrate = Bitrate::fromMiBits(settings.encodeBitrateMbs);

	m_trackingFrameOffset = settings.trackingFrameOffset;
	m_controllerPoseOffset = settings.poseTimeOffset;

	m_leftControllerPositionOffset[0] = settings.positionOffsetLeft[0];
	m_leftControllerPositionOffset[1] = settings.positionOffsetLeft[1];
	m_leftControllerPositionOffset[2] = settings.positionOffsetLeft[2];

	m_leftControllerRotationOffset[0] = settings.rotationOffsetLeft[0];
	m_leftControllerRotationOffset[1] = settings.rotationOffsetLeft[1];
	m_leftControllerRotationOffset[2] = settings.rotationOffsetLeft[2];

	m_hapticsIntensity = settings.hapticsIntensity;
}
