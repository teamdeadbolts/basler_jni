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
  try {
    camera->Open();
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::CameraInstance] Exception during camera open: "
        << e.GetDescription() << std::endl;
  }
}

CameraInstance::~CameraInstance() {
  try {
    CameraInstance::stop();

    camera->Close();
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::~CameraInstance] Exception during camera close: "
        << e.GetDescription() << std::endl;
  }
}

bool CameraInstance::start() {
  try {
    if (!camera->IsOpen()) {
      camera->Open();
    }
    camera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
    camera->AcquisitionStart.Execute();
    camera->StartGrabbing(GrabStrategy_LatestImages);

    return true;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::start] Exception during camera start: "
              << e.GetDescription() << std::endl;
    return false;
  }
}

bool CameraInstance::stop() {
  try {
    if (camera->IsGrabbing()) {
      camera->StopGrabbing();
    }

    camera->AcquisitionStop.Execute();
    return true;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::stop] Exception during camera stop: "
              << e.GetDescription() << std::endl;
    return false;
  }
}

void CameraInstance::awaitNewFrame() {
  try {
    if (!camera->IsGrabbing()) {
      // std::cout
      //     << "[CameraInstance::awaitNewFrame] Warning: called await new frame "
      //        "but the camera is not grabbing, call startCamera() first."
      //     << std::endl;
    }
    while (camera->IsGrabbing()) {
      CGrabResultPtr grabResult;
      try {
        if (camera->RetrieveResult(5000, grabResult,
                                   TimeoutHandling_ThrowException)) {
          if (grabResult->GrabSucceeded()) {
            std::lock_guard<std::mutex> lock(frameMutex);
            currentGrabResult = grabResult;
            currentFramePtr = convertToMat(grabResult);
            return;
          }
        }
      } catch (const TimeoutException &e) {
        std::cout << "[CameraInstance::awaitNewFrame] Timeout while waiting "
                     "for frame: "
                  << e.GetDescription() << std::endl;
        return;
      }
    }
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::awaitNewFrame] Exception during frame grab: "
              << e.GetDescription() << std::endl;
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
    // colorCvt = cv::COLOR_GRAY2BGR;
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
  try {
    if (camera->ExposureTime.IsReadable()) {
      return camera->ExposureTime.GetValue();
    }
    std::cout << "[CameraInstance::getExposure] ExposureTime not readable."
              << std::endl;
    return -1.0;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getExposure] Exception during getExposure: "
              << e.GetDescription() << std::endl;
    return -1.0;
  }
}

bool CameraInstance::getAutoExposure() const {
  try {
    if (camera->ExposureAuto.IsReadable()) {
      return camera->ExposureAuto.GetValue() != ExposureAuto_Off;
    }
    std::cout << "[CameraInstance::getAutoExposure] ExposureAuto not readable."
              << std::endl;
    return false;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getAutoExposure] Exception during "
                 "getAutoExposure: "
              << e.GetDescription() << std::endl;
    return false;
  }
}

double CameraInstance::getGain() const {
  try {
    if (camera->Gain.IsReadable()) {
      return camera->Gain.GetValue();
    }
    std::cout << "[CameraInstance::getGain] Gain not readable." << std::endl;
    return -1.0;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getGain] Exception during getGain: "
              << e.GetDescription() << std::endl;
    return -1.0;
  }
}

double CameraInstance::getFrameRate() const {
  try {
    if (camera->AcquisitionFrameRate.IsReadable()) {
      return camera->AcquisitionFrameRate.GetValue();
    }
    std::cout
        << "[CameraInstance::getFrameRate] AcquisitionFrameRate not readable."
        << std::endl;
    return -1.0;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::getFrameRate] Exception during getFrameRate: "
        << e.GetDescription() << std::endl;
    return -1.0;
  }
}

bool CameraInstance::getAutoWhiteBalance() const {
  try {
    if (camera->BalanceWhiteAuto.IsReadable()) {
      return camera->BalanceWhiteAuto.GetValue() != BalanceWhiteAuto_Off;
    }
    std::cout << "[CameraInstance::getWhiteBalance] BalanceRatio not readable."
              << std::endl;
    return -1.0;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getAutoWhiteBalance] Exception during "
                 "getAutoWhiteBalance: "
              << e.GetDescription() << std::endl;
    return false;
  }
}

std::vector<int> CameraInstance::getSupportedPixelFormats() const {
  try {
    std::vector<int> formats;

    if (camera->PixelFormat.IsReadable()) {
      GenApi::StringList_t supportedFormats;
      camera->PixelFormat.GetSettableValues(supportedFormats);

      for (const auto &formatStr : supportedFormats) {
        if (formatStr == "RGB8") {
          formats.push_back(4); // kBGR
        } else if (formatStr == "YCbCr422_8") {
          formats.push_back(7); // kUYVY
        } else if (formatStr == "Mono8") {
          formats.push_back(5);
        }
      }
    } else {
      std::cout << "[CameraInstance::getSupportedPixelFormats] PixelFormat not "
                   "readable."
                << std::endl;
    }

    return formats;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getSupportedPixelFormats] Exception during "
                 "getSupportedPixelFormats: "
              << e.GetDescription() << std::endl;
    return {};
  }
}

