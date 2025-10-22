#include "camera_instance.hpp"
#include <array>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/PylonIncludes.h>

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

CameraInstance::CameraInstance(IPylonDevice *device)
    : camera(std::make_unique<CBaslerUniversalInstantCamera>(device)) {
  camera->Open();
}

CameraInstance::~CameraInstance() {
  CameraInstance::stop();

  camera->Close();
}

bool CameraInstance::start() {
  if (!camera->IsOpen()) {
    camera->Open();
  }
  camera->StartGrabbing(GrabStrategy_LatestImageOnly);
  return true;
}

bool CameraInstance::stop() {
  if (camera->IsGrabbing()) {
    camera->StopGrabbing();
  }
  return true;
}

void CameraInstance::awaitNewFrame() {
  while (camera->IsGrabbing()) {
    CGrabResultPtr grabResult;
    if (camera->RetrieveResult(5000, grabResult,
                               TimeoutHandling_ThrowException)) {
      if (grabResult->GrabSucceeded()) {
        std::lock_guard<std::mutex> lock(frameMutex);
        currentGrabResult = grabResult;
        currentFramePtr = convertToMat(grabResult);
        return;
      }
    }
  }
}

std::shared_ptr<cv::Mat> CameraInstance::takeFrame() {
  std::lock_guard<std::mutex> lock(frameMutex);
  return currentFramePtr; // TODO: Maybe dont clone?
}

std::shared_ptr<cv::Mat>
CameraInstance::convertToMat(const CGrabResultPtr &grabResult) {
  int cvType;
  int colorCvt = -1;

  auto pixelType = grabResult->GetPixelType();
  switch (pixelType) {
  case PixelType_Mono8:
    cvType = CV_8UC1;
    break;
  case PixelType_BGR8packed:
    cvType = CV_8UC3;
    break;
  case PixelType_RGB8packed:
    cvType = CV_8UC3;
    colorCvt = cv::COLOR_RGB2BGR;
    break;
  case PixelType_YUV422_YUYV_Packed:
  case PixelType_YUV422packed:
    cvType = CV_8UC2;
    colorCvt = cv::COLOR_YUV2BGR_YUYV;
    break;
  case PixelType_YCbCr422_8_YY_CbCr_Semiplanar:
    cvType = CV_8UC2;
    colorCvt = cv::COLOR_YUV2BGR_UYVY;
    break;
  default:
    throw std::runtime_error("Unsupported pixel format");
  }

  cv::Mat wrapped(grabResult->GetHeight(), grabResult->GetWidth(), cvType,
                  (uint8_t *)grabResult->GetBuffer());

  cv::Mat owned = wrapped.clone();
  if (colorCvt != -1) {
    cv::Mat converted;
    cv::cvtColor(owned, converted, colorCvt);
    return std::make_shared<cv::Mat>(std::move(converted));
  }

  return std::make_shared<cv::Mat>(std::move(owned));
}

// Getter implementations

double CameraInstance::getExposure() const {
  if (camera->ExposureTime.IsReadable()) {
    return camera->ExposureTime.GetValue();
  }
  std::cerr << "[CameraInstance::getExposure] ExposureTime not readable."
            << std::endl;
  return -1.0;
}

bool CameraInstance::getAutoExposure() const {
  if (camera->ExposureAuto.IsReadable()) {
    return camera->ExposureAuto.GetValue() != ExposureAuto_Off;
  }
  std::cerr << "[CameraInstance::getAutoExposure] ExposureAuto not readable."
            << std::endl;
  return false;
}

double CameraInstance::getGain() const {
  if (camera->Gain.IsReadable()) {
    return camera->Gain.GetValue();
  }
  std::cerr << "[CameraInstance::getGain] Gain not readable." << std::endl;
  return -1.0;
}

double CameraInstance::getFrameRate() const {
  if (camera->AcquisitionFrameRate.IsReadable()) {
    return camera->AcquisitionFrameRate.GetValue();
  }
  std::cerr
      << "[CameraInstance::getFrameRate] AcquisitionFrameRate not readable."
      << std::endl;
  return -1.0;
}