std::array<double, 3> CameraInstance::getWhiteBalance() {
  std::array<double, 3> balances = {-1.0, -1.0, -1.0};

  try {
    if (camera->BalanceWhiteAuto.IsReadable()) {
      camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
      balances[0] = camera->BalanceRatio.GetValue();
      camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
      balances[1] = camera->BalanceRatio.GetValue();
      camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
      balances[2] = camera->BalanceRatio.GetValue();
    } else {
      std::cout << "[CameraInstance::getAutoWhiteBalance] BalanceWhiteAuto not "
                   "readable."
                << std::endl;
    }
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getWhiteBalance] Exception during "
                 "getWhiteBalance: "
              << e.GetDescription() << std::endl;
  }
  return balances;
}

int CameraInstance::getPixelFormat() const {
  try {
    if (camera->PixelFormat.IsReadable()) {
      switch (camera->PixelFormat.GetValue()) {
      case PixelFormat_RGB8:
        return 4; // kBGR;
      case PixelFormat_YCbCr422_8:
        return 7; // kUYVY
      case PixelFormat_Mono8:
        return 5; // kGray
      default:
        return -1;
      }
    }
    std::cout << "[CameraInstance::getPixelFormat] PixelFormat not readable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::getPixelFormat] Exception during getPixelFormat: "
        << e.GetDescription() << std::endl;
  }
  return -1;
}

double CameraInstance::getMinExposure() const {
  try {
    if (camera->ExposureTime.IsReadable()) {
      return camera->ExposureTime.GetMin();
    }

    std::cout << "[CameraInstance::getMinExposure] ExposureTime not readable"
              << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::getMinExposure] Exception during getMinExposure: "
        << e.GetDescription() << std::endl;
  }
  return -1.0;
}

double CameraInstance::getMaxExposure() const {
  try {
    if (camera->ExposureTime.IsReadable()) {
      return camera->ExposureTime.GetMax();
    }

    std::cout << "[CameraInstance::getMaxExposure] ExposureTime not readable"
              << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::getMaxExposure] Exception during getMaxExposure: "
        << e.GetDescription() << std::endl;
  }
  return -1.0;
}

double CameraInstance::getMinWhiteBalance() const {
  try {
    if (camera->BalanceRatio.IsReadable()) {
      return camera->BalanceRatio.GetMin();
    }

    std::cout
        << "[CameraInstance::getMinWhiteBalance] BalanceRatio not readable"
        << std::endl;
  } catch (GenericException &e) {
    std::cout << "[CameraInstance::getMinWhiteBalance] Exception during "
                 "getMinWhiteBalance: "
              << e.GetDescription() << std::endl;
  }
  return -1.0;
}

double CameraInstance::getMaxWhiteBalance() const {
  try {
    if (camera->BalanceRatio.IsReadable()) {
      return camera->ExposureTime.GetMax();
    }

    std::cout
        << "[CameraInstance::getMaxWhiteBalance] BalanceRatio not readable"
        << std::endl;
  } catch (GenericException &e) {
    std::cout << "[CameraInstance::getMaxWhiteBalance] Exception during "
                 "getMaxWhiteBalance: "
              << e.GetDescription() << std::endl;
  }
  return -1.0;
}

double CameraInstance::getMinGain() const {
  try {
    if (camera->Gain.IsReadable()) {
      return camera->Gain.GetMin();
    }

    std::cout << "[CameraInstance::getMinGain] Gain not readable" << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getMinGain] Exception during getMinGain: "
              << e.GetDescription() << std::endl;
  }
  return -1.0;
}

double CameraInstance::getMaxGain() const {
  try {
    if (camera->Gain.IsReadable()) {
      return camera->Gain.GetMax();
    }

    std::cout << "[CameraInstance::getMaxGain] Gain not readable" << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::getMaxGain] Exception during getMaxGain: "
              << e.GetDescription() << std::endl;
  }
  return -1.0;
}

// Setter implementations