bool CameraInstance::getAutoWhiteBalance() const {
  if (camera->BalanceWhiteAuto.IsReadable()) {
    return camera->BalanceWhiteAuto.GetValue() != BalanceWhiteAuto_Off;
  }
  std::cerr << "[CameraInstance::getWhiteBalance] BalanceRatio not readable."
            << std::endl;
  return -1.0;
}

std::vector<int> CameraInstance::getSupportedPixelFormats() const {
  std::vector<int> formats;

  if (camera->PixelFormat.IsReadable()) {
    GenApi::StringList_t supportedFormats;
    camera->PixelFormat.GetSettableValues(supportedFormats);

    for (const auto &formatStr : supportedFormats) {
      std::cout
          << "[CameraInstance::getSupportedPixelFormats] Supported Format: "
          << formatStr << std::endl;
      if (formatStr == "RGB8") {
        formats.push_back(4); // kBGR
      } else if (formatStr == "YCbCr422_8") {
        formats.push_back(7); // kUYVY
      }
    }
  } else {
    std::cerr << "[CameraInstance::getSupportedPixelFormats] PixelFormat not "
                 "readable."
              << std::endl;
  }

  return formats;
}

std::array<double, 3> CameraInstance::getWhiteBalance() {
  std::array<double, 3> balances = {-1.0, -1.0, -1.0};
  if (camera->BalanceWhiteAuto.IsReadable()) {
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
    balances[0] = camera->BalanceRatio.GetValue();
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
    balances[1] = camera->BalanceRatio.GetValue();
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
    balances[2] = camera->BalanceRatio.GetValue();
  } else {
    std::cerr << "[CameraInstance::getAutoWhiteBalance] BalanceWhiteAuto not "
                 "readable."
              << std::endl;
  }
  return balances;
}

int CameraInstance::getPixelFormat() const {
  if (camera->PixelFormat.IsReadable()) {
    switch (camera->PixelFormat.GetValue()) {
    case PixelFormat_RGB8:
      return 4; // kBGR;
    case PixelFormat_YCbCr422_8:
      return 7; // kUYVY
    default:
      return -1;
    }
  }
  std::cerr << "[CameraInstance::getPixelFormat] PixelFormat not readable."
            << std::endl;
  return -1;
}

double CameraInstance::getMinExposure() const {
  if (camera->ExposureTime.IsReadable()) {
    return camera->ExposureTime.GetMin();
  }

  std::cerr << "[CameraInstance::getMinExposure] ExposureTime not readable"
            << std::endl;
  return -1.0;
}

double CameraInstance::getMaxExposure() const {
  if (camera->ExposureTime.IsReadable()) {
    return camera->ExposureTime.GetMax();
  }

  std::cerr << "[CameraInstance::getMaxExposure] ExposureTime not readable"
            << std::endl;
  return -1.0;
}

double CameraInstance::getMinWhiteBalance() const {
  if (camera->BalanceRatio.IsReadable()) {
    return camera->BalanceRatio.GetMin();
  }

  std::cerr << "[CameraInstance::getMinWhiteBalance] BalanceRatio not readable"
            << std::endl;
  return -1.0;
}

double CameraInstance::getMaxWhiteBalance() const {
  if (camera->BalanceRatio.IsReadable()) {
    return camera->ExposureTime.GetMax();
  }

  std::cerr << "[CameraInstance::getMaxWhiteBalance] BalanceRatio not readable"
            << std::endl;
  return -1.0;
}

// Setter implementations

bool CameraInstance::setExposure(double exposure) {
  if (camera->ExposureTime.IsWritable() && camera->ExposureAuto.IsWritable() &&
      camera->ExposureMode.IsWritable()) {
    auto min = camera->ExposureTime.GetMin();
    auto max = camera->ExposureTime.GetMax();

    exposure = std::clamp(exposure, min, max);

    camera->ExposureAuto.SetValue(ExposureAuto_Off);
    camera->ExposureMode.SetValue(ExposureMode_Timed);
    camera->ExposureTime.SetValue(exposure);
    if (camera->ExposureTimeMode.IsWritable()) {
      camera->ExposureTimeMode.SetValue(ExposureTimeMode_Standard);
    }
    return true;
  }

  std::cerr << "[CameraInstance::setExposure] ExposureTime or ExposureAuto or "
               "ExposureMode not writable."
            << std::endl;
  return false;
}

bool CameraInstance::setAutoExposure(bool enable) {
  if (camera->ExposureAuto.IsWritable()) {
    if (enable) {
      camera->ExposureAuto.SetValue(ExposureAuto_Continuous);
    } else {
      camera->ExposureAuto.SetValue(ExposureAuto_Off);
    }
    return true;
  }
  std::cerr << "[CameraInstance::setAutoExposure] ExposureAuto not writable."
            << std::endl;
  return false;
}

bool CameraInstance::setGain(double gain) {
  if (camera->Gain.IsWritable()) {
    auto min = camera->Gain.GetMin();
    auto max = camera->Gain.GetMax();

    gain = std::clamp(gain, min, max);

    camera->Gain.SetValue(gain);
    return true;
  }

  std::cerr << "[CameraInstance::setGain] Gain not writable." << std::endl;
  return false;
}

bool CameraInstance::setFrameRate(double frameRate) {
  if (camera->AcquisitionFrameRate.IsWritable()) {
    auto min = camera->AcquisitionFrameRate.GetMin();
    auto max = camera->AcquisitionFrameRate.GetMax();

    frameRate = std::clamp(frameRate, min, max);

    camera->AcquisitionFrameRate.SetValue(frameRate);
    return true;
  }

  std::cerr
      << "[CameraInstance::setFrameRate] AcquisitionFrameRate not writable."
      << std::endl;
  return false;
}

bool CameraInstance::setWhiteBalance(std::array<double, 3> balance) {
  if (camera->BalanceRatio.IsWritable() &&
      camera->BalanceRatioSelector.IsWritable()) {
    auto min = camera->BalanceRatio.GetMin();
    auto max = camera->BalanceRatio.GetMax();

    // Set Red
    balance[0] = std::clamp(balance[0], min, max);
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
    camera->BalanceRatio.SetValue(balance[0]);

    // Set Green
    balance[1] = std::clamp(balance[1], min, max);
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
    camera->BalanceRatio.SetValue(balance[1]);

    // Set Blue
    balance[2] = std::clamp(balance[2], min, max);
    camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
    camera->BalanceRatio.SetValue(balance[2]);

    return true;
  }

  std::cerr << "[CameraInstance::setWhiteBalance] BalanceRatio or "
               "BalanceRatioSelector not writable."
            << std::endl;
  return false;
}

bool CameraInstance::setAutoWhiteBalance(bool enable) {
  if (camera->BalanceWhiteAuto.IsWritable()) {
    if (enable) {
      camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Continuous);
    } else {
      camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
    }
    return true;
  }
  std::cerr
      << "[CameraInstance::setAutoWhiteBalance] BalanceWhiteAuto not writable."
      << std::endl;
  return false;
}

bool CameraInstance::setPixelFormat(int format) {
  try {
    if (camera->PixelFormat.IsWritable()) {
      switch (format) {
      case 4: // kBGR
        camera->PixelFormat.SetValue(PixelFormat_RGB8);
        return true;
      case 7: // kUYVY
        camera->PixelFormat.SetValue(PixelFormat_YCbCr422_8);
        return true;
      default:
        std::cerr << "[CameraInstance::setPixelFormat] Unsupported pixel "
                     "format value: "
                  << format << std::endl;
        return false;
      }
    }
    std::cerr << "[CameraInstance::setPixelFormat] PixelFormat not writable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cerr
        << "[CameraInstance::setPixelFormat] Exception setting PixelFormat: "
        << e.GetDescription() << std::endl;
  }

  return false;
}