bool CameraInstance::setExposure(double exposure) {
  try {
    if (camera->ExposureTime.IsWritable() &&
        camera->ExposureAuto.IsWritable() &&
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

    std::cout
        << "[CameraInstance::setExposure] ExposureTime or ExposureAuto or "
           "ExposureMode not writable."
        << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setExposure] Exception during setExposure: "
              << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setAutoExposure(bool enable) {
  try {
    if (camera->ExposureAuto.IsWritable()) {
      if (enable) {
        camera->ExposureAuto.SetValue(ExposureAuto_Continuous);
      } else {
        camera->ExposureAuto.SetValue(ExposureAuto_Off);
      }
      return true;
    }
    std::cout << "[CameraInstance::setAutoExposure] ExposureAuto not writable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setAutoExposure] Exception during "
                 "setAutoExposure: "
              << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setGain(double gain) {
  try {
    if (camera->Gain.IsWritable() && camera->GainSelector.IsWritable()) {
      auto min = camera->Gain.GetMin();
      auto max = camera->Gain.GetMax();
      camera->GainSelector.SetValue(GainSelector_All);
      camera->GainAuto.SetValue(GainAuto_Off);

      gain = std::clamp(gain, min, max);

      camera->Gain.SetValue(gain);
      return true;
    }

    std::cout << "[CameraInstance::setGain] Gain not writable." << std::endl;

  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setGain] Exception during setGain: "
              << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setFrameRate(double frameRate) {
  try {
    if (camera->AcquisitionFrameRateEnable.IsWritable()) {
      camera->AcquisitionFrameRateEnable.SetValue(true);
    }

    if (camera->AcquisitionFrameRate.IsWritable()) {
      auto min = camera->AcquisitionFrameRate.GetMin();
      auto max = camera->AcquisitionFrameRate.GetMax();

      frameRate = std::clamp(frameRate, min, max);

      camera->AcquisitionFrameRate.SetValue(frameRate);

      return true;
    }

    std::cout
        << "[CameraInstance::setFrameRate] AcquisitionFrameRate not writable."
        << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::setFrameRate] Exception during setFrameRate: "
        << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setWhiteBalance(std::array<double, 3> balance) {
  try {
    if (camera->BalanceRatio.IsWritable() &&
        camera->BalanceRatioSelector.IsWritable()) {
      auto min = camera->BalanceRatio.GetMin();
      auto max = camera->BalanceRatio.GetMax();
      this->setAutoWhiteBalance(false);

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

    std::cout << "[CameraInstance::setWhiteBalance] BalanceRatio or "
                 "BalanceRatioSelector not writable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setWhiteBalance] Exception during "
                 "setWhiteBalance: "
              << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setAutoWhiteBalance(bool enable) {
  try {
    if (camera->BalanceWhiteAuto.IsWritable()) {
      if (enable) {
        camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Continuous);
      } else {
        camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
      }
      return true;
    }
    std::cout << "[CameraInstance::setAutoWhiteBalance] BalanceWhiteAuto not "
                 "writable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setAutoWhiteBalance] Exception during "
                 "setAutoWhiteBalance: "
              << e.GetDescription() << std::endl;
  }
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
      case 5: // kGray
        camera->PixelFormat.SetValue(PixelFormat_Mono8);
        return true;
      default:
        std::cout << "[CameraInstance::setPixelFormat] Unsupported pixel "
                     "format value: "
                  << format << std::endl;
        return false;
      }
    }
    std::cout << "[CameraInstance::setPixelFormat] PixelFormat not writable."
              << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::setPixelFormat] Exception setting PixelFormat: "
        << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setBrightness(double brightness) {
  try {
    if (camera->BslBrightness.IsWritable()) {
      brightness = std::clamp(brightness, -1.0, 1.0);

      camera->BslBrightness.SetValue(brightness);
      return true;
    }

    std::cout << "[CameraInstance::setBrightness] BslBrightness not writable"
              << std::endl;
  } catch (const GenericException &e) {
    std::cout
        << "[CameraInstance::setBrightness] Exception during setBrightness: "
        << e.GetDescription() << std::endl;
  }
  return false;
}

bool CameraInstance::setPixelBinning(int binMode, int horzBin, int vertBin) {
  try {
    if (camera->BinningHorizontal.IsWritable() &&
        camera->BinningVertical.IsWritable() &&
        camera->BinningHorizontalMode.IsWritable() &&
        camera->BinningVerticalMode.IsWritable()) {
      if (binMode == 0) {
        camera->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);
        camera->BinningVerticalMode.SetValue(BinningVerticalMode_Average);
      } else if (binMode == 1) {
        camera->BinningHorizontalMode.SetValue(BinningHorizontalMode_Sum);
        camera->BinningVerticalMode.SetValue(BinningVerticalMode_Sum);
      } else {
        std::cout
            << "[CameraInstance::setPixelBinning] Unsupported BinningMode: "
            << binMode << std::endl;
        return false;
      }

      if (camera->BinningSelector.IsWritable()) {
        camera->BinningSelector.SetValue(BinningSelector_Sensor);
      }
      camera->BinningHorizontal.SetValue(horzBin);
      camera->BinningVertical.SetValue(vertBin);
      return true;
    }
    std::cout << "[CameraInstance::setPixelBinning] "
                 "BinningMode[Horizonal|Vertical] or "
                 "Binning[Horizontal|Vertical] not writable."
              << std::endl;

    // log each one to see which is not writable
    if (!camera->BinningHorizontal.IsWritable()) {
      std::cout << " - BinningHorizontal not writable." << std::endl;
    }
    if (!camera->BinningVertical.IsWritable()) {
      std::cout << " - BinningVertical not writable." << std::endl;
    }
    if (!camera->BinningHorizontalMode.IsWritable()) {
      std::cout << " - BinningHorizontalMode not writable." << std::endl;
    }
    if (!camera->BinningVerticalMode.IsWritable()) {
      std::cout << " - BinningVerticalMode not writable." << std::endl;
    }

  } catch (const GenericException &e) {
    std::cout << "[CameraInstance::setPixelBinning] Exception setting pixel "
                 "binning: "
              << e.GetDescription() << std::endl;
  }
  return false;
}